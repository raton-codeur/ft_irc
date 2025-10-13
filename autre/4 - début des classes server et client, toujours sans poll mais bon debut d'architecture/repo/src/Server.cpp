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

int Server::getServerFd() const
{
	return _server_fd;
}

void Server::addClient(int client_fd)
{
	_clients.push_back(new Client(_next_client_id, client_fd));
	clients_by_id[_next_client_id++] = _clients.back();
	clients_by_fd[client_fd] = _clients.back();
}

void Server::checkClients()
{
	int n;
	char buffer[1024];
	std::string s = "message from server: ok\n";
	Client* client;
	for (Iterator it = _clients.begin(); it != _clients.end(); )
	{
		client = *it;
		n = recv(client->getFd(), buffer, sizeof(buffer), 0);
		if (n == 0)
		{
			std::cout << "client " << client->getId() << ": disconnected" << std::endl;
			clients_by_id.erase(client->getId());
			clients_by_fd.erase(client->getFd());
			it = _clients.erase(it);
			delete client;
			continue;
		}
		else if (n == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK) {} // pas de données envoyées mais la connexion est toujours là
			else
				perror_and_throw("recv");
		}
		else
		{
			std::cout << "message from client " << client->getId() << ": ";
			std::cout.write(buffer, n);
		}
		if (send(client->getFd(), s.c_str(), s.size(), 0) == -1)
			perror_and_throw("send");
		++it;
	}
}
