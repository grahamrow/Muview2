#include "preferences.h"
#include "ui_preferences.h"
#include <QColorDialog>
#include <QColor>

Preferences::Preferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Preferences)
{
    ui->setupUi(this);
    backgroundColor = QColor::fromRgbF(0.9, 0.8, 1.0).dark();
    ui->colorBox->setPalette(QPalette(backgroundColor));
    connect(ui->setBackgroundColor, SIGNAL(clicked()), this, SLOT(setcolor()));
}

void Preferences::setcolor()
{
    QColor color = QColorDialog::getColor(backgroundColor, this);
    if (color.isValid())
    {
        ui->colorBox->setPalette(QPalette(color));
        ui->colorBox->setAutoFillBackground(true);
        backgroundColor = color;
    }
}

QColor Preferences::getBackgroundColor() {
    return backgroundColor;
}

Preferences::~Preferences()
{
    delete ui;
}

int Preferences::getSpriteResolution()
{
    return ui->spriteResolution->value();
}

float Preferences::getVectorLength()
{
    return ((float)ui->vectorLength->value())/100.f;
}

float Preferences::getVectorRadius()
{
    return ((float)ui->vectorRadius->value())/100.f;
}

float Preferences::getVectorTipToTail()
{
    return ((float)ui->tipPercent->value())/100.f;
}

float Preferences::getVectorInnerToOuter()
{
    return ((float)ui->shaftRadiusPercent->value())/100.f;
}

float Preferences::getBrightness()
{
    return ((float)ui->brightnessSlider->value())/50.0;
}

QString Preferences::getFormat()
{
    return ui->imageFormat->currentText();
}

QSize Preferences::getImageDimensions()
{
    if (ui->fixedSize->isChecked()) {
        return QSize(ui->imageWidth->value(), ui->imageHeight->value());
    } else {
        return QSize(-1,-1);
    }
}

QString Preferences::getColorScale()
{
    return ui->colorScaleGroup->checkedButton()->text();
}

QString Preferences::getColorQuantity()
{
    return ui->coloredQuantityGroup->checkedButton()->text();
}

QString Preferences::getVectorOrigin()
{
    return ui->vectorOriginGroup->checkedButton()->text();
}
