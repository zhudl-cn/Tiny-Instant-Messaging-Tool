#include "ChatService.h"
#include "Msg.h"
#include <AsyncLog.h>
#include <vector>

#define Params std::placeholders
using namespace net;

ChatService::ChatService() {
	//user handler
	m_msgHandlerMap.insert({ LOGIN_MSG, std::bind(&ChatService::login, this, Params::_1, Params::_2, Params::_3) });
	m_msgHandlerMap.insert({ LOGINOUT_MSG, std::bind(&ChatService::loginout, this, Params::_1, Params::_2, Params::_3) });
	m_msgHandlerMap.insert({ REG_MSG, std::bind(&ChatService::reg, this, Params::_1, Params::_2, Params::_3) });
	m_msgHandlerMap.insert({ ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, Params::_1, Params::_2, Params::_3) });
	m_msgHandlerMap.insert({ ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, Params::_1, Params::_2, Params::_3) });

	//group handler
	m_msgHandlerMap.insert({ CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, Params::_1, Params::_2, Params::_3) });
	m_msgHandlerMap.insert({ ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, Params::_1, Params::_2, Params::_3) });
	m_msgHandlerMap.insert({ GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, Params::_1, Params::_2, Params::_3) });

	// check offline message
	if (m_redis.connect()) {
		m_redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, Params::_1, Params::_2));
	}
}

void ChatService::reset() {
	//set user state to offline
	m_userModel.resetState();
}

MsgHandler ChatService::getHandler(int msgid) {
	auto it = m_msgHandlerMap.find(msgid);
	if (it == m_msgHandlerMap.end()) {
		//if msgid is valid, return null handler
		LOGE("msgid:", msgid, " cannot find handler!");
		return nullptr;
	} else {
		return m_msgHandlerMap[msgid];
	}
}

void ChatService::login(const TcpConnectionPtr& conn, json& js, Timestamp time) {
	int id = js["id"].get<int>();
	string pwd = js["password"];

	User user = m_userModel.query(id);
	if (user.getId() == id && user.getPwd() == pwd) {
		if (user.getState() == "online") {
			json response;
			response["msgid"] = LOGIN_MSG_ACK;
			response["errno"] = 2;
			response["errmsg"] = "this account has logined";
			conn->send(response.dump());
		} else {
			{
				lock_guard<mutex> lock(m_connMutex);
				m_userConnMap.insert({ id, conn });
			}

			m_redis.subscribe(id);

			user.setState("online");
			m_userModel.updateState(user);

			json response;
			response["msgid"] = LOGIN_MSG_ACK;
			response["errno"] = 0;
			response["id"] = user.getId();
			response["name"] = user.getName();

			vector<string> vec = m_offlineMsgModel.query(id);
			if (!vec.empty()) {
				response["offlinemsg"] = vec;
				m_offlineMsgModel.remove(id);
			}

			vector<User> userVec = m_friendModel.query(id);
			if (!userVec.empty()) {
				vector<string> vec2;
				for (User& user : userVec) {
					json js;
					js["id"] = user.getId();
					js["name"] = user.getName();
					js["state"] = user.getState();
					vec2.push_back(js.dump());
				}
				response["friends"] = vec2;
			}

			vector<Group> groupuserVec = m_groupModel.queryGroups(id);
			if (!groupuserVec.empty()) {
				vector<string> groupV;
				for (Group& group : groupuserVec) {
					json grpjson;
					grpjson["id"] = group.getId();
					grpjson["groupname"] = group.getName();
					grpjson["groupdesc"] = group.getDesc();
					vector<string> userV;
					for (GroupUser& user : group.getUsers()) {
						json js;
						js["id"] = user.getId();
						js["name"] = user.getName();
						js["state"] = user.getState();
						js["role"] = user.getRole();
						userV.push_back(js.dump());
					}
					grpjson["users"] = userV;
					groupV.push_back(grpjson.dump());
				}

				response["groups"] = groupV;
			}
			conn->send(response.dump());
		}
	} else {
		json response;
		response["msgid"] = LOGIN_MSG_ACK;
		response["errno"] = 1;
		response["errmsg"] = "id or password is invalid!";
		conn->send(response.dump());
	}
}

