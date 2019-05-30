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
    subsampling = 1;
    vectorLength = 1.0f;
    vectorRadius = 0.5f;
    vectorTipLengthRatio = 0.4f;
    vectorShaftRadiusRatio = 0.4f;

    // Slicing and Thresholding
    xSliceLow=ySliceLow=zSliceLow=thresholdLow=0;
    xSliceHigh=ySliceHigh=zSliceHigh=thresholdHigh=16*100;

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

        QVector<int> size = dataPtr->field->shape();
        int numNodes = dataPtr->field->num_elements();
        // Push new data
        for(int i=0; i<size[0]; i+=subsampling) {
            for(int j=0; j<size[1]; j+=subsampling) {
                for(int k=0; k<size[2]; k+=subsampling) {
                    instPositions << QVector4D((float)i,(float)j,(float)k,0.0);
                    instMagnetizations << QVector4D(dataPtr->field->at(i,j,k), 0.0);
                }   
            }
        }

        diffuseShader.bind();
        displayObject->vao->bind();

        // Buffers for coordinates and colors
        displayObject->pos_vbo.bind();
        if (displayObject->pos_vbo.size() != numNodes * sizeof(QVector4D)) {
            qDebug() << "Reallocating translation buffer to size:" << numNodes << "---" << displayObject->pos_vbo.size();
            displayObject->pos_vbo.allocate( numNodes * sizeof(QVector4D) );
        }
        displayObject->pos_vbo.write(0, instPositions.constData(), numNodes * sizeof(QVector4D));
        

        displayObject->mag_vbo.bind();
        if (displayObject->mag_vbo.size() != numNodes * sizeof(QVector4D)) {
            qDebug() << "Reallocating magnetization buffer to size:" << numNodes;
            displayObject->mag_vbo.allocate( numNodes * sizeof(QVector4D) );
        }
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

    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    glEnable(GL_SMOOTH);
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

// void GLWidget::updateGeom()
// {

// }


