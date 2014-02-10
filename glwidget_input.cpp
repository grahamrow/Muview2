#include <QtGui>
#include <glwidget.h>

void GLWidget::updateCOM()
{
    const long unsigned int *size = dataPtr->shape();
    xcom = (float)size[0]*0.5;
    ycom = (float)size[1]*0.5;
    zcom = (float)size[2]*0.5;
//    qDebug() << "COM:" << xcom << ycom << zcom ;
}

void GLWidget::keyPressEvent( QKeyEvent* e )
{
    switch ( e->key() )
    {
        case Qt::Key_Escape:
            QCoreApplication::instance()->quit();
            break;
        default:
            QGLWidget::keyPressEvent( e );
    }
}
void GLWidget::mousePressEvent(QMouseEvent *e)
{
    previousMousePosition = QVector2D(e->localPos());
    if (e->buttons() & Qt::LeftButton) {
        leftMousePressed = true;
    } else if (e->buttons() & Qt::MidButton) {
        middleMousePressed = true;
    } else if (e->buttons() & Qt::RightButton) {
        rightMousePressed = true;
    }
}

void GLWidget::mouseReleaseEvent(QMouseEvent *e)
{
    leftMousePressed = false;
    middleMousePressed = false;
    rightMousePressed = false;
    needsUpdate=true;
}

void GLWidget::mouseMoveEvent(QMouseEvent *e)
{
    QVector2D diff = QVector2D(e->localPos()) - previousMousePosition;
    QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();

    if (leftMousePressed) {
        rotation = QQuaternion::fromAxisAndAngle(n, diff.length()) * rotation;
    } else if (middleMousePressed) {
        translation.translate(diff.x()/20.0, -diff.y()/20.0, 0.0);
    } else if (rightMousePressed) {
        rotation = QQuaternion::fromAxisAndAngle(QVector3D(0.0,0.0,1.0), -diff.y()/20.0) * rotation;
    }

//    if (leftMousePressed || middleMousePressed || rightMousePressed )
    previousMousePosition = QVector2D(e->localPos());
    needsUpdate = true;
}

void GLWidget::wheelEvent(QWheelEvent *e)
{
    if(e->orientation() == Qt::Vertical)
    {
        translation.translate(0,0,e->delta()/40.0);
    }
    needsUpdate = true;
}

void GLWidget::setXSliceLow(int low)
{
    if (xSliceLow != low) {
        xSliceLow = low;
        needsUpdate = true;
    }
}

void GLWidget::setXSliceHigh(int high)
{
    if (xSliceLow != high) {
        xSliceHigh = high;
        needsUpdate = true;
    }
}

void GLWidget::setYSliceLow(int low)
{
    if (ySliceLow != low) {
        ySliceLow = low;
        needsUpdate = true;
    }
}

void GLWidget::setYSliceHigh(int high)
{
    if (ySliceLow != high) {
        ySliceHigh = high;
        needsUpdate = true;
    }
}

void GLWidget::setZSliceLow(int low)
{
    if (zSliceLow != low) {
        zSliceLow = low;
        needsUpdate = true;
    }
}

void GLWidget::setZSliceHigh(int high)
{
    if (zSliceLow != high) {
        zSliceHigh = high;
        needsUpdate = true;
    }
}
