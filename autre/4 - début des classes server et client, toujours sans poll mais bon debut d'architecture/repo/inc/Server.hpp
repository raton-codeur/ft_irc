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

	Server(const Server&);
	Server& operator=(const Server&);

	public:

	typedef std::list<Client*>::iterator Iterator;

	std::map<int, Client*> clients_by_id;
	std::map<int, Client*> clients_by_fd;

	Server();
	~Server();

	int getServerFd() const;

	void addClient(int client_fd);
	void checkClients();
};
