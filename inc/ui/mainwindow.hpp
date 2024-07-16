#pragma once
#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "base/VideoWidget.hpp"
#include <QTime>
#include <QTimer>
#include <QString>
#include <QStringListModel>
#include <QKeyEvent>
#include "ui/VideoDock.hpp"
#include "base/PlayerDB.hpp"
#include "ui/ListContextMenu.hpp"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow;}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    enum PlayMode
    {SINGLE, LIST, SINGLELOOP, LISTLOOP};
    struct PlayerMemo
    {
        qint64 audio_pos = 0;
        qint64 video_pos = 0;
        QModelIndex index;
        PlayMode play_mode = PlayMode::LIST;
    };

private:
    void init();

private slots:
    void play();
    void last();
    void next();
    void select(const QModelIndex &index);
    void refresh_time();
    void open_files();
    void open_folder();
    void show_list();
    void open_list();

    void change_pos();
    void change_played_time_by_slider_moved(const int &value);
    void reset_list(const QStringList *files, const QStringList *paths, const int &type);
    void set_player_rate(const double &value);
    void store_state();
    void recover_state();
    void append_to(const QString& list);

    void playPositionChangedEvent(const qint64 &pos);
    void tabWidgetChangedEvent(const int& index);
    void mediaStatusChangedEvent(const QMediaPlayer::MediaStatus &status);
    void listMenuEvent(const int& code);
    void keyPressEvent(QKeyEvent *event);
    void videoWidgetKeyPressEvent(QKeyEvent *event);
    void videoWidgetMousePressEvent(QMouseEvent *event);
    void videoDockOrderEvent(const int &order);
    void playModeChangedEvent();

private:
    Ui::MainWindow *ui;
    QMediaPlayer *_player;
    QAudioOutput *_audio_output;
    VideoWidget *_video_widget;
    VideoDock *_video_dock;
    QTimer _timer;
    QTime _time;
    PlayerDB *_db;
    ListContextMenu *_menu;

    PlayerMemo _memo;
    QStringList _audio_path;
    QStringList _video_path;
    QStringList _audio_list;
    QStringListModel *_audio_model;
    QStringList _video_list;
    QStringListModel *_video_model;
    QModelIndex _index;
    QPoint _hover_pos;
    PlayMode _play_mode = PlayMode::LIST;
};