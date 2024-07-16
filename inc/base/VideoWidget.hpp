#pragma once
#include <QKeyEvent>
#include <QVideoWidget>


class VideoWidget : public QVideoWidget
{
    Q_OBJECT

public:
    VideoWidget(QWidget *parent = nullptr);

public: signals:
    void keyPress(QKeyEvent *);
    void mousePress(QMouseEvent *);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
};