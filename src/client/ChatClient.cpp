#include "ChatClient.h"
extern sem_t rwsem;

ChatClient::ChatClient(InetAddress addr) {
	InitialHandlerMap();

	m_socketfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (-1 == sockets::connect(m_socketfd, addr.getSockAddrInet())) {
		std::cerr << "connect failed!" << std::endl;
		exit(-1);
	}
}

void ChatClient::onMessageHandler(SOCKET socketfd) {
	while (true) {
		char buffer[1024] = { 0 };
		memset(buffer, 0, sizeof buffer);
		int len = sockets::read(socketfd, buffer, 1024);
		if (-1 == len) {
			close(socketfd);
			exit(-1);
		}

		json js = json::parse(buffer);
		int msgtype = js["msgid"].get<int>();

		std::string time, name, msg;
		if (js["time"].is_string()) {
			time = js["time"].get<string>();
		}
		if (js["name"].is_string()) {
			name = js["name"].get<string>();
		}
		if (js["msg"].is_string()) {
			msg = js["msg"].get<string>();
		}
		
		if (ONE_CHAT_MSG == msgtype) {
			std::cout << "\033[37m" << time << " [" << js["id"] << "]" << name << " said: " << "\033[0m" << msg << std::endl;
		} else if (GROUP_CHAT_MSG == msgtype) {
			std::cout << "\033[37m" << "group msg [" << js["groupid"] << "]:" << time << " [" << js["id"] << "]" << name << " said: " << "\033[0m" << msg << std::endl;
		} else if (LOGIN_MSG_ACK == msgtype) {
			LoginHandler(js);
			
			//notify main thread
			sem_post(&rwsem);
		} else if (REG_MSG_ACK == msgtype) {
			Registerhandler(js);

			//notify main thread
			sem_post(&rwsem);
		}
	}
}

void ChatClient::Registerhandler(json& responsejs) {
	if (0 != responsejs["errno"].get<int>()) {
		cerr << "user register failed!" << endl;
	} else {
		cout  << "user register success, yourID is " << "\033[31m" << responsejs["id"] << "\033[0m" << endl;
	}
}

void ChatClient::LoginHandler(json& responsejs) {

	if (0 != responsejs["errno"].get<int>()) {
		cerr << responsejs["errmsg"] << endl;
	} else {
		m_curUser.setId(responsejs["id"].get<int>());
		m_curUser.setName(responsejs["name"]);

		//get friend list
		if (responsejs.contains("friends")) {
			m_hasFriend = true;
			getFriendList(responsejs);
		}

		// get group list
		if (responsejs.contains("groups")) {
			m_hasGroup = true;
			getGroupList(responsejs);
		}

		// show offline message
		if (responsejs.contains("offlinemsg")) {
			m_hasOfflineMsg = true;
			getOfflineMessage(responsejs);
		}

		// show all data of curuser
		showLoginResult();

		m_loginResult = true;
		m_isChatting = true;
	}
}

void ChatClient::handleCmdhandler() {
	help();

	char buffer[1024];
	memset(buffer, 0, sizeof buffer);
	while (m_isChatting) {
		//wait for input
		cin.getline(buffer, 1024);

		std::string commandbuf(buffer);
		std::string command;
		int idx = commandbuf.find(">");
		if (-1 == idx) {
			command = commandbuf;
		} else {
			command = commandbuf.substr(0, idx);
		}
		auto iter = m_msgHandlerMap.find(command);
		if (iter == m_msgHandlerMap.end()) {
			cerr << "invalid input command!" << endl;
			continue;
		}

		iter->second(commandbuf.substr(idx + 1, commandbuf.size() - idx));
	}
}

void ChatClient::getFriendList(json& responsejs) {
	m_curUserFriendList.clear();
	vector<string> vec = responsejs["friends"];
	for (string& str : vec) {
		json js = json::parse(str);
		User user;
		user.setId(js["id"].get<int>());
		user.setName(js["name"]);
		user.setState(js["state"]);
		m_curUserFriendList.push_back(user);
	}
}