void GLWidget::paintGL()
{
    // Clear the buffer with the current clearing color
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    if (displayOn) {

        QVector<int> size = dataPtr->field->shape();
        int xnodes = size[0];
        int ynodes = size[1];
        int znodes = size[2];
        int numNodes = dataPtr->field->num_elements();

        GLfloat thrLo = ((GLfloat)thresholdLow)/1600.0;
        GLfloat thrHi = ((GLfloat)thresholdHigh)/1600.0;
        GLfloat xSlLo = (xmax-xmin)*(GLfloat)xSliceLow/1600.0;
        GLfloat xSlHi = (xmax-xmin)*(GLfloat)xSliceHigh/1600.0;
        GLfloat ySlLo = (ymax-ymin)*(GLfloat)ySliceLow/1600.0;
        GLfloat ySlHi = (ymax-ymin)*(GLfloat)ySliceHigh/1600.0;
        GLfloat zSlLo = (zmax-zmin)*(GLfloat)zSliceLow/1600.0;
        GLfloat zSlHi = (zmax-zmin)*(GLfloat)zSliceHigh/1600.0;
        view.setToIdentity();
        view.translate(xLoc, yLoc, zoom);
        view.rotate(xRot / 1600.0, 1.0, 0.0, 0.0);
        view.rotate(yRot / 1600.0, 0.0, 1.0, 0.0);
        view.rotate(zRot / 1600.0, 0.0, 0.0, 1.0);

        diffuseShader.bind();
        diffuseShader.setUniformValue("view",              view);
        diffuseShader.setUniformValue("projection",        projection);
        diffuseShader.setUniformValue("brightness",        brightness);
        diffuseShader.setUniformValue("light.position",    lightPosition);
        diffuseShader.setUniformValue("light.intensities", lightIntensity);
        diffuseShader.setUniformValue("ambient",           lightAmbient);
        diffuseShader.setUniformValue("brightness",        brightness);
        diffuseShader.setUniformValue("maxmag",            maxmag);
        diffuseShader.setUniformValue("thresholdLow",      thrLo);
        diffuseShader.setUniformValue("thresholdHigh",     thrHi);
        diffuseShader.setUniformValue("xSliceLow",         xSlLo);
        diffuseShader.setUniformValue("xSliceHigh",        xSlHi);
        diffuseShader.setUniformValue("ySliceLow",         ySlLo);
        diffuseShader.setUniformValue("ySliceHigh",        ySlHi);
        diffuseShader.setUniformValue("zSliceLow",         zSlLo);
        diffuseShader.setUniformValue("zSliceHigh",        zSlHi);
        diffuseShader.setUniformValue("display_type",      5);
        diffuseShader.setUniformValue("com",               QVector3D(xcom, ycom, zcom));

        // for(int i=0; i<xnodes; i+=subsampling) {
        //     for(int j=0; j<ynodes; j+=subsampling) {
        //         for(int k=0; k<znodes; k+=subsampling) {
        //             instPositions << QVector4D((float)i,(float)j,(float)k,0.0);
        //             instMagnetizations << QVector4D(dataPtr->field->at(i,j,k), 0.0);
        //         }   
        //     }
        // }
        // Vertex Array 
        displayObject->vao->bind();

        // // Buffers for coordinates and colors
        displayObject->pos_vbo.bind();
        // if (displayObject->pos_vbo.size() != numNodes * sizeof(QVector4D)) {
        //     qDebug() << "Reallocating translation buffer to size:" << numNodes << "---" << displayObject->pos_vbo.size();
        //     displayObject->pos_vbo.allocate( numNodes * sizeof(QVector4D) );
        // }
        // displayObject->pos_vbo.write(0, instPositions.constData(), numNodes * sizeof(QVector4D));
        

        displayObject->mag_vbo.bind();
        // if (displayObject->mag_vbo.size() != numNodes * sizeof(QVector4D)) {
        //     qDebug() << "Reallocating magnetization buffer to size:" << numNodes;
        //     displayObject->mag_vbo.allocate( numNodes * sizeof(QVector4D) );
        // }
        // displayObject->mag_vbo.write(0, instMagnetizations.constData(), numNodes * sizeof(QVector4D));

        // Draw everything in one call
        gl330Funcs->glDrawArraysInstanced( GL_TRIANGLES, 0, displayObject->count, numNodes);

        // Release buffers
        displayObject->pos_vbo.release();
        displayObject->mag_vbo.release();
        displayObject->vao->release();
        
        // // Clear Qt containers
        // instPositions.clear();
        // instMagnetizations.clear();

        //                 if (colorScale ==  ("HSL")) {
        //                     spriteColor = QColor::fromHslF(hueVal, 1.0, lumVal);
        //                 } else if (colorScale ==  ("Grayscale")) {
        //                     spriteColor = QColor::fromHslF(0.0, 0.0, hueVal);
        //                 } else if (colorScale ==  ("Blue to Red")) {
        //                     if (hueVal <= 0.5) {
        //                         spriteColor = QColor::fromHsvF(0.0,1.0-2.0*hueVal,1.0);
        //                     } else {
        //                         spriteColor = QColor::fromHsvF(0.5,(hueVal-0.5)*2.0,1.0);
        //                     }
        //                 } else if (colorScale ==  ("Custom")) {
        //                     spriteColor = customSpriteColor(hueVal);
        //                 } else {
        //                     spriteColor = QColor::fromRgbF(0.0,0.0,0.0);
        //                 }

        //                 model = globalMovement;
        //                 model.translate(((float)i-xcom)*2.0,((float)j-ycom)*2.0,((float)k-zcom)*2.0);
        //                 if (displayType == 0) {
        //                     // Don't rotate the cubes, but do expand them to fill empty space...
        //                     model.scale(xnodes > subsampling ? (float)subsampling : 1.0f,
        //                                 ynodes > subsampling ? (float)subsampling : 1.0f,
        //                                 znodes > subsampling ? (float)subsampling : 1.0f);
        //                 } else {
        //                     // Rotate the cones or vectors, but don't mess with their aspect ratios...
        //                     float scale = (spriteScale == "Proportional") ? relmag : 1.0f;
        //                     model.scale((1.0f + (float)subsampling*0.4f)*scale);
        //                     model.rotate(180.0*(phi+0.5*PI)/PI, 0.0, 0.0, 1.0);
        //                     model.rotate(180.0*theta/PI,        1.0, 0.0, 0.0);
        //                 }

        //                 if (valuedim == 1 ) {
        //                     tempObject = &cube;
        //                 } else {
        //                     tempObject = displayObject;
        //                 }

        //                 if (tempObject == &cube ) {
        //                     flatShader.bind();
        //                     flatShader.setUniformValue("model",      model);
        //                     flatShader.setUniformValue("view",       view);
        //                     flatShader.setUniformValue("projection", projection);
        //                     flatShader.setUniformValue("color",      spriteColor);
        //                     flatShader.setUniformValue("brightness", brightness);
        //                 } else {
        //                     diffuseShader.bind();
        //                     diffuseShader.setUniformValue("model",             model);
        //                     diffuseShader.setUniformValue("view",              view);
        //                     diffuseShader.setUniformValue("projection",        projection);
        //                     diffuseShader.setUniformValue("color",             spriteColor);
        //                     diffuseShader.setUniformValue("light.position",    lightPosition);
        //                     diffuseShader.setUniformValue("light.intensities", lightIntensity);
        //                     diffuseShader.setUniformValue("ambient",           lightAmbient);
        //                     diffuseShader.setUniformValue("brightness",        brightness);
        //                 }

        //                 tempObject->vao->bind();

        //                 glDrawArrays( GL_TRIANGLES, 0, tempObject->count );

        //             }
        //         }
        //     }
        // }
    }
}

void GLWidget::toggleDisplay(int type)
{
    displayType = type;
    if (displayType == 0) {
        displayObject = &cube;
    } else if (displayType == 1) {
        displayObject = &cone;
    } else {
        displayObject = &vect;
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
}

void GLWidget::setSpriteScale(QString value)
{
    spriteScale = value;
}
