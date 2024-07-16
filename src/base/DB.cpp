#include "base/DB.hpp"


DB::DB(){}

DB::DB(const char path[])
{
    open(path);
}

DB::~DB()
{
    close();
}

bool DB::init(const char path[], const char table_name[], const std::list<std::string> &items, const std::list<std::string> &types, const std::list<std::string> &constraints)
{
    if (items.size() != types.size())
    {
        return false;
    }

    const size_t length = items.size();
    bool ans = sqlite3_open(path, &_db) == SQLITE_OK;
    if (!ans || length == 0 || this->has_table(table_name))
    {
        return ans;
    }

    std::list<std::string>::const_iterator items_it = items.begin(), types_it = types.begin(), constraints_it = constraints.begin(), constraints_end = constraints.end();
    _sql.clear();
    _sql = "CREATE TABLE ";
    _sql.append(table_name).append("(alive INTEGER DEFAULT 1,");

    if (constraints_it == constraints_end)
    {
        _sql.append(*items_it++).append(" ").append(*types_it++).append(" PRIMARY KEY NOT NULL UNIQUE");
    }
    else
    {
        _sql.append(*items_it++).append(" ").append(*types_it++).append(" ").append(*constraints_it++).append(" PRIMARY KEY NOT NULL UNIQUE");
    }
    if (length > 1)
    {
        _sql.append(",");
    }

    for (size_t i = 2; i < length; ++i)
    {
        if (constraints_it == constraints_end)
        {
            _sql.append(*items_it++).append(" ").append(*types_it++).append(",");
        }
        else
        {
            _sql.append(*items_it++).append(" ").append(*types_it++).append(" ").append(*constraints_it++).append(",");
        }
    }

    if (items_it != items.end())
    {
        if (constraints_it == constraints_end)
        {
            _sql.append(*items_it).append(" ").append(*types_it);
        }
        else
        {
            _sql.append(*items_it).append(" ").append(*types_it).append(" ").append(*constraints_it);
        }
    }
    _sql.append(");");

    return sqlite3_exec(_db, _sql.c_str(), 0, 0, &_msg) == SQLITE_OK;
}

bool DB::open(const char path[])
{
    if (_db != nullptr)
    {
        sqlite3_close(_db);
    }
    return sqlite3_open(path, &_db) == SQLITE_OK;
}

bool DB::close()
{
    if (sqlite3_close(_db) == SQLITE_OK)
    {
        _db = nullptr;
        return true;
    }
    else
    {
        return false;
    }
}

const std::vector<std::map<std::string, std::string>>& DB::result() const
{
    return _result;
}

bool DB::add_table(const char name[], const std::list<std::string> &items, const std::list<std::string> &types, const std::list<std::string> &constraints)
{
    const size_t length = items.size();
    if (_db == nullptr || length < 1 || length != types.size()|| this->has_table(name))
    {
        return false;
    }
    
    std::list<std::string>::const_iterator items_it = items.begin(), types_it = types.begin(), constraints_it = constraints.begin(), constraints_end = constraints.end();
    _sql.clear();
    _sql = "CREATE TABLE ";
    _sql.append(name).append(" (alive INTEGER DEFAULT 1,");

    if (constraints_it == constraints_end)
    {
        _sql.append(*items_it++).append(" ").append(*types_it++).append(" PRIMARY KEY NOT NULL UNIQUE");
    }
    else
    {
        _sql.append(*items_it++).append(" ").append(*types_it++).append(" PRIMARY KEY ").append(*constraints_it++).append(" NOT NULL UNIQUE");
    }
    if (length > 1)
    {
        _sql.append(",");
    }

    for (size_t i = 2; i < length; ++i)
    {
        if (constraints_it == constraints_end)
        {
            _sql.append(*items_it++).append(" ").append(*types_it++).append(",");
        }
        else
        {
            _sql.append(*items_it++).append(" ").append(*types_it++).append(" ").append(*constraints_it++).append(",");
        }
    }

    if (items_it != items.end())
    {
        if (constraints_it == constraints_end)
        {
            _sql.append(*items_it).append(" ").append(*types_it);
        }
        else
        {
            _sql.append(*items_it).append(" ").append(*types_it).append(" ").append(*constraints_it);
        }
    }
    _sql.append(");");

    return sqlite3_exec(_db, _sql.c_str(), 0, 0, &_msg) == SQLITE_OK;
}

