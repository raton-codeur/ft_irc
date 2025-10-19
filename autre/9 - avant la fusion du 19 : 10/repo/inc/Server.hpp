#pragma once
#include "main.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "CommandHandler.hpp"

class Server
{
private:

	static const size_t BACKLOG = 128;
	static const size_t BUFFER_RECV_SIZE = 1024;
	static const size_t MAX_READ_PER_CYCLE = 8192;
	static const size_t MAX_LINE_LENGTH = 512;
	static const size_t MAX_CLIENT_INPUT_BUFFER = 16384;

	int _port;
	std::string _password;
	int _server_fd;
	std::vector<Client*> _clients;
	std::vector<struct pollfd> _poll_array;
	std::map<std::string, Client*> _clients_by_nick;
	std::map<std::string, Channel*> _channels;
	char _buffer_recv[BUFFER_RECV_SIZE];
	CommandHandler _cmdHandler;

	Channel* getChannel(const std::string& name);
	Channel* getOrCreateChannel(const std::string& name);
	void addClient(int client_fd);
	void acceptClients();
	void deleteClient(size_t i);
	void handleClientErrors(size_t i);
	void handleClientPOLLIN(size_t& i);
	void handleClientEvents();

	Server();
	Server(const Server&);
	Server& operator=(const Server&);

public:

	Server(int argc, char** argv);
	~Server();

	void run();
};
