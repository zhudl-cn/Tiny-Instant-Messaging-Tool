#pragma once
#include "Sockets.h"
#include "TcpConnection.h"
#include "AsyncLog.h"
#include "Timestamp.h"
#include "Group.h"
#include "User.h"
#include "Msg.h"
//#include "Convertor.h"
#include "json.hpp"

#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <atomic>
#include <thread>
#include <functional>
#include <vector>
#include <iostream>

using namespace net;
using json = nlohmann::json;

class ChatClient {
public:
	ChatClient(InetAddress addr);
	~ChatClient() = default;

	SOCKET fd() const { return m_socketfd; }

	bool loginSuccess() const{ return m_loginResult; }

	void onMessageHandler(SOCKET socketfd);

	void Registerhandler(json& responsejs);
	void LoginHandler(json& responsejs);
	void handleCmdhandler();

private:
	void getFriendList(json& responsejs);
	void getGroupList(json& responsejs);
	void getOfflineMessage(json& responsejs);
	void showLoginResult();

	void help();
	void chat(std::string string);
	void addfriend(std::string string);
	void creategroup(std::string string);
	void addgroup(std::string string);
	void groupchat(std::string string);
	void loginout(std::string string);
	std::string getCurrentTime();

	void InitialHandlerMap();

private:
	SOCKET			m_socketfd;

	bool			m_loginResult{ false };
	bool			m_hasFriend{ false };
	bool			m_hasGroup{ false };
	bool			m_hasOfflineMsg{ false };
	bool			m_isChatting{ false };

	User						m_curUser;
	std::vector<User>			m_curUserFriendList;
	std::vector<Group>			m_curUserGroupList;
	std::vector<std::string>	m_curUserOfflineMessage;

	std::unordered_map<std::string, std::string>							m_msgMap;
	std::unordered_map<std::string, std::function<void(std::string)>>		m_msgHandlerMap;
};



