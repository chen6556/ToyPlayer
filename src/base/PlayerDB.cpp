#include "base/PlayerDB.hpp"
#include <list>
#include <QFileInfo>


PlayerDB::PlayerDB()
    : DB()
{}

PlayerDB::PlayerDB(const char path[])
    : DB(path)
{
    init();
}

bool PlayerDB::init()
{
    if (add_table("dict", {"key", "value"}, {"TEXT", "TEXT"}))
    {
        build_list("stored_audio_list");
        build_list("stored_video_list");
        insert("dict", std::list<std::list<std::string>>({{"'audio_path'", "'C:/'"}, {"'video_path'", "'C:/'"},
                    {"audio_playmode", "1"}, {"video_playmode", "1"}, {"autoplay", "0"}, 
                    {"last_model_index", "-1"}, {"last_pos", "0"}, {"last_widget_index", "0"},
                    {"audio_rate", "1"}, {"video_rate", "1"}}));
        return true;
    }
    else
    {
        return false;
    }
}

bool PlayerDB::open(const char path[])
{
    if (DB::open(path))
    {
        init();
        return true;
    }
    else
    {
        return false;
    }
}

bool PlayerDB::close()
{
    return DB::close();
}

const QStringList PlayerDB::play_lists()
{
    std::vector<std::string> lists = tables();
    lists.erase(std::find(lists.cbegin(), lists.cend(), "dict"));
    QStringList temp;
    for (const std::string &s : lists)
    {
        temp.append(s.c_str());
    }
    return temp;
}

const QStringList PlayerDB::show_list(const QString &list)
{
    search_by_item(list.toStdString().c_str(), "alive", "1");
    QStringList temp;
    for (const std::map<std::string, std::string> &m : _result)
    {
        temp.append(m.at("file").c_str());
    }
    return temp;
}

void PlayerDB::set_param(const char key[], const char value[])
{
    if (count_item("dict", "key", key) == 0)
    {
        insert("dict", std::list<std::string>({key, value}));
    }
    else
    {
        update_item("dict", key, "value", value);
    }
}

const QString PlayerDB::get_param(const char key[])
{
    search_by_item("dict", "key", key);
    return _result.empty() ? QString() : QString::fromStdString(_result.back().at("value"));
}

const QMap<QString, QString> PlayerDB::get_params()
{
    search_by_item("dict", "alive", "1");
    QMap<QString, QString> result;
    for (const std::map<std::string, std::string>& row : _result)
    {
        result.insert(row.at("key").c_str(), row.at("value").c_str());
    }
    _result.clear();
    return result;
}

void PlayerDB::set_params(const QMap<QString, QString>& params)
{
    for (QKeyValueIterator<const QString &, const QString &, QMap<QString, QString>::const_iterator> it = params.constKeyValueBegin(), end = params.constKeyValueEnd(); it != end; ++it)
    {
        if (!update_item("dict", it->first.toStdString().c_str(), it->first.toStdString().c_str(), it->second.toStdString().c_str()))
        {
            insert("dict", {it->first.toStdString(), it->second.toStdString()});
        }   
    }
}



void PlayerDB::append_medias(const QString &list, const QStringList &files, const QStringList &paths)
{
    std::list<std::list<std::string>> temp;
    std::string sql("SELECT MAX(id) FROM ");
    sql.append(list.toStdString()).append(";");
    run_sql(sql);
    QStringList::const_iterator files_it = files.cbegin(), paths_it = paths.cbegin();
    size_t count = _result.front().at("MAX(id)").empty() ? 0 : std::stoull(_result.front().at("MAX(id)"));
    for (size_t i = 0, end = files.size(); i < end; ++i, ++files_it, ++paths_it)
    {
        if (!has_media(list, *files_it, *paths_it))
        {
            insert(list.toStdString().c_str(), {std::to_string(++count), ("'" + *files_it + "'").toStdString(), ("'" + *paths_it + "'").toStdString()});
        }
    }
}

void PlayerDB::get_medias(const QString &list, QStringList &files, QStringList &paths)
{
    search_by_item(list.toStdString().c_str(), "alive", "1");
    files.clear();
    paths.clear();
    for (const std::map<std::string, std::string> &row : _result)
    {
        files.append(row.at("name").c_str());
        paths.append(row.at("path").c_str());
    }
}

void PlayerDB::remove_media(const QString &list, const QString &file, const QString &path)
{
    remove(list.toStdString().c_str(), {"name", "path"}, {("'" + file + "'").toStdString(), ("'" + path + "'").toStdString()}, false);
}



void PlayerDB::build_list(const char list[])
{
    add_table(list, {"id", "name", "path"}, {"INTEGER", "TEXT", "TEXT"}, {"AUTOINCREMENT"});
}

void PlayerDB::remove_list(const char list[])
{
    drop(list);
}

void PlayerDB::rename_list(const char old_name[], const char new_name[])
{
    rename_table(old_name, new_name);
}

void PlayerDB::clear_list(const char list[])
{
    clear(list, false);
}

bool PlayerDB::has_media(const QString& list, const QString& file, const QString& path)
{
    return count_items(list.toStdString().c_str(), {"name", "path"}, {("'" + file + "'").toStdString(), ("'" + path + "'").toStdString()}) > 0;
}