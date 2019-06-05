#include <QCoreApplication>
#include <QKeyEvent>
#include <QTimer>
#include <math.h>
#include "glwidget.h"

GLWidget::GLWidget( const QGLFormat& glformat, QWidget* parent )
    : QGLWidget( glformat, parent )
{
    // Defaults
    displayOn  = false;
    toggleDisplay(0); // Start with cubes
    brightness = 1.0;
    xRot = yRot = zRot = 0;
    xLoc = yLoc = 0;
    zoom = -300.0;
    slices = 16;
    subsampling = 0;
    vectorLength = 1.0f;
    vectorRadius = 0.5f;
    vectorTipLengthRatio = 0.4f;
    vectorShaftRadiusRatio = 0.4f;

    // Slicing and Thresholding
    xSliceLow=ySliceLow=zSliceLow=thresholdLow=0;
    xSliceHigh=ySliceHigh=zSliceHigh=thresholdHigh=16*100;

    // Map from display type to int

    display_type_map["Full Orientation"] = 1;
    display_type_map["In-Plane Angle"]   = 2;
    display_type_map["X Coordinate"]      = 3;
    display_type_map["Y Coordinate"]      = 4;
    display_type_map["Z Coordinate"]      = 5;

    // Load shader programs, lights, models, etc.
    context()->makeCurrent();

    // Draw at a fixed framerate (if updates are needed)
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(16);
}

QColor GLWidget::customSpriteColor(float value) {
    // Number of colors in the list
    int numColors = customColors.length();

    // Part of the mapping that value takes given this number of colors
    float interval = 1.0f / (static_cast<float>(numColors) - 1.0f);

    // Find which colors in the list we're in between
    int firstColorIndex = static_cast<int>(0.999f*value/interval);
    if (firstColorIndex > (numColors-2)) {
        firstColorIndex = numColors-2;
    }
    QColor firstColor = customColors.at(firstColorIndex);
    QColor secondColor = customColors.at(firstColorIndex + 1);

    // Rescale the value to within 0:1
    value = (value - firstColorIndex*interval)/interval;

    // Linear interpolation between the chosen colors
    qreal r = firstColor.redF() + value * (secondColor.redF() - firstColor.redF());
    qreal g = firstColor.greenF() + value * (secondColor.greenF() - firstColor.greenF());
    qreal b = firstColor.blueF() + value * (secondColor.blueF() - firstColor.blueF());

    return QColor::fromRgbF(r,g,b);
    
}

void GLWidget::setCustomColorScale(QList<QColor> colors)
{
    // Set the private color variables to the inputs
    customColors = colors;
}

void GLWidget::updateData(QSharedPointer<OMFReader> data)
{
    if (data.isNull()) {
        displayOn = false;
    } else {
        valuedim = data->valuedim;
        // Nothing set for the extents...
        if (valuedim == 3) {
            data->field->minmaxMagnitude(minmag, maxmag);
        } else if (valuedim == 1) {
            data->field->minmaxScalar(minmag, maxmag);
        }
        dataPtr    = data;
        displayOn  = true;
        // Update the display
        updateCOM();
        updateExtent();
        needsUpdate = true;

        pushBuffers();
    }
}

void GLWidget::pushLUT() {
    if (colorScale !=  "HSL") {
        QVector4D lut[256];
        for (int i=0; i<256; i++) {
            float h = ((float)i)/255.0;
            if (colorScale ==  ("Grayscale")) {
                spriteColor = QColor::fromHslF(0.0, 0.0, h);
            } else if (colorScale ==  ("Blue to Red")) {
                if (h <= 0.5) {
                    spriteColor = QColor::fromHsvF(0.0,1.0-2.0*h,1.0);
                } else {
                    spriteColor = QColor::fromHsvF(0.5,(h-0.5)*2.0,1.0);
                }
            } else if (colorScale ==  ("Green to White")) {
                spriteColor = QColor::fromHsvF(0.3,1.0-h,0.25+0.75*h);

            } else if (colorScale ==  ("Custom")) {
                spriteColor = customSpriteColor(h);
            }

            lut[i] = QVector4D(spriteColor.redF(), spriteColor.greenF(), spriteColor.blueF(), 0.0);
        }
        currentShader->setUniformValueArray("color_lut", lut, 256);
    }
} 

