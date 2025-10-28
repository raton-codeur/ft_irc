#pragma once
#include "main.hpp"
#include "CommandHandler.hpp"

class Client;
class Channel;

class Server
{
private:

	// constant
	static const size_t _BACKLOG = 128;

	// attributes
	int _port;
	std::string _password;
	std::string _hostname;
	int _server_fd;
	std::vector<Client*> _clients;
	std::vector<struct pollfd*> _pollArray;
	std::map<std::string, Client*> _clientsByNick;
	std::map<std::string, Channel*> _channels;
	CommandHandler _cmdHandler;

	// utils <-> run
	void addClient(int client_fd);
	void acceptClients();
	void handleClientEvents();
	void removeClients();

	// utils
	void deleteClient(size_t i);

	// disabled
	Server();
	Server(const Server&);
	Server& operator=(const Server&);

	public:

	// constructor, destructor
	Server(int argc, char** argv);
	~Server();

	// main
	void run();

	// get, add, remove
	const std::string& getHostname() const;
	const std::string& getPassword() const;
	Client* getClientByNick(const std::string& nick);
	void addClientToNickMap(Client& client);
	void removeClientFromNickMap(const std::string& nick);
	Channel* getChannel(const std::string& name);
	Channel* getOrCreateChannel(const std::string& name);
	void deleteChannel(const std::string& name);

	// others
	void notifyClients(const std::set<std::string>& channels, const std::string& message, Client* exclude);
	void notifyClients(Channel* channel, const std::string& message, Client* exclude);
	void sendNamesList(Client& client, Channel* channel);
};
