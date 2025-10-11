#include "Server.hpp"

Server::Server() : _backlog(5), _next_client_id(0)
{
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd == -1)
		perror_and_throw("socket");

	fcntl(_server_fd, F_SETFL, O_NONBLOCK);

	int opt = 1;
	setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in addr_server;
	addr_server.sin_family = AF_INET;
	addr_server.sin_addr.s_addr = INADDR_ANY;
	addr_server.sin_port = htons(6667);

	if (bind(_server_fd, (struct sockaddr*)&addr_server, sizeof(addr_server)) == -1)
		perror_and_throw("bind");

	if (listen(_server_fd, _backlog) == -1)
		perror_and_throw("listen");
	std::cout << "server is listening on port 6667..." << std::endl;
}

Server::~Server()
{
	if (_server_fd != -1)
	{
		close(_server_fd);
		std::cout << "server : fd has been closed" << std::endl;
	}
	for (Iterator it = _clients.begin(); it != _clients.end(); ++it)
		delete *it;
}

void Server::addClient(int client_fd)
{
	_poll_array.push_back(pollfd());
	_clients.push_back(new Client(_next_client_id, client_fd));
	_clients_by_id[_next_client_id++] = _clients.back();
	_clients_by_fd[client_fd] = _clients.back();
}

void Server::main()
{
}
