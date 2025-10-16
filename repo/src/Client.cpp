#include "Client.hpp"

Client::Client(Server& server, int fd) : _server(server), _fd(fd), _hostname("localhost"), _registered(false), _password_ok(false), _to_disconnect(false)
{
	std::cout << "new client (fd " << fd << ")" << std::endl;
}

Client::~Client()
{
	close(_fd);
	std::cout << "client (fd " << _fd << "): freed" << std::endl;
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
	return _password_ok;
}

void Client::setPasswordOk()
{
	_password_ok = true;
}

bool Client::toDisconnect() const
{
	return _to_disconnect;
}

void Client::markToDisconnect()
{
	_to_disconnect = true;
}

std::string Client::getPrefix() const
{
	return ":" + _nickname + "!" + _username + "@" + _hostname;
}

std::string& Client::getIn()
{
	return _in;
}

Server &Client::getServer()
{
	return _server;
}

const std::set<std::string> &Client::getChannels() const
{
	return _channels;
}

void Client::sendMessage(const std::string &message) const
{
	std::string msg = message + "\r\n";
	if (send(_fd, msg.c_str(), msg.size(), 0) == -1)
		std::cerr << "Failed to send message to client fd " << _fd << std::endl;
}