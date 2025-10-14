#pragma once
#include "main.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "CommandHandler.hpp"

class Server
{
	private:

	const int _backlog;

	int _server_fd;
	std::vector<Client*> _clients;
	std::vector<struct pollfd> _poll_array;
	std::map<std::string, Channel*> _channels;

	int _next_client_id;
	std::string	_password;
	CommandHandler	*_cmdHandler;

	void addClient(int client_fd, std::string &clientIP);
	void acceptClients();
	void deleteClient(Client* client, int i);
	void handleClientEvents();

	Server(const Server&);
	Server& operator=(const Server&);

	public:

	Server();
	~Server();
	void run();

	Client* findClientByNick(const std::string &nick);
	Channel* getOrCreateChannel(const std::string &name);
	Channel* findChannel(const std::string &name);
	const std::map<std::string, Channel*>& getChannels() const;
};