void GLWidget::pushBuffers()
{
    if (displayOn) {
        QVector<int> size = dataPtr->field->shape();
        // int numNodes = dataPtr->field->num_elements();
        int incr_x = ((1 << subsampling) > size[0]) ? size[0] : (1 << subsampling);
        int incr_y = ((1 << subsampling) > size[1]) ? size[1] : (1 << subsampling);
        int incr_z = ((1 << subsampling) > size[2]) ? size[2] : (1 << subsampling);

        // numNodes  = size[0]/incr_x;
        // numNodes *= size[1]/incr_y;
        // numNodes *= size[2]/incr_z;
        numNodes = 0;
        
        // Push new data
        for(int i=0; i<size[0]; i+=incr_x) {
            for(int j=0; j<size[1]; j+=incr_y) {
                for(int k=0; k<size[2]; k+=incr_z) {
                    instPositions << QVector4D((float)i,(float)j,(float)k,0.0);
                    instMagnetizations << QVector4D(dataPtr->field->at(i,j,k), 0.0);
                    numNodes++;
                }   
            }
        }

        if ( numNodes <= 1) { 
            subsampling --;
        }

        currentShader->bind();
        displayObject->vao->bind();

        // Buffers for coordinates and colors
        displayObject->pos_vbo.bind();
        displayObject->pos_vbo.allocate( numNodes * sizeof(QVector4D) );
        displayObject->pos_vbo.write(0, instPositions.constData(), numNodes * sizeof(QVector4D));
        
        displayObject->mag_vbo.bind();
        displayObject->mag_vbo.allocate( numNodes * sizeof(QVector4D) );
        displayObject->mag_vbo.write(0, instMagnetizations.constData(), numNodes * sizeof(QVector4D));

        // Release buffers
        displayObject->pos_vbo.release();
        displayObject->mag_vbo.release();
        displayObject->vao->release();
        
        // Clear Qt containers
        instPositions.clear();
        instMagnetizations.clear();
    }
}

void GLWidget::updateExtent()
{
    QVector<int> size = dataPtr->field->shape();
    xmax = size[0];
    ymax = size[1];
    zmax = size[2];
    xmin = 0.0;
    ymin = 0.0;
    zmin = 0.0;
}

void GLWidget::update() {
    if (needsUpdate) {
        pushBuffers();
        updateGL();
        needsUpdate = false;
        emit doneRenderingFrame(filename);
    }
}

void GLWidget::renderFrame(QString file)
{
    filename = file;
    needsUpdate = true;
    update();
}

void GLWidget::initializeGL()
{
    qDebug() << "Widget OpenGl: " << format().majorVersion() << "." << format().minorVersion();
    qDebug() << "Context valid: " << context()->isValid();
    qDebug() << "Really used OpenGl: " << context()->format().majorVersion() << "." << context()->format().minorVersion();
    qDebug() << "OpenGl information: VENDOR:       " << (const char*)glGetString(GL_VENDOR);
    qDebug() << "                    RENDERDER:    " << (const char*)glGetString(GL_RENDERER);
    qDebug() << "                    VERSION:      " << (const char*)glGetString(GL_VERSION);
    qDebug() << "                    GLSL VERSION: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    // gl330Funcs = 0;
    // gl330Funcs = context()->versionFunctions<QOpenGLFunctions_3_3_Core>();
    gl330Funcs = new QOpenGLFunctions_3_3_Core;
    if (gl330Funcs)
        gl330Funcs->initializeOpenGLFunctions();
    else
    {
        qWarning() << "Could not obtain required OpenGL context version";
        exit(1);
    }

    initializeAssets();

    QGLFormat glFormat = QGLWidget::format();
    if ( !glFormat.sampleBuffers() )
        qWarning() << "Could not enable sample buffers";

    // Set the clear color to black
    backgroundColor = QColor::fromRgbF(0.9, 0.8, 1.0).dark();
    qglClearColor( backgroundColor );

    glEnable( GL_MULTISAMPLE );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    glEnable( GL_SMOOTH );
    glDepthFunc( GL_LEQUAL );
}

void GLWidget::resizeGL( int w, int h )
{
    // Set the viewport to window dimensions
    glViewport( 0, 0, w, qMax( h, 1 ) );
    qreal aspect = qreal(w) / qreal(h ? h : 1);
    projection.setToIdentity();
    projection.perspective(45.0f,aspect,0.1f,10000.0f);

    needsUpdate = true;
}