void ChatService::reg(const TcpConnectionPtr& conn, json& js, Timestamp time) {
	string name = js["name"];
	string pwd = js["password"];

	User user;
	user.setName(name);
	user.setPwd(pwd);
	user.setState("offline");

	bool state = m_userModel.insert(user);
	if (state) {
		json response;
		response["msgid"] = REG_MSG_ACK;
		response["errno"] = 0;
		response["id"] = user.getId();
		conn->send(response.dump());
	} else {
		json response;
		response["msgid"] = REG_MSG_ACK;
		response["errno"] = 1;
		conn->send(response.dump());
	}
}

void ChatService::loginout(const TcpConnectionPtr& conn, json& js, Timestamp time) {
	int userid = js["id"].get<int>();

	{
		lock_guard<mutex> lock(m_connMutex);
		auto it = m_userConnMap.find(userid);
		if (it != m_userConnMap.end()) {
			m_userConnMap.erase(it);
		}
	}

	m_redis.unsubscribe(userid);

	User user(userid, "", "", "offline");
	m_userModel.updateState(user);
}

void ChatService::clientCloseException(const TcpConnectionPtr& conn) {
	User user;
	{
		lock_guard<mutex> lock(m_connMutex);
		for (auto it = m_userConnMap.begin(); it != m_userConnMap.end(); ++it) {
			if (it->second == conn) {
				user.setId(it->first);
				m_userConnMap.erase(it);
				break;
			}
		}
	}

	m_redis.unsubscribe(user.getId());

	if (user.getId() != -1) {
		user.setState("offline");
		m_userModel.updateState(user);
	}
}

void ChatService::oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time) {
	int toid = js["toid"].get<int>();

	{
		lock_guard<mutex> lock(m_connMutex);
		auto it = m_userConnMap.find(toid);
		if (it != m_userConnMap.end()) {
			it->second->send(js.dump());
			return;
		}
	}

	User user = m_userModel.query(toid);
	if (user.getState() == "online") {
		m_redis.publish(toid, js.dump());
		return;
	}

	m_offlineMsgModel.insert(toid, js.dump());
}

void ChatService::addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time) {
	int userid = js["id"].get<int>();
	int friendid = js["friendid"].get<int>();

	m_friendModel.insert(userid, friendid);
}

void ChatService::createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time) {
	int userid = js["id"].get<int>();
	string name = js["groupname"];
	string desc = js["groupdesc"];

	Group group(-1, name, desc);
	if (m_groupModel.createGroup(group)) {
		m_groupModel.addGroup(userid, group.getId(), "creator");
	}
}

void ChatService::addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time) {
	int userid = js["id"].get<int>();
	int groupid = js["groupid"].get<int>();
	m_groupModel.addGroup(userid, groupid, "normal");
}

void ChatService::groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time) {
	int userid = js["id"].get<int>();
	int groupid = js["groupid"].get<int>();
	vector<int> useridVec = m_groupModel.queryGroupUsers(userid, groupid);

	//lock_guard<mutex> lock(m_connMutex);
	for (int id : useridVec) {
		auto it = m_userConnMap.find(id);
		if (it != m_userConnMap.end()) {
			it->second->send(js.dump());
		} else {
			User user = m_userModel.query(id);
			if (user.getState() == "online") {
				m_redis.publish(id, js.dump());
			} else {
				m_offlineMsgModel.insert(id, js.dump());
			}
		}
	}
}

void ChatService::handleRedisSubscribeMessage(int userid, string msg) {
	//lock_guard<mutex> lock(m_connMutex);
	auto it = m_userConnMap.find(userid);
	if (it != m_userConnMap.end()) {
		it->second->send(msg);
		return;
	}

	m_offlineMsgModel.insert(userid, msg);
}