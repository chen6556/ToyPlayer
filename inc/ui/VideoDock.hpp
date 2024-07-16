#pragma once
#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QDoubleSpinBox>


QT_BEGIN_NAMESPACE
namespace Ui { class VideoDock; }
QT_END_NAMESPACE

class VideoDock: public QWidget
{
    Q_OBJECT

public:
    VideoDock(QWidget *parent = nullptr);
    ~VideoDock();

    QLabel *tatol_time_label();
    QLabel *played_time_label();
    QSlider *slider();
    QDoubleSpinBox *rate();

public: signals:
    void order(const int&);
    /*
        1: last_button
        2: play_button
        3: next_button
        4: full_screen_button
    */

private:
    void init();

private:
    Ui::VideoDock *ui;
    
};