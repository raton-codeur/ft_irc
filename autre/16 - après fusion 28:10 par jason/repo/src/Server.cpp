#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

static bool has_white_spaces(const std::string& str)
{
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (std::isspace(static_cast<unsigned char>(str[i])))
			return true;
	}
	return false;
}

static void checkArgs(int argc, char** argv)
{
	if (argc != 3)
		errorAndThrow("Usage: ./ircserv <port> <password>");
	char* end;
	long port = std::strtol(argv[1], &end, 10);
	if (*end != '\0' || port < 1024 || port > 65535)
		errorAndThrow("Error: invalid port number (must be an integer between 1024 and 65535)");
	if (std::strlen(argv[2]) == 0 || std::strlen(argv[2]) > 32 || has_white_spaces(argv[2]))
		errorAndThrow("Error: invalid password (must be 1-32 characters long, no whitespaces)");
}

Server::Server(int argc, char** argv) : _hostname("irc.qhauuy-jteste.local"), _cmdHandler(*this)
{
	checkArgs(argc, argv);
	_port = std::atoi(argv[1]);
	_password = argv[2];

	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd == -1)
		perrorAndThrow("socket");

	if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) == -1)
		perrorAndThrow("fcntl (server)");

	int opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		perrorAndThrow("setsockopt");

	struct sockaddr_in addr_server;
	std::memset(&addr_server, 0, sizeof(addr_server));
	addr_server.sin_family = AF_INET;
	addr_server.sin_addr.s_addr = INADDR_ANY;
	addr_server.sin_port = htons(_port);

	if (bind(_server_fd, (struct sockaddr*)&addr_server, sizeof(addr_server)) == -1)
		perrorAndThrow("bind");

	if (listen(_server_fd, _BACKLOG) == -1)
		perrorAndThrow("listen");
	std::cout << "server is listening on port " << _port << "..." << std::endl;

	struct pollfd* p = new struct pollfd;
	p->fd = _server_fd;
	p->events = POLLIN;
	p->revents = 0;
	_pollArray.push_back(p);

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
	for (size_t i = 0; i < _pollArray.size(); ++i)
		delete _pollArray[i];
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		delete it->second;
}

void Server::run()
{
	if (poll(_pollArray[0], _pollArray.size(), -1) == -1)
	{
		if (errno == EINTR)
			return;
		else
			perrorAndThrow("poll");
	}
	acceptClients();
	handleClientEvents();
	removeClients();
}

void Server::addClient(int client_fd)
{
	struct pollfd* p = new struct pollfd;
	p->fd = client_fd;
	p->events = POLLIN;
	p->revents = 0;
	_pollArray.push_back(p);

	_clients.push_back(new Client(*this, client_fd, p->events));
}

void Server::acceptClients()
{
	int client_fd;

	if (_pollArray[0]->revents & (POLLHUP | POLLERR | POLLNVAL))
		errorAndThrow("server socket error");
	if (_pollArray[0]->revents & POLLIN)
	{
		while (true)
		{
			client_fd = accept(_server_fd, NULL, NULL);
			if (client_fd != -1)
			{
				if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
					perrorAndThrow("fcntl (client)");
				addClient(client_fd);
			}
			else if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			else if (errno == ECONNABORTED || errno == EINTR)
				checkSignals();
			else
				perrorAndThrow("accept");
		}
	}
}

void Server::handleClientEvents()
{
	for (size_t i = 1; i < _pollArray.size(); ++i)
	{
		if (_pollArray[i]->revents)
		{
			if (_pollArray[i]->revents & (POLLHUP | POLLERR | POLLNVAL))
			{
				if (_pollArray[i]->revents & POLLNVAL)
					_clients[i]->setHardDisconnect("client (fd " + std::to_string(_clients[i]->getFd()) + "): hard disconnect: invalid fd");
				else if (_pollArray[i]->revents & POLLERR)
					_clients[i]->setHardDisconnect("client (fd " + std::to_string(_clients[i]->getFd()) + "): hard disconnect: network error");
				else
					_clients[i]->setHardDisconnect("client (fd " + std::to_string(_clients[i]->getFd()) + "): hard disconnect: disconnected");
				continue;
			}
			if (_pollArray[i]->revents & POLLIN)
				_clients[i]->handlePOLLIN(_cmdHandler);
			if (_pollArray[i]->revents & POLLOUT && !_clients[i]->isHardDisconnect())
				_clients[i]->handlePOLLOUT();
		}
	}
}

void Server::deleteClient(size_t i)
{
	removeClientFromNickMap(_clients[i]->getNickname());
	delete _clients[i];
	delete _pollArray[i];
	_clients[i] = _clients.back();
	_clients.pop_back();
	_pollArray[i] = _pollArray.back();
	_pollArray.pop_back();
}

void Server::removeClients()
{
	for (size_t i = 1; i < _clients.size(); )
	{
		if ((_clients[i]->isSoftDisconnect() && _clients[i]->hasEmptyOutbox()) || _clients[i]->isHardDisconnect())
		{
			_clients[i]->printDisconnectLog();
			deleteClient(i);
		}
		else
			++i;
	}
}

const std::string& Server::getHostname() const
{
	return _hostname;
}

const std::string& Server::getPassword() const
{
	return _password;
}

Client* Server::getClientByNick(const std::string& nick)
{
	std::map<std::string, Client*>::iterator it = _clientsByNick.find(nick);
	if (it != _clientsByNick.end())
		return it->second;
	return NULL;
}

void Server::addClientToNickMap(Client& client)
{
	_clientsByNick[client.getNickname()] = &client;
}

void Server::removeClientFromNickMap(const std::string& nick)
{
	_clientsByNick.erase(nick);
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

void Server::deleteChannel(const std::string& name)
{
	std::map<std::string, Channel*>::iterator it = _channels.find(name);
	if (it != _channels.end())
	{
		delete it->second;
		_channels.erase(it);
		std::cout << "channel " << name << ": deleted (empty)" << std::endl;
	}
}

void Server::notifyClients(const std::set<std::string>& channels, const std::string& message, Client* exclude = NULL)
{
	for (std::set<std::string>::const_iterator it = channels.begin(); it != channels.end(); ++it)
	{
		Channel *chan = getChannel(*it);
		if (!chan) continue;

		const std::set<Client*> &members = chan->getMembers();
		for (std::set<Client*>::const_iterator mit = members.begin(); mit != members.end(); ++mit)
		{
			if (*mit != exclude)
				(*mit)->send(message);
		}
	}
}

void Server::notifyClients(Channel *channel, const std::string &message, Client *exclude)
{
	if (!channel)
		return;

	const std::set<Client*> &members = channel->getMembers();
	for (std::set<Client*>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		if (*it != exclude)
			(*it)->send(message);
	}
}

void Server::sendNamesList(Client& client, Channel* channel)
{
	if (!channel)
		return;

	std::string names;
	const std::set<Client*>& members = channel->getMembers();

	for (std::set<Client*>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		if (!names.empty())
			names += " ";
		if (channel->isOperator(*it))
			names += "@";
		names += (*it)->getNickname();
	}
	client.send(":" + _hostname + " 353 " + client.getNickname() + " = " + channel->getName() + " :" + names);
	client.send(":" + _hostname + " 366 " + client.getNickname() + " " +channel->getName() + " :End of /NAMES list");
}
