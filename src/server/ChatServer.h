#include <TcpServer.h>
#include <EventLoop.h>

using namespace net;

class ChatServer {
public:
	ChatServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg);

	void start();

private:
	void onConnection(const TcpConnectionPtr& conn);

	void onMessage(const TcpConnectionPtr& conn,
		ByteBuffer* buffer,
		Timestamp time);

	TcpServer       m_server;
	EventLoop*		m_loop;
};