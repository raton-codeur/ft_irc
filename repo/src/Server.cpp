#include "Server.hpp"

static bool has_white_spaces(const std::string& str)
{
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (std::isspace(str[i]))
			return true;
	}
	return false;
}

static void checkArgs(int argc, char** argv)
{
	if (argc != 3)
		error_and_throw("Usage: ./ircserv <port> <password>");
	char* end;
	long port = std::strtol(argv[1], &end, 10);
	if (*end != '\0' || port < 1024 || port > 65535)
		error_and_throw("Error: invalid port number (must be an integer between 1024 and 65535)");
	if (std::strlen(argv[2]) == 0 || std::strlen(argv[2]) > 32 || has_white_spaces(argv[2]))
		error_and_throw("Error: invalid password (must be 1-32 characters long, no whitespaces)");
}

Server::Server(int argc, char** argv) : _backlog(5), _cmdHandler(*this)
{
	checkArgs(argc, argv);
	_port = std::atoi(argv[1]);
	_password = argv[2];

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
	addr_server.sin_port = htons(_port);

	if (bind(_server_fd, (struct sockaddr*)&addr_server, sizeof(addr_server)) == -1)
		perror_and_throw("bind");

	if (listen(_server_fd, _backlog) == -1)
		perror_and_throw("listen");
	std::cout << "server is listening on port " << _port << "..." << std::endl;

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
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		delete it->second;
}

Channel* Server::getChannel(const std::string& name) const
{
	std::map<std::string, Channel*>::iterator it = _channels.find(name);
	if (it != _channels.end())
		return it->second;
	return NULL;
}

Channel* Server::getOrCreateChannel(const std::string& name)
{
	Channel* channel = getChannel(name);
	if (!channel)
	{
		channel = new Channel(name);
		_channels[name] = channel;
		std::cout << "channel " << name << ": created" << std::endl;
	}
	return channel;
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
	if (g_stop_requested)
		return;
	handleClientsEvents();
}

void Server::addClient(int client_fd)
{
	_clients.push_back(new Client(*this, client_fd));

	struct pollfd p;
	p.fd = client_fd;
	p.events = POLLIN;
	p.revents = 0;
	_poll_array.push_back(p);
}

void Server::acceptClients()
{
	int client_fd;

	if (_poll_array[0].revents & (POLLHUP | POLLERR | POLLNVAL))
		error_and_throw("server socket error");
	if (_poll_array[0].revents & POLLIN)
	{
		while (true)
		{
			client_fd = accept(_server_fd, NULL, NULL);
			if (g_stop_requested)
				return;
			if (client_fd != -1)
			{
				if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
					perror_and_throw("fcntl (client)");
				addClient(client_fd);
			}
			else if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			else if (errno == ECONNABORTED || errno == EINTR)
				continue;
			else
				perror_and_throw("accept");
		}
	}
}

void Server::deleteClient(size_t i)
{
	delete _clients[i];
	_clients[i] = _clients.back();
	_clients.pop_back();
	_poll_array[i] = _poll_array.back();
	_poll_array.pop_back();
}

void Server::handleClientErrors(size_t i)
{
	if (_poll_array[i].revents & POLLNVAL)
		std::cout << "client (fd " << _clients[i]->getFd() << "): disconnected (invalid fd)" << std::endl;
	else if (_poll_array[i].revents & POLLERR)
		std::cout << "client (fd " << _clients[i]->getFd() << "): disconnected (network error)" << std::endl;
	else
		std::cout << "client (fd " << _clients[i]->getFd() << "): disconnected" << std::endl;
	deleteClient(i);
}

void Server::handleClientPOLLIN(size_t& i)
{
	ssize_t n;
	while (_poll_array[i].revents & POLLIN)
	{
		do
			n = recv(_clients[i]->getFd(), _buffer_recv, sizeof(_buffer_recv), 0);
		while (n == -1 && errno == EINTR);
		if (g_stop_requested)
			return;
		if (n > 0)
		{
			_clients[i]->getIn().append(_buffer_recv, n);
			_cmdHandler.handleCommand(_clients[i]);
		}
		else if (n == 0)
		{
			std::cout << "client (fd " << _clients[i]->getFd() << "): disconnected" << std::endl;
			deleteClient(i);
			return;
		}
		else if (errno == EAGAIN || errno == EWOULDBLOCK)
			break;
		else
		{
			std::cout << "client (fd " << _clients[i]->getFd() << "): disconnected (error)" << std::endl;
			deleteClient(i);
		}
	}
	++i;
}

void Server::handleClientsEvents()
{
	for (size_t i = 1; i < _poll_array.size();)
	{
		if (_poll_array[i].revents & (POLLHUP | POLLERR | POLLNVAL))
			handleClientErrors(i);
		else if (_poll_array[i].revents & POLLIN)
			handleClientPOLLIN(i);
		else if (g_stop_requested)
			return;
		else
			++i;
	}
}
