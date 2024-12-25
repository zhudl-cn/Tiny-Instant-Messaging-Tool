#pragma once
#include "User.h"
#include <vector>

class FriendModel {
public:
    void insert(int userid, int friendid);

    std::vector<User> query(int userid);
};

