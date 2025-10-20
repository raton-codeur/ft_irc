#pragma once
#include "main.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "CommandHandler.hpp"

class Server
{
private:

	// constants
	static const size_t BACKLOG = 128;
	static const size_t BUFFER_RECV_SIZE = 1024;
	static const size_t MAX_READ_PER_CYCLE = 8192;
	static const size_t MAX_LINE_LENGTH = 512;
	static const size_t MAX_CLIENT_INPUT_BUFFER = 16384;

	// attributes
	int _port;
	std::string _password;
	int _server_fd;
	const std::string _hostname;
	std::vector<Client*> _clients;
	std::vector<struct pollfd> _poll_array;
	std::map<std::string, Client*> _clients_by_nick;
	std::map<std::string, Channel*> _channels;
	char _buffer_recv[BUFFER_RECV_SIZE];
	CommandHandler _cmdHandler;

	// methods <-> poll
	void addClient(int client_fd);
	void acceptClients();
	void handleClientErrors(size_t i);
	void handleClientPOLLIN(size_t& i);
	void handleClientEvents();

	// utils
	Channel* getChannel(const std::string& name);

	// canonical form
	Server();
	Server(const Server&);
	Server& operator=(const Server&);

public:

	// canonical form
	Server(int argc, char** argv);
	~Server();

	// main method
	void run();

	// get, add, delete
	const std::string& getHostname() const;
	const std::string& getPassword() const;
	Client* getClientByNick(const std::string& nick);
	Channel* getOrCreateChannel(const std::string& name);
	void addClientToNickMap(Client* client);
	void deleteClient(size_t i);

	// other
	void notifyClients(const std::set<std::string>& channels, const std::string& message, Client* exclude);
	void notifyClients(Channel* channel, const std::string& message, Client* exclude);
	void sendNamesList(Client& client, Channel* channel);
};
