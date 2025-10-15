#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _hostname("localhost"), _registered(false)
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

std::string Client::getPrefix() const
{
	return ":" + _nickname + "!" + _username + "@" + _hostname;
}
