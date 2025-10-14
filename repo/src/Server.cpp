#include "Server.hpp"

Server::Server() : _backlog(5), _next_client_id(0)
{
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd == -1)
		perror_and_throw("socket");

	if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) == -1)
		perror_and_throw("fcntl (server)");

	int opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		perror_and_throw("setsockopt");

	struct sockaddr_in addr_server;
	std::memset(&addr_server, 0, sizeof(addr_server));
	addr_server.sin_family = AF_INET;
	addr_server.sin_addr.s_addr = INADDR_ANY;
	addr_server.sin_port = htons(6667);

	if (bind(_server_fd, (struct sockaddr*)&addr_server, sizeof(addr_server)) == -1)
		perror_and_throw("bind");

	if (listen(_server_fd, _backlog) == -1)
		perror_and_throw("listen");
	std::cout << "server is listening on port 6667..." << std::endl;

	struct pollfd p;
	p.fd = _server_fd;
	p.events = POLLIN;
	p.revents = 0;
	_poll_array.push_back(p);

	_clients.push_back(nullptr);
}

Server::~Server()
{
	if (_server_fd != -1)
	{
		close(_server_fd);
		std::cout << "server : fd has been closed" << std::endl;
	}
	for (size_t i = 1; i < _clients.size(); ++i)
		delete _clients[i];
}

void Server::run()
{
	if (poll(&_poll_array[0], _poll_array.size(), -1) == -1)
	{
		if (errno == EINTR)
			return;
		else
			perror_and_throw("poll");
	}
	acceptClients();
	handleClientEvents();
}

void Server::addClient(int client_fd)
{
	_clients.push_back(new Client(_next_client_id++, client_fd));

	struct pollfd p;
	p.fd = client_fd;
	p.events = POLLIN;
	p.revents = 0;
	_poll_array.push_back(p);
}

void Server::acceptClients()
{
	if (_poll_array[0].revents & (POLLHUP | POLLERR | POLLNVAL))
		error_and_throw("server socket error");
	if (_poll_array[0].revents & POLLIN)
	{
		while (true)
		{
			int client_fd = accept(_server_fd, NULL, NULL);
			if (client_fd == -1)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK || g_stop_requested)
					break;
				else if (errno == ECONNABORTED || errno == EINTR)
					continue;
				else
					perror_and_throw("accept");
			}
			else
			{
				if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
					perror_and_throw("fcntl (client)");
				addClient(client_fd);
			}
		}
	}
}

void Server::deleteClient(Client* client, int i)
{
	_clients[i] = _clients.back();
	_clients.pop_back();
	_poll_array[i] = _poll_array.back();
	_poll_array.pop_back();
	delete client;
}

void Server::handleClientEvents()
{
	int n;
	char buffer[1024];
	for (size_t i = 1; i < _poll_array.size(); )
	{
		if (_poll_array[i].revents & (POLLHUP | POLLERR | POLLNVAL))
		{
			if (_poll_array[i].revents & POLLHUP)
				std::cout << "client " << _clients[i]->getId() << ": disconnected" << std::endl;
			else if (_poll_array[i].revents & POLLERR)
				std::cout << "client " << _clients[i]->getId() << ": disconnected (network error)" << std::endl;
			else
				std::cout << "client " << _clients[i]->getId() << ": disconnected (invalid fd)" << std::endl;
			deleteClient(_clients[i], i);
		}
		else if (_poll_array[i].revents & POLLIN)
		{
			while (true)
			{
				n = recv(_clients[i]->getFd(), buffer, sizeof(buffer), 0);
				if (n == -1)
				{
					if (g_stop_requested)
						return;
					else if (errno == EAGAIN || errno == EWOULDBLOCK)
						++i;
					else if (errno == EINTR)
						continue;
					else
					{
						std::cout << "client " << _clients[i]->getId() << ": disconnected (error)" << std::endl;
						deleteClient(_clients[i], i);
					}
				}
				else if (n == 0)
				{
					std::cout << "client " << _clients[i]->getId() << ": disconnected" << std::endl;
					deleteClient(_clients[i], i);
				}
				else
				{
					std::cout << "message from client " << _clients[i]->getId() << ": ";
					std::cout.write(buffer, n);
					++i;
				}
				break;
			}
		}
		else
			++i;
	}
}