bool DB::add_column(const char table[], const std::list<std::string> &items, const std::list<std::string> &types, const std::list<std::string> &constraints)
{
    size_t length = items.size(), count = 1;
    if (_db == nullptr || length == 0 || length != types.size()|| !this->has_table(table))
    {
        return false;
    }
    const std::vector<std::string> item_list = this->items(table);
    if (std::find(item_list.cbegin(), item_list.cend(), std::string(table)) != item_list.cend())
    {
        return false;
    }

    std::list<std::string>::const_iterator items_it = items.begin(), types_it = types.begin();
    _sql.clear();
    _sql = "ALTER TABLE ";
    _sql.append(table).append(" ADD COLUMN ");
    _sql.append(*items_it++).append(" ").append(*types_it++);
    for (size_t i = 1; i < length; ++i)
    {
        _sql.append(",").append(*items_it++).append(" ").append(*types_it++);
    }
    _sql.append(";");

    return sqlite3_exec(_db, _sql.c_str(), 0, 0, &_msg) == SQLITE_OK;
}

bool DB::insert(const char table[], const std::list<std::string> &values, const bool alive)
{
    size_t count = 0, length = values.size();
    if (_db == nullptr || length < 1|| !this->has_table(table) || this->count_item(table, this->items(table).at(1).c_str(), values.begin()->c_str()) > 0)
    {
        return false;
    }

    _sql.clear();
    _sql = "INSERT INTO ";
    _sql.append(table).append(" VALUES (").append(alive ? "1," : "0,");

    for (const std::string &value : values)
    {
        _sql.append(value);
        if (++count < length)
        {
            _sql.append(", ");
        }
        else
        {
            _sql.append(");");
        }
    }

    return sqlite3_exec(_db, _sql.c_str(), 0, 0, &_msg) == SQLITE_OK;
}

bool DB::insert(const char table[], const std::list<std::string> &items, const std::list<std::string> &values, const bool alive)
{
    const size_t length = items.size();
    if (_db == nullptr || length < 1 || length != values.size()|| !this->has_table(table) || this->count_items(table, items, values) > 0)
    {
        return false;
    }

    _sql.clear();
    _sql = "INSERT INTO ";
    _sql.append(table).append(" (alive");
    std::string temp = alive ? ") VALUES (1" : ") VALUES (0";

    for (const std::string &item : items)
    {
        _sql.append(",").append(item);
    }
    for (const std::string &value : values)
    {
        temp.append(",").append(value);
    }
    _sql.append(temp).append(");");

    return sqlite3_exec(_db, _sql.c_str(), 0, 0, &_msg) == SQLITE_OK;
}

bool DB::insert(const char table[], const std::list<std::list<std::string>> &values, const bool alive)
{
    size_t length = values.size();
    if (_db == nullptr || length < 1|| !this->has_table(table))
    {
        return false;
    }

    _sql.clear();
    _sql = "INSERT INTO ";
    _sql.append(table).append(" VALUES ");

    size_t count, end = values.size(), index = 0;
    for (const std::list<std::string> &row : values)
    {
        length = row.size(), count = 0;
        _sql.append(alive ? "(1, " : "(0, ");
        for (const std::string &value : row)
        {   
            _sql.append(value);
            if (++count < length)
            {
                _sql.append(", ");
            }
            else
            {
                _sql.append(++index < end ? ")," : ");");
            }
        }
    }

    return sqlite3_exec(_db, _sql.c_str(), 0, 0, &_msg) == SQLITE_OK;
}