void GLWidget::paintGL()
{
    // Clear the buffer with the current clearing color
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    if (displayOn) {

        QVector<int> size = dataPtr->field->shape();
        // int numNodes = dataPtr->field->num_elements();
        sprite *tempSprite;
        QOpenGLShaderProgram *tempShader;

        GLfloat thrLo = ((GLfloat)thresholdLow)/1600.0;
        GLfloat thrHi = ((GLfloat)thresholdHigh)/1600.0;
        GLfloat xSlLo = (xmax-xmin)*(GLfloat)xSliceLow/1600.0;
        GLfloat xSlHi = (xmax-xmin)*(GLfloat)xSliceHigh/1600.0;
        GLfloat ySlLo = (ymax-ymin)*(GLfloat)ySliceLow/1600.0;
        GLfloat ySlHi = (ymax-ymin)*(GLfloat)ySliceHigh/1600.0;
        GLfloat zSlLo = (zmax-zmin)*(GLfloat)zSliceLow/1600.0;
        GLfloat zSlHi = (zmax-zmin)*(GLfloat)zSliceHigh/1600.0;
        GLfloat sc    = (GLfloat)(1 << subsampling);
        view.setToIdentity();
        view.translate(xLoc, yLoc, zoom);
        view.rotate(xRot / 1600.0, 1.0, 0.0, 0.0);
        view.rotate(yRot / 1600.0, 0.0, 1.0, 0.0);
        view.rotate(zRot / 1600.0, 0.0, 0.0, 1.0);

        if (valuedim == 1 ) {
            tempSprite = &cube;
            tempShader = &cubeShader;
        } else {
            tempSprite = displayObject;
            tempShader = currentShader;
        }

        tempShader->bind();
        tempShader->setUniformValue("view",              view);
        tempShader->setUniformValue("projection",        projection);
        tempShader->setUniformValue("brightness",        brightness);
        tempShader->setUniformValue("light.position",    lightPosition);
        tempShader->setUniformValue("light.intensities", lightIntensity);
        tempShader->setUniformValue("ambient",           lightAmbient);
        tempShader->setUniformValue("brightness",        brightness);
        tempShader->setUniformValue("maxmag",            maxmag);
        tempShader->setUniformValue("thresholdLow",      thrLo);
        tempShader->setUniformValue("thresholdHigh",     thrHi);
        tempShader->setUniformValue("xSliceLow",         xSlLo);
        tempShader->setUniformValue("xSliceHigh",        xSlHi);
        tempShader->setUniformValue("ySliceLow",         ySlLo);
        tempShader->setUniformValue("ySliceHigh",        ySlHi);
        tempShader->setUniformValue("zSliceLow",         zSlLo);
        tempShader->setUniformValue("zSliceHigh",        zSlHi);
        tempShader->setUniformValue("display_type",      display_type_map[coloredQuantity]);
        tempShader->setUniformValue("use_color_lut",     (colorScale !=  "HSL") ? 1 : 0);
        tempShader->setUniformValue("com",               QVector3D(xcom, ycom, zcom));
        tempShader->setUniformValue("do_rotate",         (displayObject == &cube) ? 0 : 1);
        tempShader->setUniformValue("valuedim",          valuedim);
        tempShader->setUniformValue("scale",             sc);

        // Vertex Array 
        tempSprite->vao->bind();

        // Buffers for coordinates and colors
        tempSprite->pos_vbo.bind();
        tempSprite->mag_vbo.bind();

        // Draw everything in one call
        gl330Funcs->glDrawArraysInstanced( GL_TRIANGLES, 0, tempSprite->count, numNodes);

        // Release buffers
        tempSprite->pos_vbo.release();
        tempSprite->mag_vbo.release();
        tempSprite->vao->release();          
    }
}

void GLWidget::toggleDisplay(int type)
{
    displayType = type;
    if (displayType == 0) {
        displayObject = &cube;
        currentShader = &cubeShader;
    } else if (displayType == 1) {
        displayObject = &cone;
        currentShader = &standardShader;
    } else {
        displayObject = &vect;
        currentShader = &standardShader;
    }
    needsUpdate = true;
}

void GLWidget::setBackgroundColor(QColor color) {
    backgroundColor = color;
    qglClearColor(backgroundColor);
    needsUpdate = true;
}

void GLWidget::setSpriteDimensions(int newslices, float length, float radius, float tipLengthRatio, float shaftRadiusRatio, QString origin)
{
    if ( (slices != newslices) || (vectorLength != length) || (vectorRadius != radius) ||
         (vectorTipLengthRatio != tipLengthRatio) || (vectorShaftRadiusRatio != shaftRadiusRatio) ||
         (vectorOrigin != origin) )
    {
        slices = newslices;
        vectorLength = length;
        vectorRadius = radius;
        vectorTipLengthRatio = tipLengthRatio;
        vectorShaftRadiusRatio = shaftRadiusRatio;
        vectorOrigin = origin;
        initializeVect(slices, 5.0f*vectorLength, 1.0f*vectorRadius, vectorTipLengthRatio, vectorShaftRadiusRatio);
        initializeCone(slices, 1.0*vectorRadius, 2.0*vectorLength);
        needsUpdate = true;
    }
}

void GLWidget::setBrightness(float bright)
{
    brightness = bright;
    needsUpdate = true;
}

void GLWidget::setColoredQuantity(QString value)
{
    coloredQuantity = value;
}

void GLWidget::setColorScale(QString value)
{
    colorScale = value;
    pushLUT();
}

void GLWidget::setSpriteScale(QString value)
{
    spriteScale = value;
}
