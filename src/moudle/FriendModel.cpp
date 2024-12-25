#include "FriendModel.h"
#include "DBManager.h"

void FriendModel::insert(int userid, int friendid) {
    char sql[1024] = { 0 };
    snprintf(sql, sizeof sql,  "insert into friend values('%d', '%d')", userid, friendid);

    DBManager mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
        }
    }
}

//query friend
std::vector<User> FriendModel::query(int userid) {
    char sql[1024] = { 0 };
    snprintf(sql, sizeof sql, 
        "select a.id, a.name, a.state from user a inner join friend b on b.friendid=a.id where b.userid=%d", userid);

    std::vector<User> vec;
    DBManager mysql;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                User user;
                user.setId(std::stoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}