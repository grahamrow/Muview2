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


float Preferences::getBrightness()
{
    return ((float)ui->brightnessSlider->value())/50.0;
}
