#pragma once
#include "DB.hpp"
#include <QStringList>
#include <QModelIndex>

class PlayerDB: protected DB
{
public:
    PlayerDB();
    PlayerDB(const char path[]);

    bool open(const char path[]);
    bool close();
    bool init();

    const QStringList play_lists();
    const QStringList show_list(const QString &list);
    void set_param(const char key[], const char value[]);
    const QString get_param(const char key[]);
    const QMap<QString, QString> get_params();
    void set_params(const QMap<QString, QString>& params);

    void append_medias(const QString &list, const QStringList &files, const QStringList &paths);
    void get_medias(const QString &list, QStringList &files, QStringList &paths);
    void remove_media(const QString &list, const QString &file, const QString &path);

    void build_list(const char list[]);
    void remove_list(const char list[]);
    void rename_list(const char old_name[], const char new_name[]);
    void clear_list(const char list[]);

    bool has_media(const QString& list, const QString& file, const QString& path);

};