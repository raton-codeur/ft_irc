#pragma once
#include "main.hpp"

class Client
{
	private:

	int _id;
	int _fd;

	Client();
	Client(const Client&);
	Client& operator=(const Client&);

	public:

	Client(int id, int fd);
	~Client();

	int getId() const;
	int getFd() const;

	typedef std::list<Client>::iterator It;
};
