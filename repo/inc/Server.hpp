#pragma once
#include "main.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "CommandHandler.hpp"

class Server
{
private:

	const int _backlog;

	int _port;
	std::string _password;
	int _server_fd;
	std::vector<Client*> _clients;
	std::vector<struct pollfd> _poll_array;
	std::map<std::string, Client*> _clients_by_nick;
	std::map<std::string, Channel*> _channels;
	char _buffer_recv[1024];
	CommandHandler _cmdHandler;

	Channel* getChannel(const std::string& name) const;
	Channel* getOrCreateChannel(const std::string& name);
	void addClient(int client_fd);
	void acceptClients();
	void deleteClient(size_t i);
	void handleClientErrors(size_t i);
	void handleClientPOLLIN(size_t& i);
	void handleClientsEvents();

	Server();
	Server(const Server&);
	Server& operator=(const Server&);

public:

	Server(int argc, char** argv);
	~Server();

	void run();
};
