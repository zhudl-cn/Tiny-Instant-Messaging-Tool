#pragma once
#include "GroupUser.h"
#include <string>
#include <vector>

class Group {
public:
    Group() = default;
    Group(int id, std::string name, std::string desc);

    void setId(int id) { m_id = id; }
    void setName(std::string name) { m_name = name; }
    void setDesc(std::string desc) { m_desc = desc; }

    int getId() { return m_id; }
    std::string getName() { return m_name; }
    std::string getDesc() { return m_desc; }
    std::vector<GroupUser>& getUsers() { return m_users; }

private:
    int                     m_id{-1};
    std::string             m_name{""};
    std::string             m_desc{""};
    std::vector<GroupUser>  m_users;
};

