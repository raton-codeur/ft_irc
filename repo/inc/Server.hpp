#pragma once
#include "main.hpp"
#include "Client.hpp"

class Server
{
	private:

	const int _backlog;

	int _server_fd;
	std::list<Client*> _clients;
	int _next_client_id;
	std::map<int, Client*> _clients_by_id;
	std::map<int, Client*> _clients_by_fd;
	std::vector<struct pollfd> _poll_array;

	void addClient(int client_fd);
	void removeClient(int client_id);

	typedef std::list<Client*>::iterator Iterator;

	Server(const Server&);
	Server& operator=(const Server&);

	public:

	Server();
	~Server();

	void main();
};
