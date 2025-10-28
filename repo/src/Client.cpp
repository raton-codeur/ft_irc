#include "Client.hpp"
#include "CommandHandler.hpp"

Client::Client(Server& server, int fd, short& pollEvents) : _server(server), _fd(fd), _pollEvents(pollEvents), _outboxOffset(0), _hostname("localhost"), _registered(false), _passwordOk(false), _welcomeSent(false), _softDisconnect(false), _hardDisconnect(false)
{
	std::cout << "new client (fd " << fd << ")" << std::endl;
}

Client::~Client()
{
	close(_fd);
	std::cout << "fd " << _fd << ": closed" << std::endl;
}

int Client::getFd() const
{
	return _fd;
}

const std::string& Client::getNickname() const
{
	return _nickname;
}

void Client::setNickname(const std::string& nickname)
{
	_nickname = nickname;
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

const std::string& Client::getRealname() const
{
	return _realname;
}
void Client::setRealname(const std::string& realname)
{
	_realname = realname;
}

std::string Client::getHostname() const
{
	return _hostname;
}

std::string Client::getPrefix() const
{
	return ":" + _nickname + "!" + _username + "@" + _hostname;
}

bool Client::isRegistered() const
{
	return _registered;
}

void Client::setRegistered()
{
	_registered = true;
}

bool Client::isPasswordOk() const
{
	return _passwordOk;
}

void Client::setPasswordOk()
{
	_passwordOk = true;
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

const std::set<std::string>& Client::getChannels() const
{
	return _channels;
}

bool Client::isInChannel(const std::string& name) const
{
	return (_channels.find(name) != _channels.end());
}

void Client::addToChannel(const std::string& name)
{
	_channels.insert(name);
}

void Client::removeFromChannel(const std::string& name)
{
	_channels.erase(name);
}

bool Client::isSoftDisconnect() const
{
	return _softDisconnect;
}

bool Client::isHardDisconnect() const
{
	return _hardDisconnect;
}

void Client::setSoftDisconnect(const std::string& logServer, const std::string& logClient)
{
	_softDisconnect = true;
	_disconnectLog = logServer;
	if (!logClient.empty())
		send(logClient);
	removePOLLIN();
}

void Client::setHardDisconnect(const std::string& logServer)
{
	_hardDisconnect = true;
	_disconnectLog = logServer;
}

bool Client::hasEmptyOutbox() const
{
	return _outbox.empty();
}

void Client::printDisconnectLog() const
{
	if (!_disconnectLog.empty())
		std::cout << _disconnectLog << std::endl;
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

void Client::send(const std::string& message)
{
	_outbox.push_back(message + "\r\n");
	addPOLLOUT();
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
			setHardDisconnect("client (fd " + std::to_string(_fd) + "): hard disconnect: disconnected");
			return;
		}
		else if (errno == EINTR)
			checkSignals();
		else if (errno == EAGAIN || errno == EWOULDBLOCK)
			break;
		else
		{
			setHardDisconnect("client (fd " + std::to_string(_fd) + "): hard disconnect: recv error");
			return;
		}
	}
	cmdHandler.parseAndExecute(*this, _inbox);
void Client::tryRegisterClient(const std::string& hostname)
{
	if (_registered || !_password_ok || _nickname.empty() || _username.empty())
		return;
	_registered = true;
	sendWelcome(hostname);
}

void Client::sendWelcome(const std::string &hostname)
{

	std::string nick = _nickname.empty() ? "*" : _nickname;

	sendMessage(":" + hostname + " 001 " + nick + " :Welcome to the IRC network " + hostname);
	sendMessage(":" + hostname + " 002 " + nick + " :Your host is " + hostname);
	sendMessage(":" + hostname + " 003 " + nick + " :This server was created just now");
	sendMessage(":" + hostname + " 004 " + nick + " " + hostname + " 1.0 i o t k l");
	sendMessage(":" + hostname + " 375 " + nick + " :- " + hostname + " Message of the day - ");
	sendMessage(":" + hostname + " 372 " + nick + " :- Vive les crepes!");
	sendMessage(":" + hostname + " 376 " + nick + " :End of /MOTD command.");
}

void Client::handlePOLLOUT()
{
	const char* message;
	size_t len;
	ssize_t n;
	size_t total_sent = 0;

	while (!_outbox.empty())
	{
		message = _outbox.front().c_str() + _outboxOffset;
		len = _outbox.front().size() - _outboxOffset;
		n = ::send(_fd, message, len, 0);
		if (n >= 0)
		{
			total_sent += n;
			_outboxOffset += n;
			if (_outboxOffset == _outbox.front().size())
			{
				_outboxOffset = 0;
				_outbox.pop_front();
			}
			if (total_sent >= _MAX_SEND_PER_CLIENT)
				return;
		}
		else if (errno == EINTR)
			checkSignals();
		else if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		else
		{
			setHardDisconnect("client (fd " + std::to_string(_fd) + "): hard disconnect: send error");
			return;
		}
	}
	removePOLLOUT();
}
