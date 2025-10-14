#pragma once
#include "main.hpp"
#include "Client.hpp"

class Server
{
	private:

	const int _backlog;

	int _server_fd;
	std::vector<Client*> _clients;
	std::vector<struct pollfd> _poll_array;
	int _next_client_id;

	void addClient(int client_fd);
	void acceptClients();
	void deleteClient(Client* client, int i);
	void handleClientEvents();

	Server(const Server&);
	Server& operator=(const Server&);

	public:

	Server();
	~Server();
	void run();
};
