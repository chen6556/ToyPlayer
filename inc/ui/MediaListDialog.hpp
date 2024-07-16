#pragma once
#include <QDialog>
#include <QStringListModel>
#include "base/PlayerDB.hpp"
#include "ui/ListContextMenu.hpp"
#include <QPoint>


QT_BEGIN_NAMESPACE
namespace Ui { class MediaListDialog; }
QT_END_NAMESPACE

class MediaListDialog: public QDialog
{
    Q_OBJECT

public:
    MediaListDialog(QWidget *parent = nullptr);
    ~MediaListDialog();

public: signals:
    void list_selected(const QStringList*, const QStringList*, const int&);

private:
    void init();

private slots:
    void new_list();
    void append_files();
    void drop_list();
    void rename_list(const QModelIndex &index);
    void show_details(const QModelIndex &index);
    
    void listMenuEvent(const int& code);

public:
    void new_list(const int& index, const QStringList& files, const QStringList& paths);

private:
    Ui::MediaListDialog *ui;
    PlayerDB *_db;
    ListContextMenu *_menu;

    QStringList _audio_path;
    QStringList _video_path;
    QStringList _audio_list;
    QStringList _audio_details_list;
    QStringListModel *_audio_model;
    QStringListModel *_audio_details_model;
    QStringList _video_list;
    QStringList _video_details_list;
    QStringListModel *_video_model;
    QStringListModel *_video_details_model;
    QModelIndex _index;
    QPoint _hover_pos;
};