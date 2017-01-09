#include <QtGui>
#include "glwidget.h"

void GLWidget::updateCOM()
{
    QVector<int> size = dataPtr->field->shape();
    xcom = (float)size[0]*0.5;
    ycom = (float)size[1]*0.5;
    zcom = (float)size[2]*0.5;
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
    (void) e;
    leftMousePressed = false;
    middleMousePressed = false;
    rightMousePressed = false;
    needsUpdate=true;
}

void GLWidget::mouseMoveEvent(QMouseEvent *e)
{
    QVector2D diff = QVector2D(e->localPos()) - previousMousePosition;

    if (e->buttons() & Qt::RightButton) {
        setXRotation(xRot + 800 * diff.y());
        setYRotation(yRot + 800 * diff.x());
      } else if (e->buttons() & Qt::LeftButton) {
        setXRotation(xRot + 800 * diff.y());
        setZRotation(zRot + 800 * diff.x());
      } else if (e->buttons() & Qt::MidButton) {
        setXLoc(xLoc + 0.2*diff.x());
        setYLoc(yLoc - 0.2*diff.y());
      }

    previousMousePosition = QVector2D(e->localPos());

}

void GLWidget::wheelEvent(QWheelEvent *e)
{
    if(e->orientation() == Qt::Vertical)
    {
         zoom += (float)(e->delta()) / 50;
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

void GLWidget::setThresholdLow(int low)
{
    if (thresholdLow != low) {
        thresholdLow = low;
        needsUpdate = true;
    }
}

void GLWidget::setThresholdHigh(int high)
{
    if (thresholdLow != high) {
        thresholdHigh = high;
        needsUpdate = true;
    }
}

static void qNormalizeAngle(int &angle)
{
  while (angle < 0)
    angle += 360 * 1600;
  while (angle > 360 * 1600)
    angle -= 360 * 1600;
}

void GLWidget::setXRotation(int angle)
{
  qNormalizeAngle(angle);
  if (angle != xRot) {
    xRot = angle;
    emit xRotationChanged(angle);
    needsUpdate = true;
  }
}

void GLWidget::setYRotation(int angle)
{
  qNormalizeAngle(angle);
  if (angle != yRot) {
    yRot = angle;
    emit yRotationChanged(angle);
    needsUpdate = true;
  }
}

void GLWidget::setZRotation(int angle)
{
  qNormalizeAngle(angle);
  if (angle != zRot) {
    zRot = angle;
    emit zRotationChanged(angle);
    needsUpdate = true;
  }
}

void GLWidget::setXLoc(float val)
{
  if (xLoc != val) {
    xLoc = val;
    emit COMChanged(val);
    needsUpdate = true;
  }
}

void GLWidget::setYLoc(float val)
{
  if (yLoc != val) {
    yLoc = val;
    emit COMChanged(val);
    needsUpdate = true;
  }
}

void GLWidget::setZLoc(float val)
{
  if (zLoc != val) {
    zLoc = val;
    emit COMChanged(val);
    needsUpdate = true;
  }
}

void GLWidget::increaseSubsampling()
{
    subsampling++;
    needsUpdate = true;
}

void GLWidget::decreaseSubsampling()
{
    if (subsampling > 1) {
        subsampling--;
        needsUpdate = true;
    }
}

void GLWidget::setXCom(float val)
{
  if (xcom != val) {
    xcom = val;
    emit COMChanged(val);
    needsUpdate = true;
  }
}

void GLWidget::setYCom(float val)
{
  if (ycom != val) {
    ycom = val;
    emit COMChanged(val);
    needsUpdate = true;
  }
}

void GLWidget::setZCom(float val)
{
  if (zcom != val) {
    zcom = val;
    emit COMChanged(val);
    needsUpdate = true;
  }
}