void ChatClient::getGroupList(json& responsejs) {
	m_curUserGroupList.clear();

	vector<string> vec1 = responsejs["groups"];
	for (string& groupstr : vec1) {
		json grpjs = json::parse(groupstr);
		Group group;
		group.setId(grpjs["id"].get<int>());
		group.setName(grpjs["groupname"]);
		group.setDesc(grpjs["groupdesc"]);

		vector<string> vec2 = grpjs["users"];
		for (string& userstr : vec2) {
			GroupUser user;
			json js = json::parse(userstr);
			user.setId(js["id"].get<int>());
			user.setName(js["name"]);
			user.setState(js["state"]);
			user.setRole(js["role"]);
			group.getUsers().push_back(user);
		}

		m_curUserGroupList.push_back(group);
	}
}

void ChatClient::getOfflineMessage(json& responsejs) {
	m_curUserOfflineMessage = responsejs["offlinemsg"];
}

void ChatClient::showLoginResult() {
	std::cout << "\033[32m================ Welcome " << m_curUser.getName() << "================\033[0m" << std::endl;
	if (m_hasFriend) {
		std::cout << "------------------ " << "friend list" << " ------------------" << std::endl;
		for (auto f_user : m_curUserFriendList) {
			std::cout << "| " << f_user.getId() << " | " << f_user.getName() << " | " << f_user.getState() <<std::endl;
		}
		std::cout << "------------------ " << "-----------" << " ------------------" << std::endl;
	}

	if (m_hasGroup) {
		std::cout << "------------------ " << "group list" << " ------------------" << std::endl;
		for (auto group : m_curUserGroupList) {
			std::cout << "| " << group.getId() << " | " << group.getName() <<std::endl;
		}
		std::cout << "------------------ " << "----------" << " ------------------" << std::endl;
	}

	if (m_hasOfflineMsg) {
		std::cout << "------------------ " << "unread message" << " ------------------" << std::endl;
		for (auto msg : m_curUserOfflineMessage) {
			json js = json::parse(msg);
			// 2024-12-24 16:41:19 [13]zhang san said: hi
			if (ONE_CHAT_MSG == js["msgid"].get<int>()) {
				std::cout << "\033[31m" << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>() << " said: " << "\033[0m" << js["msg"].get<string>() << endl;
			} else {
				std::cout << "\033[31m" << "Group[" << js["groupid"] << "]:\n"
					      << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>() << " said: " << "\033[0m" << js["msg"].get<string>() << endl;
			}
		}
		std::cout << "-------------------" << "----------" << "-------------------" << std::endl;
	}
	std::cout << "\033[32m=====================================================\033[0m" << std::endl;
}

void ChatClient::help() {
	std::cout << "\033[32m" << "[TIPS]" << "\033[0m" << " command lines list ==================" << std::endl;
	for (auto& p : m_msgMap) {
		cout << p.first << " : " << p.second << endl;
		std::cout << std::endl;
	}
	std::cout << "\033[32m" << "[TIPS]" << "\033[0m" << "command lines list ==================" << std::endl;
}

void ChatClient::chat(std::string str) {
	int idx = str.find(":"); // friendid:message
	if (-1 == idx) {
		cerr << "chat command invalid!" << endl;
		return;
	}

	int friendid = std::stoi(str.substr(0, idx).c_str());
	std::string message = str.substr(idx + 1, str.size() - idx);

	//package msg
	// {"msgid":ONE_CHAT_MSG, "id":xx, "name":"xx", "toid":xx, "msg":"xxxx", "time":"xxx"}
	json js;
	js["msgid"] = ONE_CHAT_MSG;
	js["id"] = m_curUser.getId();
	js["name"] = m_curUser.getName();
	js["toid"] = friendid;
	js["msg"] = message;
	js["time"] = getCurrentTime();
	std::string buffer = js.dump();

	sockets::write(m_socketfd, buffer.c_str(), strlen(buffer.c_str()) + 1);
}

void ChatClient::addfriend(std::string str) {
	int friendid = std::stoi(str.c_str());

	//package msg
	// {"msgid":ADD_FRIEND_MSG, "id":xx, "friendid":"xx"}
	json js;
	js["msgid"] = ADD_FRIEND_MSG;
	js["id"] = m_curUser.getId();
	js["friendid"] = friendid;
	string buffer = js.dump();

	sockets::write(m_socketfd, buffer.c_str(), strlen(buffer.c_str()) + 1);
}

