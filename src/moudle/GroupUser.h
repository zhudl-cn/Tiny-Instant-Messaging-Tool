#pragma once
#include "User.h"
#include <string>

class GroupUser :
    public User {
public:
    void setRole(std::string role) { this->role = role; }
    std::string getRole() { return this->role; }

private:
    std::string role;
};