bool DB::insert(const char table[], const std::list<std::list<std::string>> &items, const std::list<std::list<std::string>> &values, const bool alive)
{
    const size_t length = items.size();
    if (_db == nullptr || length < 1 || length != values.size()|| !this->has_table(table))
    {
        return false;
    }

    _sql.clear();
    _sql = "INSERT INTO ";
    _sql.append(table);
    std::string temp;

    std::list<std::list<std::string>>::const_iterator items_row = items.begin(), values_row = values.begin();
    for (size_t i = 0, end = length; i < end; ++i, ++items_row, ++values_row)
    {
        temp = alive ? ") VALUES (1" : ") VALUES (0";
        _sql.append(" (alive");
        for (const std::string item : *items_row)
        {
            _sql.append(",").append(item);
        }
        for (const std::string value : *values_row)
        {
            temp.append(",").append(value);
        }
        _sql.append(temp).append(i < end - 1 ? ")," : ");");
    }

    return sqlite3_exec(_db, _sql.c_str(), 0, 0, &_msg) == SQLITE_OK;
}

int DB::query(void* data, int argc, char** values, char** items)
{
    std::map<std::string, std::string> temp;
    for (int i = 0; i < argc; ++i)
    {
        temp.insert(std::make_pair<std::string, std::string>(items[i], values[i] == NULL ? "" : values[i]));
    }
    static_cast<std::vector<std::map<std::string, std::string>>*>(data)->push_back(temp);

    return 0;
}

int DB::get_items(void* data, int argc, char** values, char** items)
{
    for (int i = 0; i < argc; ++i)
    {
        if (std::strncmp(items[i], "name", 4) == 0)
        {
            static_cast<std::vector<std::string>*>(data)->push_back(values[i]);
            return 0;
        }
    }
    return -1;
}

bool DB::search_by_item(const char table[], const char item[], const char value[], const bool alive_only)
{
    if (_db == nullptr || !this->has_table(table))
    {
        return false;
    }

    _sql.clear();
    _sql = "SELECT * FROM ";
    _sql.append(table).append(alive_only ? " WHERE alive = 1 AND " : " WHERE ").append(item).append(" = ").append(value).append(";");

    _result.clear();
    return sqlite3_exec(_db, _sql.c_str(), &DB::query, &_result, &_msg) == SQLITE_OK;
}

bool DB::search_by_items(const char table[], const std::list<std::string> &items, const std::list<std::string> &values, const bool alive_only)
{
    if (_db == nullptr || items.size() != values.size() || !this->has_table(table))
    {
        return false;
    }

    _sql.clear();
    _sql = "SELECT * FROM ";
    _sql.append(table).append(alive_only ? " WHERE (alive = 1 AND " : " WHERE (");

    std::list<std::string>::const_iterator items_it = items.begin(), values_it = values.begin();
    _sql.append(*items_it++).append(" = ").append(*values_it++);
    for (size_t i = 1, end = items.size(); i < end; ++i)
    {
        _sql.append(" AND ").append(*items_it++).append(" = ").append(*values_it++);
    }
    _sql.append(";");

    _result.clear();
    return sqlite3_exec(_db, _sql.c_str(), &DB::query, &_result, &_msg) == SQLITE_OK;
}

const std::vector<std::string> DB::tables()
{
    std::vector<std::string> res;
    if (_db == nullptr)
    {
        return res;
    }

    _sql.clear();
    _sql = "SELECT name FROM sqlite_master WHERE type = 'table' ORDER BY name;";

    _result.clear();
    if (sqlite3_exec(_db, _sql.c_str(), &DB::query, &_result, &_msg) == SQLITE_OK)
    {
        for (const std::map<std::string, std::string>& m : _result)
        {
            res.push_back(m.begin()->second);
        }
    }
    res.erase(std::find(res.begin(), res.end(), "sqlite_sequence"));
    return res;
}

const std::vector<std::string> DB::items(const char table[])
{
    std::vector<std::string> res;
    if (_db == nullptr || !this->has_table(table))
    {
        return res;
    }

    _sql.clear();
    _sql = "PRAGMA  table_info (";
    _sql.append(table).append(");");

    sqlite3_exec(_db, _sql.c_str(), &DB::get_items, &res, &_msg);    
    return res;
}

