#include "Client.hpp"

Client::Client(int id, int fd, std::string &clientIP) : _id(id), _fd(fd), _authenticated(false), _hostname(clientIP)
{
	std::cout << "new client: " << id << std::endl;
}

Client::~Client()
{
	close(_fd);
	std::cout << "client " << _id << ": fd has been closed" << std::endl;
	std::cout << "client " << _id << ": freed" << std::endl;
}

int Client::getId() const
{
	return _id;
}

int Client::getFd() const
{
	return _fd;
}

const std::string &Client::getNick() const
{
	return _nickname;
}

void Client::setNick(const std::string &nick)
{
	_nickname = nick;
}

const std::string &Client::getUser() const
{
	return _username;
}

void Client::setUser(const std::string &user)
{
	_username = user;
}

bool Client::isAuthenticated() const
{
	return _authenticated;
}

void Client::setAuthenticated(bool auth)
{
	_authenticated = auth;
}

void Client::addChannel(const std::string &name)
{
	_channels.insert(name);
}

void Client::removeChannel(const std::string &name)
{
	_channels.erase(name);
}

bool Client::isInChannel(const std::string &name) const
{
	return (_channels.find(name) != _channels.end());
}

std::string Client::getPrefix() const
{
	return std::string(":" + _nickname + "!" + _username + "@"+ _hostname);
}

