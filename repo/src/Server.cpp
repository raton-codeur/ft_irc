#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

static void checkArgs(int argc, char** argv)
{
	if (argc != 3)
		errorAndThrow("usage: ./ircserv <port> <password>");
	char* end;
	long port = std::strtol(argv[1], &end, 10);
	if (*end != '\0' || port < 1024 || port > 65535)
		errorAndThrow("error: invalid port number (must be an integer between 1024 and 65535)");
	if (!isValidPassword(argv[2]))
		errorAndThrow("error: invalid password (must be 1-32 characters long, no whitespaces)");
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

	struct pollfd p;
	p.fd = _server_fd;
	p.events = POLLIN;
	p.revents = 0;
	_pollArray.push_back(p);

	_clients.push_back(NULL);
}

Server::~Server()
{
	if (_server_fd != -1)
		close(_server_fd);
	for (size_t i = 1; i < _clients.size(); ++i)
		delete _clients[i];
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		delete it->second;
}

void Server::run()
{
	if (poll(_pollArray.data(), _pollArray.size(), -1) == -1)
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
	struct pollfd p;
	p.fd = client_fd;
	p.events = POLLIN;
	p.revents = 0;
	_pollArray.push_back(p);

	_clients.push_back(new Client(*this, client_fd, _clients.size()));
}

void Server::acceptClients()
{
	int client_fd;

	if (_pollArray[0].revents & (POLLHUP | POLLERR | POLLNVAL))
		errorAndThrow("server socket error");
	if (_pollArray[0].revents & POLLIN)
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
		if (_pollArray[i].revents)
		{
			if (_pollArray[i].revents & (POLLHUP | POLLERR | POLLNVAL))
			{
				if (_pollArray[i].revents & POLLNVAL)
					_clients[i]->setHardDisconnect("disconnected client: invalid fd");
				else if (_pollArray[i].revents & POLLERR)
					_clients[i]->setHardDisconnect("disconnected client: network error");
				else
					_clients[i]->setHardDisconnect("disconnected client");
				continue;
			}
			if (_pollArray[i].revents & POLLIN)
				_clients[i]->handlePOLLIN(_cmdHandler);
			if (_pollArray[i].revents & POLLOUT && !_clients[i]->isHardDisconnect())
				_clients[i]->handlePOLLOUT();
		}
	}
}

void Server::detachClientFromChannels(Client& client)
{
	std::set<std::string> channels(client.getChannels());
	for (std::set<std::string>::const_iterator it = channels.begin(); it != channels.end(); ++it)
	{
		const std::string& name = *it;
		Channel* channel = getChannel(name);
		if (!channel)
		{
			client.removeFromChannel(name);
			continue;
		}
		channel->removeMember(&client);
		if (channel->isOperator(&client))
			channel->removeOperator(&client);
		channel->removeInvite(&client);
		client.removeFromChannel(name);

		if (!channel->getMembers().empty() && channel->getOperators().empty())
		{
			Client* new_op = *(channel->getMembers().begin());
			channel->addOperator(new_op);
			std::string op_msg = ":" + new_op->getPrefix() + " MODE " + channel->getName() + " +o " + new_op->getNickname();
			notifyChannelMembers(channel, op_msg, NULL);
		}
		if (channel->getMembers().empty())
			deleteChannel(name);
	}
}

void Server::deleteClient(size_t i)
{
	Client* client = _clients[i];
	detachClientFromChannels(*client);
	removeClientFromNickMap(client->getNickname());
	delete client;
	if (i == _clients.size() - 1)
	{
		_clients.pop_back();
		_pollArray.pop_back();
		return;
	}
	_clients[i] = _clients.back();
	_clients[i]->setI(i);
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
		std::cout << "channel " << name << ": deleted" << std::endl;
	}
}

void Server::removePOLLIN(size_t i)
{
	_pollArray[i].events &= ~POLLIN;
}

void Server::removePOLLOUT(size_t i)
{
	_pollArray[i].events &= ~POLLOUT;
}

void Server::addPOLLOUT(size_t i)
{
	_pollArray[i].events |= POLLOUT;
}

void Server::notifyChannelMembers(Channel* channel, const std::string& message, Client* exclude = NULL)
{
	if (!channel || message.empty())
		return;

	const std::set<Client*>& members = channel->getMembers();
	for (std::set<Client*>::const_iterator it = members.begin(); it != members.end(); ++it)
		{
		if (*it && *it != exclude)
			(*it)->send(message);
	}
}

void Server::notifyChannelMembers(const std::set<std::string>& channels, const std::string& message, Client* exclude = NULL)
{
	Channel* channel;
	std::set<Client*> targets;

	if (message.empty())
		return;
	for (std::set<std::string>::const_iterator it = channels.begin(); it != channels.end(); ++it)
	{
		channel = getChannel(*it);
		if (!channel)
			continue;
		targets.insert(channel->getMembers().begin(), channel->getMembers().end());
	}
	for (std::set<Client*>::const_iterator it = targets.begin(); it != targets.end(); ++it)
	{
		if (*it && *it != exclude)
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