const bool DB::has_table(const char table[])
{
    if (_db == nullptr)
    {
        return false;
    }

    std::vector<std::string> res;
    _sql.clear();
    _sql = "SELECT name FROM sqlite_master WHERE type = 'table' ORDER BY name;";

    _result.clear();
    if (sqlite3_exec(_db, _sql.c_str(), &DB::query, &_result, &_msg) == SQLITE_OK)
    {
        for (const std::map<std::string, std::string>& m : _result)
        {
            res.push_back(m.begin()->second);
        }
    }

    _result.clear();
    return std::find(res.cbegin(), res.cend(), std::string(table)) != res.cend();
}

const char* DB::message() const
{
    return _msg;
}

bool DB::update_item(const char table[], const char primary_key[], const char item[], const char value[])
{
    if (_db == nullptr || !this->has_table(table) || this->count_item(table, this->items(table).at(1).c_str(), primary_key) == 0)
    {
        return false;
    }

    const std::string key = this->items(table).at(1);

    _sql.clear();
    _sql = "UPDATE ";
    _sql.append(table).append(" SET ").append(item).append(" = ").append(value);
    _sql.append(" WHERE alive = 1 AND ").append(key).append(" = ").append(primary_key).append(";");
    
    return sqlite3_exec(_db, _sql.c_str(), 0, 0, &_msg) == SQLITE_OK;
}

bool DB::update_items(const char table[], const char primary_key[], const std::list<std::string> &items, const std::list<std::string> &values)
{
    const size_t length = items.size();
    if (_db == nullptr || length < 1 || !this->has_table(table) || this->count_item(table, this->items(table).at(1).c_str(), primary_key) == 0)
    {
        return false;
    }

    const std::string key = this->items(table).at(1);

    _sql.clear();
    _sql = "UPDATE ";
    _sql.append(table).append(" SET (");

    std::list<std::string>::const_iterator items_it = items.begin(), values_it = values.begin();
    _sql.append(*items_it++).append(" = ").append(*values_it++);
    for (size_t i = 1; i < length; ++i)
    {
        _sql.append(",").append(*items_it++).append(" = ").append(*values_it++);
    }
    _sql.append("FROM ").append(table).append(" WHERE alive = 1 AND ").append(key).append(" = ").append(primary_key).append(";");
    
    return sqlite3_exec(_db, _sql.c_str(), 0, 0, &_msg) == SQLITE_OK;
}

bool DB::drop(const char table[])
{
    if (_db == nullptr || !this->has_table(table))
    {
        return false;
    }

    _sql.clear();
    _sql = "DROP TABLE ";
    _sql.append(table).append(";");

    return sqlite3_exec(_db, _sql.c_str(), 0, 0, &_msg) == SQLITE_OK;
}

bool DB::clear(const char table[], const bool pseudo)
{
    if (_db == nullptr || !this->has_table(table))
    {
        return false;
    }

    _sql.clear();
    if (pseudo)
    {
        _sql = "UPDATE ";
        _sql.append(table).append(" SET alive = 0;");
    }
    else
    {
        _sql = "DELETE FROM ";
        _sql.append(table).append(";");
    }

    return sqlite3_exec(_db, _sql.c_str(), 0, 0, &_msg) == SQLITE_OK;
}

bool DB::remove(const char table[], const char primary_key[], const bool pseudo)
{
    if (_db == nullptr || !this->has_table(table))
    {
        return false;
    }

    const std::string key = this->items(table).at(1);
    _sql.clear();

    if (pseudo)
    {
        _sql = "UPDATE ";
        _sql.append(table).append(" SET alive = 0 WHERE ").append(key).append(" = ").append(primary_key).append(";");
    }
    else
    {
        _sql = "DELETE FROM ";
        _sql.append(table).append(" WHERE ").append(key).append(" = ").append(primary_key).append(";");
    }

    return sqlite3_exec(_db, _sql.c_str(), 0, 0, &_msg) == SQLITE_OK;
}

