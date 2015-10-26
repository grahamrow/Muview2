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
    connect(ui->setBackgroundColor, SIGNAL(clicked()), this, SLOT(setColor()));
    connect(ui->setCustomColor1, SIGNAL(clicked()), this, SLOT(setCustomColor1()));
    connect(ui->setCustomColor2, SIGNAL(clicked()), this, SLOT(setCustomColor2()));
    connect(ui->setCustomColor3, SIGNAL(clicked()), this, SLOT(setCustomColor3()));
}

void Preferences::setColor()
{
    QColor color = QColorDialog::getColor(backgroundColor, this);
    if (color.isValid())
    {
        ui->colorBox->setPalette(QPalette(color));
        ui->colorBox->setAutoFillBackground(true);
        backgroundColor = color;
    }
}

void Preferences::setCustomColor1()
{
    QColor color = QColorDialog::getColor(customColor1, this);
    if (color.isValid())
    {
        ui->customColorBox1->setPalette(QPalette(color));
        ui->customColorBox1->setAutoFillBackground(true);
        customColor1 = color;
    }
}

void Preferences::setCustomColor2()
{
    QColor color = QColorDialog::getColor(customColor2, this);
    if (color.isValid())
    {
        ui->customColorBox2->setPalette(QPalette(color));
        ui->customColorBox2->setAutoFillBackground(true);
        customColor2 = color;
    }
}

void Preferences::setCustomColor3()
{
    QColor color = QColorDialog::getColor(customColor3, this);
    if (color.isValid())
    {
        ui->customColorBox3->setPalette(QPalette(color));
        ui->customColorBox3->setAutoFillBackground(true);
        customColor3 = color;
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

QString Preferences::getSpriteScale()
{
    return ui->spriteScaleGroup->checkedButton()->text();
}
