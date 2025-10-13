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
	_clients.push_back(new Client(_next_client_id, client_fd));
	_clients_by_id[_next_client_id++] = _clients.back();
	_clients_by_fd[client_fd] = _clients.back();

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
				if (errno == EINTR || errno == ECONNABORTED) {}
				if (errno == EAGAIN || errno == EWOULDBLOCK)
					break;
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
	_clients_by_id.erase(client->getId());
	_clients_by_fd.erase(client->getFd());
	_clients.remove(client);
	delete client;
	_poll_array[i] = _poll_array.back();
	_poll_array.pop_back();
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

	int n;
	char buffer[1024];
	std::string s = "message from server: ok\n";
	for (size_t i = 1; i < _poll_array.size(); )
	{
		Client* client = _clients_by_fd[_poll_array[i].fd];
		if (_poll_array[i].revents & (POLLHUP | POLLERR | POLLNVAL))
		{
			std::cout << "client " << client->getId() << ": disconnected" << std::endl;
			deleteClient(client, i);
		}
		else if (_poll_array[i].revents & POLLIN)
		{
			while (true)
			{
				n = recv(client->getFd(), buffer, sizeof(buffer), 0);
				if (n == -1)
				{
					
					if (errno == EAGAIN || errno == EWOULDBLOCK) {}
					else
						std::cout << "client " << client->getId() << ": disconnected (error)" << std::endl;
					deleteClient(client, i);
					continue;
				}
				else if (n == 0)
				{
					std::cout << "client " << client->getId() << ": disconnected" << std::endl;
					deleteClient(client, i);
					continue;
				}
				else
				{}
			}
		}
	}

	// à modifier :
	// int n;
	// char buffer[1024];
	// std::string s = "message from server: ok\n";
	// Client* client;
	// for (Iterator it = _clients.begin(); it != _clients.end(); )
	// {
	// 	client = *it;
	// 	n = recv(client->getFd(), buffer, sizeof(buffer), 0);
	// 	if (n == 0)
	// 	{
	// 		std::cout << "client " << client->getId() << ": disconnected" << std::endl;
	// 		_clients_by_id.erase(client->getId());
	// 		_clients_by_fd.erase(client->getFd());
	// 		it = _clients.erase(it);
	// 		delete client;
	// 		continue;
	// 	}
	// 	else if (n == -1)
	// 	{
	// 		if (errno == EAGAIN || errno == EWOULDBLOCK) {} // pas de données envoyées mais la connexion est toujours là
	// 		else
	// 			perror_and_throw("recv");
	// 	}
	// 	else
	// 	{
	// 		std::cout << "message from client " << client->getId() << ": ";
	// 		std::cout.write(buffer, n);
	// 		if (send(client->getFd(), s.c_str(), s.size(), 0) == -1)
	// 			perror_and_throw("send");
	// 	}
	// 	++it;
	// }
}