void ChatClient::creategroup(std::string str) {
	int idx = str.find(":");
	if (-1 == idx) {
		cerr << "creategroup command invalid!" << endl;
		return;
	}

	string groupname = str.substr(0, idx);
	string groupdesc = str.substr(idx + 1, str.size() - idx);

	//package msg
	// {"msgid":CREATE_GROUP_MSG, "id":xx, "groupname":"xx", "groupdesc":xx}
	json js;
	js["msgid"] = CREATE_GROUP_MSG;
	js["id"] = m_curUser.getId();
	js["groupname"] = groupname;
	js["groupdesc"] = groupdesc;
	string buffer = js.dump();

	sockets::write(m_socketfd, buffer.c_str(), strlen(buffer.c_str()) + 1);
}

void ChatClient::addgroup(std::string str) {
	int groupid = std::stoi(str.c_str());

	//package msg
	// {"msgid":ADD_GROUP_MSG, "id":xx, "groupid":"xx"}
	json js;
	js["msgid"] = ADD_GROUP_MSG;
	js["id"] = m_curUser.getId();
	js["groupid"] = groupid;
	string buffer = js.dump();

	sockets::write(m_socketfd, buffer.c_str(), strlen(buffer.c_str()) + 1);
}

void ChatClient::groupchat(std::string str) {
	int idx = str.find(":");
	if (-1 == idx) {
		cerr << "groupchat command invalid!" << endl;
		return;
	}

	int groupid = std::stoi(str.substr(0, idx).c_str());
	string message = str.substr(idx + 1, str.size() - idx);

	//package msg
	// {"msgid":GROUP_CHAT_MSG, "id":xx, "name":xx, "groupid":"xx", "msg":"xx", "time":"xx"}
	json js;
	js["msgid"] = GROUP_CHAT_MSG;
	js["id"] = m_curUser.getId();
	js["name"] = m_curUser.getName();
	js["groupid"] = groupid;
	js["msg"] = message;
	js["time"] = getCurrentTime();
	string buffer = js.dump();

	sockets::write(m_socketfd, buffer.c_str(), strlen(buffer.c_str()) + 1);
}

void ChatClient::loginout(std::string str) {

	//package msg
	// {"msgid":LOGINOUT_MSG, "id":xx}
	json js;
	js["msgid"] = LOGINOUT_MSG;
	js["id"] = m_curUser.getId();
	string buffer = js.dump();

	sockets::write(m_socketfd, buffer.c_str(), strlen(buffer.c_str()) + 1);
	m_isChatting = false;
}

std::string ChatClient::getCurrentTime() {
	return Timestamp::now().toFormattedString();
}

void ChatClient::InitialHandlerMap() {
	m_msgMap = {
	{"help", "show all available cmds"},
	{"chat", "chat with one person, format: chat>friendid:message. for example: chat>15:hi"},
	{"addfriend", "add friends, format: add>friendid. for example: add>15"},
	{"creategroup", "create group, format: newgroup>groupname:groupdesc. for example: newgroup>Happy Family Group:me and famlies"},
	{"addgroup", "join a group, format: addgroup>groupid"},
	{"groupchat", "chat in group, format: chatgroup>groupid:message. for example: chatgroup>1:hello everyone!"},
	{"loginout", "login out, format: loginout"} };

	m_msgHandlerMap = {
	{"help",		std::bind(&ChatClient::help, this)},
	{"chat",		std::bind(&ChatClient::chat, this, std::placeholders::_1)},
	{"add",			std::bind(&ChatClient::addfriend, this, std::placeholders::_1)},
	{"newgroup",	std::bind(&ChatClient::creategroup, this, std::placeholders::_1)},
	{"addgroup",	std::bind(&ChatClient::addgroup, this, std::placeholders::_1)},
	{"chatgroup",	std::bind(&ChatClient::groupchat, this, std::placeholders::_1)},
	{"loginout",	std::bind(&ChatClient::loginout, this, std::placeholders::_1)} };
}