bool DB::remove(const char table[], const char item[], const char value[], const bool pseudo)
{
    if (_db == nullptr || !this->has_table(table))
    {
        return false;
    }

    _sql.clear();
    if (pseudo)
    {
        _sql = "UPDATE ";
        _sql.append(table).append(" SET alive = 0 WHERE ").append(item).append(" = ").append(value).append(";");
    }
    else
    {
        _sql = "DELETE FROM";
        _sql.append(table).append(" WHERE ").append(item).append(" = ").append(value).append(";");
    }

    return sqlite3_exec(_db, _sql.c_str(), 0, 0, &_msg) == SQLITE_OK;
}

bool DB::remove(const char table[], const std::list<std::string> &items, const std::list<std::string> &values, const bool pseudo)
{
    const size_t length = items.size();
    if (_db == nullptr || length < 1 || values.size() != length || !this->has_table(table))
    {
        return false;
    }

    std::list<std::string>::const_iterator items_it = items.begin(), values_it = values.begin();
    _sql.clear();

    if (pseudo)
    {
        _sql = "UPDATE ";
        _sql.append(table).append(" SET alive = 0 WHERE ").append(*items_it++).append(" = ").append(*values_it++);   
    }
    else
    {
        _sql = "DELETE FROM ";
        _sql.append(table).append(" WHERE ").append(*items_it++).append(" = ").append(*values_it++);
    }
    for (size_t i = 1; i < length; ++i)
    {
        _sql.append(" AND ").append(*items_it++).append(" = ").append(*values_it++);
    }
    _sql.append(";");
    
    return sqlite3_exec(_db, _sql.c_str(), 0, 0, &_msg) == SQLITE_OK;
}

bool DB::active(const char table[], const char primary_key[])
{
    if (_db == nullptr || !this->has_table(table))
    {
        return false;
    }

    const std::string key = this->items(table).at(1);

    _sql.clear();
    _sql = "UPDATE ";
    _sql.append(table).append(" SET alive = 1 WHERE ").append(key).append(" = ").append(primary_key).append(";");

    return sqlite3_exec(_db, _sql.c_str(), 0, 0, &_msg) == SQLITE_OK;
}

bool DB::active(const char table[], const char item[], const char value[])
{
    if (_db == nullptr || !this->has_table(table))
    {
        return false;
    }

    _sql.clear();
    _sql = "UPDATE ";
    _sql.append(table).append(" SET alive = 1 WHERE ").append(item).append(" = ").append(value).append(";");

    return sqlite3_exec(_db, _sql.c_str(), 0, 0, &_msg) == SQLITE_OK;
}

bool DB::active(const char table[], const std::list<std::string> &items, const std::list<std::string> &values)
{
    const size_t length = items.size();
    if (_db == nullptr || length != values.size() || !this->has_table(table))
    {
        return false;
    }

    std::list<std::string>::const_iterator items_it = items.begin(), values_it = values.begin();
    _sql.clear();
    _sql = "UPDATE ";
    _sql.append(table).append(" SET alive = 1 WHERE ").append(*items_it++).append(" = ").append(*values_it++);
    for (size_t i = 1; i < length; ++i)
    {
        _sql.append(" AND ").append(*items_it++).append(" = ").append(*values_it++);
    }
    _sql.append(";");

    return sqlite3_exec(_db, _sql.c_str(), 0, 0, &_msg) == SQLITE_OK;
}

const size_t DB::count_item(const char table[], const char item[], const char value[])
{
    if (_db == nullptr || !this->has_table(table))
    {
        return 0;
    }

    _sql.clear();
    _sql = "SELECT COUNT(*) FROM ";
    _sql.append(table).append(" WHERE ").append(item).append(" = ").append(value).append(";");

    _result.clear();
    if (sqlite3_exec(_db, _sql.c_str(), &query, &_result, &_msg) == SQLITE_OK)
    {
        return std::atoll(_result.front().cbegin()->second.c_str());
    }
    else
    {
        return 0;
    }
}

