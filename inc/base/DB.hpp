#pragma once
#include "base/sqlite3.h"
#include <string>
#include <vector>
#include <map>
#include <list>


class DB
{
protected:
    sqlite3* _db = nullptr;
    char* _msg = nullptr;
    std::string _sql;
    std::wstring _wsql;
    std::vector<std::map<std::string, std::string>> _result;

    static int query(void* data, int argc, char** values, char** items);
    static int get_items(void* data, int argc, char** values, char** items);

public:
    DB();
    DB(const char path[]);
    virtual ~DB();

    bool init(const char path[], const char table_name[], const std::list<std::string> &items, const std::list<std::string> &types, const std::list<std::string> &constraints = {});
    bool open(const char path[]);
    bool close();
    const std::vector<std::map<std::string, std::string>>& result() const;
    const bool has_table(const char table[]);
    const char* message() const;

    bool add_table(const char name[], const std::list<std::string> &items, const std::list<std::string> &types, const std::list<std::string> &constraints = {});
    bool add_column(const char table[], const std::list<std::string> &items, const std::list<std::string> &types, const std::list<std::string> &constraints = {});
 
    bool insert(const char table[], const std::list<std::string> &values, const bool alive = true);
    bool insert(const char table[], const std::list<std::string> &items, const std::list<std::string> &values, const bool alive = true);
    bool insert(const char table[], const std::list<std::list<std::string>> &values, const bool alive = true);
    bool insert(const char table[], const std::list<std::list<std::string>> &items, const std::list<std::list<std::string>> &values, const bool alive = true);

    bool search_by_item(const char table[], const char item[], const char value[], const bool alive_only = true);
    bool search_by_items(const char table[], const std::list<std::string> &items, const std::list<std::string> &values, const bool alive_only = true);
    const std::vector<std::string> tables();
    const std::vector<std::string> items(const char table[]);

    bool update_item(const char table[], const char primary_key[],  const char item[], const char value[]);
    bool update_items(const char table[], const char primary_key[], const std::list<std::string> &items, const std::list<std::string> &values);

    bool drop(const char table[]);
    bool clear(const char table[], const bool pseudo = true);
    bool remove(const char table[], const char primary_key[], const bool pseudo = true);
    bool remove(const char table[], const char item[], const char value[], const bool pseudo = true);
    bool remove(const char table[], const std::list<std::string> &items, const std::list<std::string> &values, const bool pseudo = true);

    bool active(const char table[], const char primary_key[]);
    bool active(const char table[], const char item[], const char value[]);
    bool active(const char table[], const std::list<std::string> &items, const std::list<std::string> &values);

    const size_t count_item(const char table[], const char item[], const char value[]);
    const size_t count_items(const char table[], const std::list<std::string> &items, const std::list<std::string> &values);

    bool rename_table(const char table[], const char name[]);

    bool run_sql(const char sql[]);
    bool run_sql(const std::string& sql);

    std::string max(const char table[], const char item[]);
    std::string min(const char table[], const char item[]);
    std::string avg(const char table[], const char item[]);
    std::string sum(const char table[], const char item[]);
}; 