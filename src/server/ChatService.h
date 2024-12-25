#pragma once
#include <unordered_map>
#include <functional>
#include <mutex>
#include "TcpConnection.h"
#include "userModel.h"
#include "OfflineMsgModel.h"
#include "FriendModel.h"
#include "GroupModel.h"
#include "CacheManager.h"
#include "json.hpp"

using namespace net;
using json = nlohmann::json;

using MsgHandler = std::function<void(const TcpConnectionPtr&, json&, Timestamp)>;

class ChatService {
public:
    static ChatService* instance(){
        static ChatService service;
        return &service;
    }

    void login(const TcpConnectionPtr& conn, json& js, Timestamp time);

    void reg(const TcpConnectionPtr& conn, json& js, Timestamp time);

    void oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time);

    void addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time);

    void createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);

    void addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);

    void groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time);

    void loginout(const TcpConnectionPtr& conn, json& js, Timestamp time);

    void clientCloseException(const TcpConnectionPtr& conn);

    void reset();

    MsgHandler getHandler(int msgid);

    void handleRedisSubscribeMessage(int userid, string msg);

private:
    ChatService();

    unordered_map<int, MsgHandler>          m_msgHandlerMap;

    unordered_map<int, TcpConnectionPtr>    m_userConnMap;

    mutex                                   m_connMutex;

    UserModel                               m_userModel;
    OfflineMsgModel                         m_offlineMsgModel;
    FriendModel                             m_friendModel;
    GroupModel                              m_groupModel;

    CacheManager m_redis;
};