const size_t DB::count_items(const char table[], const std::list<std::string> &items, const std::list<std::string> &values)
{
    const size_t length = items.size();
    if (_db == nullptr || length != values.size() || !this->has_table(table))
    {
        return 0;
    }

    std::list<std::string>::const_iterator items_it = items.begin(), values_it = values.begin();
    _sql.clear();
    _sql = "SELECT COUNT(*) FROM ";
    _sql.append(table).append(" WHERE ").append(*items_it++).append(" = ").append(*values_it++);

    for (size_t i = 1; i < length; ++i)
    {
        _sql.append(" AND ").append(*items_it++).append(" = ").append(*values_it++);
    }
    _sql.append(";");

    _result.clear();
    if (sqlite3_exec(_db, _sql.c_str(), &query, &_result, &_msg) == SQLITE_OK)
    {
        return std::atoll(_result.front().cbegin()->second.c_str());
    }
    else
    {
        return 0;
    }
}

bool DB::rename_table(const char table[], const char name[])
{
    if (_db == nullptr || !this->has_table(table))
    {
        return false;
    }

    _sql = "ALTER TABLE ";
    _sql.append(table).append(" RENAME TO ").append(name).append(";");

    return sqlite3_exec(_db, _sql.c_str(), 0, 0, &_msg) == SQLITE_OK;
}

bool DB::run_sql(const char sql[])
{
    _result.clear();
    return sqlite3_exec(_db, sql, &query, &_result, &_msg) == SQLITE_OK;
}

bool DB::run_sql(const std::string& sql)
{
    _result.clear();
    return sqlite3_exec(_db, sql.c_str(), &query, &_result, &_msg) == SQLITE_OK;
}

std::string DB::max(const char table[], const char item[])
{
    _result.clear();
    _sql = "SELECT MAX(";
    _sql.append(item).append(") FROM ").append(table).append(";");
    if (sqlite3_exec(_db, _sql.c_str(), &query, &_result, &_msg) == SQLITE_OK)
    {   
        std::string key("MAX(");
        key.append(item).append(")");
        if (!_result.empty())
        {
            return _result.back().at(key);
        }
        else
        {
            return std::string();
        }
    }
    else
    {
        return std::string();
    }
}

std::string DB::min(const char table[], const char item[])
{
    _result.clear();
    _sql = "SELECT MIN(";
    _sql.append(item).append(") FROM ").append(table).append(";");
    if (sqlite3_exec(_db, _sql.c_str(), &query, &_result, &_msg) == SQLITE_OK)
    {   
        std::string key("MIN(");
        key.append(item).append(")");
        if (!_result.empty())
        {
            return _result.back().at(key);
        }
        else
        {
            return std::string();
        }
    }
    else
    {
        return std::string();
    }
}

std::string DB::avg(const char table[], const char item[])
{
    _result.clear();
    _sql = "SELECT AVG(";
    _sql.append(item).append(") FROM ").append(table).append(";");
    if (sqlite3_exec(_db, _sql.c_str(), &query, &_result, &_msg) == SQLITE_OK)
    {   
        std::string key("AVG(");
        key.append(item).append(")");
        if (!_result.empty())
        {
            return _result.back().at(key);
        }
        else
        {
            return std::string();
        }
    }
    else
    {
        return std::string();
    }
}

std::string DB::sum(const char table[], const char item[])
{
    _result.clear();
    _sql = "SELECT SUM(";
    _sql.append(item).append(") FROM ").append(table).append(";");
    if (sqlite3_exec(_db, _sql.c_str(), &query, &_result, &_msg) == SQLITE_OK)
    {   
        std::string key("SUM(");
        key.append(item).append(")");
        if (!_result.empty())
        {
            return _result.back().at(key);
        }
        else
        {
            return std::string();
        }
    }
    else
    {
        return std::string();
    }
}







