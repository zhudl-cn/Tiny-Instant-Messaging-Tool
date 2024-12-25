#pragma once
#include "mysql.h"
#include <string>

class DBManager {
public:
    DBManager();
    ~DBManager();

    bool connect();

    bool update(std::string sql);

    MYSQL_RES* query(std::string sql);

    MYSQL* getConnection();

private:
    MYSQL*  m_conn;
};

