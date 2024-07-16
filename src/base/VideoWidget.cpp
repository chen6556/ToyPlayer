#include "base/VideoWidget.hpp"


VideoWidget::VideoWidget(QWidget *parent)
    : QVideoWidget(parent) 
{
    QVideoWidget::setMouseTracking(true);
    this->setMouseTracking(true);
}

void VideoWidget::mousePressEvent(QMouseEvent *event)
{
    emit mousePress(event);
}

void VideoWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    this->setFullScreen(!this->isFullScreen());
}

void VideoWidget::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Escape:
        this->setFullScreen(false);
        break;
    default:
        emit keyPress(event);
        break;
    }
}