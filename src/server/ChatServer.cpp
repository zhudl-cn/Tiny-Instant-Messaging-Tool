#include "ChatServer.h"
#include "ChatService.h"

#include <functional>
#include <string>

#define PARAMS std::placeholders

using json = nlohmann::json;

ChatServer::ChatServer(EventLoop* loop,
	const InetAddress& listenAddr,
	const string& nameArg)
	: m_server(loop, listenAddr, nameArg), m_loop(loop) {
	m_server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, PARAMS::_1));
	m_server.setMessageCallback(std::bind(&ChatServer::onMessage, this, PARAMS::_1, PARAMS::_2, PARAMS::_3));

}

void ChatServer::start() { m_server.start(4); }

void ChatServer::onConnection(const TcpConnectionPtr& conn) {
	if (!conn->connected()) {
		ChatService::instance()->clientCloseException(conn);
		conn->shutdown();
	}
}

void ChatServer::onMessage(const TcpConnectionPtr& conn, ByteBuffer* buffer, Timestamp time) {
	string buf = buffer->retrieveAllAsString();
	//while (buf.back() == '}') {
		json js = json::parse(buf);
		auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
		if (msgHandler == nullptr) {
			//invalid msgid
			conn->forceClose();
		}

		msgHandler(conn, js, time);
	//}
}