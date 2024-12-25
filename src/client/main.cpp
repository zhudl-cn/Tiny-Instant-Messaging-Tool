#include "ChatClient.h"
#include "json.hpp"

using namespace net;

sem_t rwsem;

int main(int argc, char** argv) {
	if (argc < 3) {
		std::cerr << "command invalid! example: ./ChatClient 127.0.0.1 8000" << std::endl;
		exit(-1);
	}

	char* ip = argv[1];
	uint16_t port = std::stoi(argv[2]);
	InetAddress addr(ip, port);

	ChatClient client(addr);

	sem_init(&rwsem, 0, 0);

	std::thread connThread(std::bind(&ChatClient::onMessageHandler, &client, client.fd()));
	connThread.detach();

	while (true) {
		std::cout << "\033[33m================Welcome to Use ChantClient================\033[0m" << std::endl;
		std::cout << " | Input 1 -> login";
		std::cout << " | Input 2 -> register";
		std::cout << " | Input 3 -> quit" << std::endl;
		std::cout << "\033[33m==========================================================\033[0m" << std::endl;
		std::cout << "Option: ";


		int Option = 0;
		std::cin >> Option;
		std::cin.get();

		switch (Option) {
		case 1: //login
		{
			int id = 0;
			char pwd[50] = { 0 };
			std::cout << "userid:";
			std::cin >> id;
			std::cin.get();
			std::cout << "userpassword:";
			std::cin.getline(pwd, 50);

			json js;
			js["msgid"] = LOGIN_MSG;
			js["id"] = id;
			js["password"] = pwd;
			std::string request = js.dump();

			sockets::write(client.fd(), request.c_str(), strlen(request.c_str()) + 1);
			
			sem_wait(&rwsem); //wait for worker thread

			//enter chat process
			if (client.loginSuccess()) {
				client.handleCmdhandler();
			}
			break;
		}
		case 2: //register
		{
			char name[50] = { 0 };
			char pwd[50] = { 0 };
			std::cout << "\033[38;5;214musername:\033[0m";
			std::cin.getline(name, 50);
			std::cout << "\033[38;5;214userpassword:\033[0m";
			std::cin.getline(pwd, 50);

			json js;
			js["msgid"] = REG_MSG;
			js["name"] = name;
			js["password"] = pwd;
			std::string request = js.dump();

			sockets::write(client.fd(), request.c_str(), strlen(request.c_str()) + 1);
			sem_wait(&rwsem);

			break;
		}
		case 3: // quit
			sockets::close(client.fd());
			sem_destroy(&rwsem);
			return 0;
		default:
			std::cerr << "invalid input!" << std::endl;
			break;
		}
	}

	return 0;
}
