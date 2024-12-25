#include "OfflineMsgModel.h"
#include "DBManager.h"

void OfflineMsgModel::insert(int userid, std::string msg) {
    char sql[1024] = { 0 };
    snprintf(sql,sizeof sql,"insert into offlinemessage values('%d', '%s')", userid, msg.c_str());

    DBManager mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return;
        }
    }
}

void OfflineMsgModel::remove(int userid) {
    char sql[1024] = { 0 };
    sprintf(sql, "delete from offlinemessage where userid=%d", userid);

    DBManager mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return;
        }
    }
}

std::vector<std::string> OfflineMsgModel::query(int userid) {
    char sql[1024] = { 0 };
    sprintf(sql, "select message from offlinemessage where userid = %d", userid);

    std::vector<std::string> vec;
    DBManager mysql;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}