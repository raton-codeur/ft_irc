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
	CommandHandler _cmdHandler;

	void addClient(int client_fd);
	void acceptClients();
	void deleteClient(Client* client, int i);
	void handleClientEvents();
	void processClientBuffer(Client* client);
	Server();
	Server(const Server&);
	Server& operator=(const Server&);
	
	public:
	
	Server(int argc, char** argv);
	~Server();
	Channel* getChannel(const std::string& name);
	Channel* getOrCreateChannel(const std::string& name);
	Client* getClientByNick(const std::string& nick);
	void addClientToNickMap(const std::string &nick, Client *client);
	void removeClientFromNickMap(const std::string &nick);
	std::string getPassword() const;
	void notifyClients(const std::set<std::string> &channels, const std::string &message, Client *exclude);
	void run();
};
