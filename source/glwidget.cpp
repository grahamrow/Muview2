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

    // Slicing
    xSliceLow=ySliceLow=zSliceLow=0;
    xSliceHigh=ySliceHigh=zSliceHigh=16*100;

    // Load shader programs, lights, models, etc.
    context()->makeCurrent();
    initializeAssets();

    // Draw at a fixed framerate (if updates are needed)
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(16);
}

void GLWidget::updateData(QSharedPointer<matrix> data)
{
    if (data.isNull()) {
        displayOn = false;
    } else {
        dataPtr    = data;
        displayOn  = true;
        // Update the display
        updateCOM();
        updateExtent();
        needsUpdate = true;
    }
}

void GLWidget::updateHeader(QSharedPointer<OMFHeader> header, QSharedPointer<matrix> data)
{
    if (!data.isNull()) {
        valuedim = header->valuedim;
        // Nothing set for the extents...
        if (valuedim == 3) {
            data->minmaxMagnitude(minmag, maxmag);
        } else if (valuedim == 1) {
            data->minmaxScalar(minmag, maxmag);
        }
    }
}

void GLWidget::updateExtent()
{
    QVector<int> size = dataPtr->shape();
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

    view.setToIdentity();
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
        QVector<int> size = dataPtr->shape();
        int xnodes = size[0];
        int ynodes = size[1];
        int znodes = size[2];
//            qDebug() << xnodes << ynodes << znodes;
        QVector3D datum;
        float theta, phi, mag;
        float hueVal, lumVal;
        sprite *tempObject;

        QMatrix4x4 globalMovement;
        globalMovement.setToIdentity();
        globalMovement.translate(xLoc, yLoc, zoom);
        globalMovement.rotate(xRot / 1600.0, 1.0, 0.0, 0.0);
        globalMovement.rotate(yRot / 1600.0, 0.0, 1.0, 0.0);
        globalMovement.rotate(zRot / 1600.0, 0.0, 0.0, 1.0);

        for(int i=0; i<xnodes; i+=subsampling)
        {
            for(int j=0; j<ynodes; j+=subsampling)
            {
                for(int k=0; k<znodes; k+=subsampling)
                {
                    datum = dataPtr->at(i,j,k);
                    if (valuedim == 1) {
                        mag = datum.x();
                    } else {
                        mag = datum.length();
                    }

                    if ( ((valuedim == 1 ) || ((valuedim == 3) && mag != 0.0)) &&
                         i >= (xmax-xmin)*(float)xSliceLow/1600.0 &&
                         i <= (xmax-xmin)*(float)xSliceHigh/1600.0 &&
                         j >= (ymax-ymin)*(float)ySliceLow/1600.0 &&
                         j <= (ymax-ymin)*(float)ySliceHigh/1600.0 &&
                         k >= (zmax-zmin)*(float)zSliceLow/1600.0 &&
                         k <= (zmax-zmin)*(float)zSliceHigh/1600.0)
                    {

                        theta = acos(datum.z()/mag);
                        phi   = atan2(datum.y(), datum.x());
//                        qDebug() << theta << phi << mag;

                        if (valuedim == 1) {
                            if (maxmag!=minmag) {
                                phi = PI*(-0.999+1.998*(mag-minmag)/(maxmag-minmag));
                            } else {
                                phi = 0.0f;
                            }
                        }

                        lumVal = 0.5;
                        if (coloredQuantity == ("In-Plane Angle")) {
                            hueVal = (phi+PI)/(2.0f*PI);
                        } else if (coloredQuantity ==  ("Full Orientation")) {
                            hueVal = (phi+PI)/(2.0f*PI);
                            if (hueVal < 0.0f) hueVal = 0.0f;
                            if ((hueVal > 1.0f) || (hueVal < 0.0f))
                            {
                                qDebug() << "Hue out of range:" << hueVal << datum.x() << datum.y() << datum.z() << theta << phi;
                            }
                            if (valuedim == 1) {
                                lumVal=0.5;
                            } else {
                                lumVal = 0.5 + 0.5*datum.z()/mag;
                            }
                        } else if (coloredQuantity ==  ("X Coordinate")) {
                            hueVal = 0.5+ 0.5*datum.x()/mag;
                        } else if (coloredQuantity ==  ("Y Coordinate")) {
                            hueVal = 0.5+ 0.5*datum.y()/mag;
                        } else if (coloredQuantity ==  ("Z Coordinate")) {
                            hueVal = 0.5+ 0.5*datum.z()/mag;
                        } else {
                            hueVal = 0.0;
                        }

                        if (colorScale ==  ("HSL")) {
                            spriteColor = QColor::fromHslF(hueVal, 1.0, lumVal);
                        } else if (colorScale ==  ("Grayscale")) {
                            spriteColor = QColor::fromHslF(0.0, 0.0, hueVal);
                        } else if (colorScale ==  ("Blue to Red")) {
                            if (hueVal <= 0.5) {
                                spriteColor = QColor::fromHsvF(0.0,1.0-2.0*hueVal,1.0);
                            } else {
                                spriteColor = QColor::fromHsvF(0.5,(hueVal-0.5)*2.0,1.0);
                            }
                        } else {
                            spriteColor = QColor::fromRgbF(0.0,0.0,0.0);
                        }

                        model = globalMovement;
                        model.translate(((float)i-xcom)*2.0,((float)j-ycom)*2.0,((float)k-zcom)*2.0);
                        if (displayType == 0) {
                            // Don't rotate the cubes, but do expand them to fill empty space...
                            model.scale(xnodes > subsampling ? (float)subsampling : 1.0f,
                                        ynodes > subsampling ? (float)subsampling : 1.0f,
                                        znodes > subsampling ? (float)subsampling : 1.0f);
                        } else {
                            // Rotate the cones or vectors, but don't mess with their aspect ratios...
                            model.scale(1.0f + (float)subsampling*0.4f);
                            model.rotate(180.0*(phi+0.5*PI)/PI, 0.0, 0.0, 1.0);
                            model.rotate(180.0*theta/PI,        1.0, 0.0, 0.0);
                        }

                        if (valuedim == 1 ) {
                            tempObject = &cube;
                        } else {
                            tempObject = displayObject;
                        }

                        if (tempObject == &cube ) {
                            flatShader.bind();
                            flatShader.setUniformValue("model",      model);
                            flatShader.setUniformValue("view",       view);
                            flatShader.setUniformValue("projection", projection);
                            flatShader.setUniformValue("color",      spriteColor);
                            flatShader.setUniformValue("brightness", brightness);
                        } else {
                            diffuseShader.bind();
                            diffuseShader.setUniformValue("model",             model);
                            diffuseShader.setUniformValue("view",              view);
                            diffuseShader.setUniformValue("projection",        projection);
                            diffuseShader.setUniformValue("color",             spriteColor);
                            diffuseShader.setUniformValue("light.position",    lightPosition);
                            diffuseShader.setUniformValue("light.intensities", lightIntensity);
                            diffuseShader.setUniformValue("ambient",           lightAmbient);
                            diffuseShader.setUniformValue("brightness",        brightness);
                        }

                        tempObject->vao->bind();

                        glDrawArrays( GL_TRIANGLES, 0, tempObject->count );

                    }
                }
            }
        }
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

void GLWidget::setSpriteDimensions(int newslices, float length, float radius, float tipLengthRatio, float shaftRadiusRatio)
{
    if ( (slices != newslices) || (vectorLength != length) || (vectorRadius != radius) ||
         (vectorTipLengthRatio != tipLengthRatio) || (vectorShaftRadiusRatio != shaftRadiusRatio) )
    {
        slices = newslices;
        vectorLength = length;
        vectorRadius = radius;
        vectorTipLengthRatio = tipLengthRatio;
        vectorShaftRadiusRatio = shaftRadiusRatio;
        initializeVect(slices, 5.0f*vectorLength, 1.0f*vectorRadius, vectorTipLengthRatio, vectorShaftRadiusRatio);
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
