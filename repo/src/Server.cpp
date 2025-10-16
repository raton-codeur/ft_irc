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

Server::Server(int argc, char** argv) : _backlog(5), _hostname("irc.qhauuy-jteste.local"), _cmdHandler(*this)
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

std::string Server::getPassword() const
{
	return _password;
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
	_clients.push_back(new Client(*this, client_fd));

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

void Server::processClientBuffer(Client *client)
{
	std::string &in = client->getIn();
	size_t pos;

	while (true)
	{
		pos = in.find("\r\n");
		if (pos == std::string::npos)
			pos = in.find('\n'); 

		if (pos == std::string::npos)
			break;

		std::string line = in.substr(0, pos);

		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		in.erase(0, pos + ((in[pos] == '\r' && in[pos + 1] == '\n') ? 2 : 1));

		if (line.empty())
			continue;

		std::cout << "→ Command received: [" << line << "]" << std::endl;

		_cmdHandler.handleCommand(client, line);
	}
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
				std::cout << "client (fd " << _clients[i]->getFd() << "): disconnected" << std::endl;
			else if (_poll_array[i].revents & POLLERR)
				std::cout << "client (fd " << _clients[i]->getFd() << "): disconnected (network error)" << std::endl;
			else
				std::cout << "client (fd " << _clients[i]->getFd() << "): disconnected (invalid fd)" << std::endl;
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
						std::cout << "client (fd " << _clients[i]->getFd() << "): disconnected (error)" << std::endl;
						deleteClient(_clients[i], i);
					}
				}
				else if (n == 0)
				{
					std::cout << "client (fd " << _clients[i]->getFd() << "): disconnected" << std::endl;
					deleteClient(_clients[i], i);
				}
				else
				{
					_clients[i]->getIn().append(buffer, n);
					std::cout << "reçu dans le buffer in du client : " << _clients[i]->getIn() << std::endl;
					processClientBuffer(_clients[i]);
					++i;
				}
				break;
			}
		}
		else
			++i;
	}
}

Channel* Server::getChannel(const std::string& name)
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

Client *Server::getClientByNick(const std::string &nick)
{
	std::map<std::string, Client*>::iterator it = _clients_by_nick.find(nick);
	if (it != _clients_by_nick.end())
		return it->second;
	return nullptr;
}

void Server::addClientToNickMap(const std::string &nick, Client *client)
{
	_clients_by_nick[nick] = client;
}

void Server::removeClientFromNickMap(const std::string &nick)
{
	std::map<std::string, Client*>::iterator it = _clients_by_nick.find(nick);
	if (it != _clients_by_nick.end())
		_clients_by_nick.erase(it);
}

void Server::notifyClients(const std::set<std::string> &channels, const std::string &message, Client *exclude = NULL)
{
	for (std::set<std::string>::const_iterator it = channels.begin(); it != channels.end(); ++it)
	{
		Channel *chan = getChannel(*it);
		if (!chan) continue;

		const std::set<Client*> &members = chan->getClients();
		for (std::set<Client*>::const_iterator mit = members.begin(); mit != members.end(); ++mit)
		{
			if (*mit != exclude)
				(*mit)->sendMessage(message);
		}
	}
}

std::string Server::getHostname() const
{
	return _hostname;
}
