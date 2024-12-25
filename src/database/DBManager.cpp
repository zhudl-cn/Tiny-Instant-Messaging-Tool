#include "DBManager.h"
#include "AsyncLog.h"
#include <iostream>


static std::string server = "127.0.0.1";
static uint16_t port = 3306;
static std::string user = "root";
static std::string password = "123456";
static std::string dbname = "chat";

DBManager::DBManager() {
    m_conn = mysql_init(nullptr);
}

DBManager::~DBManager() {
    if (m_conn != nullptr)
        mysql_close(m_conn);
}

bool DBManager::connect() {
    MYSQL* p = mysql_real_connect(m_conn, server.c_str(), user.c_str(),
        password.c_str(), dbname.c_str(), port, nullptr, 0);
    if (p != nullptr) {
        // Default is ASCII, set to gbk is to display non English characters
        mysql_query(m_conn, "set names gbk");
    } else {
        std::cout << "connect mysql fail!" << mysql_error(m_conn) << std::endl;
    }

    return p;
}

bool DBManager::update(std::string sql) {
    if (mysql_query(m_conn, sql.c_str())) {
        std::cout << sql << ": update failed" << std::endl;
        return false;
    }

    return true;
}

MYSQL_RES* DBManager::query(std::string sql) {
    if (mysql_query(m_conn, sql.c_str())) {
        std::cout << sql << ": query failed!" << std::endl;
        return nullptr;
    }

    return mysql_use_result(m_conn);
}

MYSQL* DBManager::getConnection() {
    return m_conn;
}
