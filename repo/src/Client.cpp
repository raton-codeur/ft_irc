#include "Client.hpp"

Client::Client(int id, int fd) : _id(id), _fd(fd)
{
	std::cout << "new client: " << id << std::endl;
}

Client::~Client()
{
	close(_fd);
	std::cout << "[ client " << _id << ": fd has been closed ]";
}

int Client::getId() const
{
	return _id;
}

int Client::getFd() const
{
	return _fd;
}
