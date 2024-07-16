#include "ui/VideoDock.hpp"
#include "./ui_VideoDock.h"


VideoDock::VideoDock(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VideoDock)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);

    init();
}

VideoDock::~VideoDock()
{
    delete ui;
}

void VideoDock::init()
{
    QObject::connect(ui->last_button, &QPushButton::clicked, this, [this](){emit order(1);});
    QObject::connect(ui->play_button, &QPushButton::clicked, this, [this](){emit order(2);});
    QObject::connect(ui->next_button, &QPushButton::clicked, this, [this](){emit order(3);});
    QObject::connect(ui->full_screen_button, &QToolButton::clicked, this, [this](){emit order(4);});
}

QLabel *VideoDock::tatol_time_label()
{
    return ui->total_time;
}

QLabel *VideoDock::played_time_label()
{
    return ui->played_time;
}

QSlider *VideoDock::slider()
{
    return ui->slider;
}

QDoubleSpinBox *VideoDock::rate()
{
    return ui->rate;
}