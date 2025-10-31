#include "Server.hpp"
#include "Client.hpp"
#include "CommandHandler.hpp"

Client::Client(Server& server, int fd, size_t i) : _server(server), _fd(fd), _i(i), _outboxOffset(0), _hostname("localhost"), _passwordOk(false), _registered(false), _softDisconnect(false), _hardDisconnect(false)
{
	std::cout << "new client" << std::endl;
}

Client::~Client()
{
	close(_fd);
}

int Client::getFd() const
{
	return _fd;
}

size_t Client::getI() const
{
	return _i;
}

void Client::setI(size_t i)
{
	_i = i;
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

bool Client::isPasswordOk() const
{
	return _passwordOk;
}

void Client::setPasswordOk()
{
	_passwordOk = true;
}

bool Client::isRegistered() const
{
	return _registered;
}

void Client::setRegistered()
{
	_registered = true;
}

void Client::sendWelcome()
{
	std::string nick = _nickname.empty() ? "*" : _nickname;
	send(":" + _server.getHostname() + " 001 " + nick + " :Welcome to the IRC network " + _server.getHostname());
	send(":" + _server.getHostname() + " 002 " + nick + " :Your host is " + _server.getHostname());
	send(":" + _server.getHostname() + " 003 " + nick + " :This server was created just now");
	send(":" + _server.getHostname() + " 004 " + nick + " " + _server.getHostname() + " 1.0 i o t k l");
	send(":" + _server.getHostname() + " 375 " + nick + " :- " + _server.getHostname() + " Message of the day - ");
	send(":" + _server.getHostname() + " 372 " + nick + " :- Vive les crepes!");
	send(":" + _server.getHostname() + " 376 " + nick + " :End of /MOTD command.");
}

void Client::tryRegisterClient()
{
	if (_registered || !_passwordOk || _nickname.empty() || _username.empty())
		return;
	_registered = true;
	sendWelcome();
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
	_server.removePOLLIN(_i);
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

void Client::send(const std::string& message)
{
	_outbox.push_back(message + "\r\n");
	_server.addPOLLOUT(_i);
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
			setHardDisconnect("disconnected client");
			return;
		}
		else if (errno == EINTR)
			checkSignals();
		else if (errno == EAGAIN || errno == EWOULDBLOCK)
			break;
		else
		{
			setHardDisconnect("disconnected client: recv error");
			return;
		}
	}
	cmdHandler.parseAndExecute(*this, _inbox);
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
			setHardDisconnect("disconnected client: send error");
			return;
		}
	}
	_server.removePOLLOUT(_i);
}
