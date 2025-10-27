#include "Client.hpp"

Client::Client(Server& server, int fd, short& pollEvents) : _server(server), _fd(fd), _pollEvents(pollEvents), _outboxOffset(0), _hostname("localhost"), _registered(false), _passwordOk(false), _softDisconnect(false), _hardDisconnect(false), _welcomeSent(false)
{
	std::cout << "new client (fd " << fd << ")" << std::endl;
}

int Client::getFd() const
{
	return _fd;
}

Client::~Client()
{
	close(_fd);
	std::cout << "fd " << _fd << ": closed" << std::endl;
}

const std::string& Client::getNickname() const
{
	return _nickname;
}

void Client::setNickname(const std::string& nickname)
{
	_nickname = nickname;
}

std::string Client::getHostname() const
{
	return _hostname;
}

const std::string& Client::getUsername() const
{
	return _username;
}

void Client::setUsername(const std::string& username)
{
	_username = username;
}

bool Client::hasUsername() const
{
	return !_username.empty();
}

const std::string &Client::getRealname() const
{
	return _realname;
}
void Client::setRealname(const std::string &realname)
{
	_realname = realname;
}

bool Client::isRegistered() const
{
	return _registered;
}

void Client::setRegistered()
{
	_registered = true;
}

void Client::addToChannel(const std::string& name)
{
	_channels.insert(name);
}

void Client::removeFromChannel(const std::string& name)
{
	_channels.erase(name);
}

bool Client::isInChannel(const std::string& name) const
{
	return (_channels.find(name) != _channels.end());
}

bool Client::isPasswordOk() const
{
	return _passwordOk;
}

void Client::setPasswordOk()
{
	_passwordOk = true;
}

bool Client::isSoftDisconnect() const
{
	return _softDisconnect;
}

bool Client::isHardDisconnect() const
{
	return _hardDisconnect;
}

void Client::setSoftDisconnect()
{
	_softDisconnect = true;
	removePOLLIN();
}

void Client::setHardDisconnect()
{
	_hardDisconnect = true;
}

std::string Client::getPrefix() const
{
	return ":" + _nickname + "!" + _username + "@" + _hostname;
}

void Client::send(const std::string& message)
{
	_outbox.push_back(message);
	addPOLLOUT();
}

const std::set<std::string>& Client::getChannels() const
{
	return _channels;
}

bool Client::isReadyforWelcome() const
{
	return (!isRegistered() && hasUsername() && !getNickname().empty() && isPasswordOk());
}

void Client::sendWelcome(const std::string& hostname)
{
	if (isReadyforWelcome() && !hasWelcomeBeenSent())
	{
		setRegistered();
		send(":" + hostname + " 001 " + getNickname() + " :Welcome to the IRC network " + hostname);
		markWelcomeSent();
	}
}

bool Client::hasWelcomeBeenSent() const
{
	return _welcomeSent;
}

void Client::markWelcomeSent()
{
	_welcomeSent = true;
}

void Client::removePOLLIN()
{
	_pollEvents &= ~POLLIN;
}

void Client::removePOLLOUT()
{
	_pollEvents &= ~POLLOUT;
}

void Client::addPOLLOUT()
{
	_pollEvents |= POLLOUT;
}

bool Client::hasEmptyOutbox() const
{
	return _outbox.empty();
}




















void Client::handlePOLLIN(CommandHandler& cmdHandler)
{
	size_t total_read = 0;
	ssize_t n;

	while (true)
	{
		n = recv(_fd, _bufferRecv, _BUFFER_RECV_SIZE, 0);
		if (n > 0)
		{
			_inbox.append(_bufferRecv, n);
			total_read += n;
			if (total_read >= _MAX_RECV_PER_CLIENT)
				break;
		}
		else if (n == 0)
		{
			std::cout << "client (fd " << _fd << "): hard disconnect: disconnected" << std::endl;
			setHardDisconnect();
			return;
		}
		else if (errno == EINTR)
			checkSignals();
		else if (errno == EAGAIN || errno == EWOULDBLOCK)
			break;
		else
		{
			std::cout << "client (fd " << _fd << "): hard disconnect: error" << std::endl;
			setHardDisconnect();
			return;
		}
	}
	cmdHandler.parseAndExecute(*this, _inbox);
}








void Client::handlePOLLOUT()
{
	while (!_outbox.empty())
	{
		std::string s = _outbox.front() + "\n";
		::send(_fd, s.c_str(), s.size(), 0);
		_outbox.pop_front();
	}
	removePOLLOUT();
}

// void Client::handlePOLLOUT()
// {
// 	const char* s;
// 	size_t len;
// 	ssize_t n;

// 	while (!client.outbox.empty())
// 	{
// 		s = client.outbox.front().c_str() + client.outboxOffset;
// 		len = client.outbox.front().size() - client.outboxOffset;
// 		n = send(client.fd, s, len, 0);
// 		if (n > 0)
// 		{
// 			client.outboxOffset += n;
// 			if (client.outboxOffset == client.outbox.front().size())
// 		}
// 		if (client.outboxOffset == client.outbox.front().size())
// 			client.outbox.pop_front();
// 	}


// }
