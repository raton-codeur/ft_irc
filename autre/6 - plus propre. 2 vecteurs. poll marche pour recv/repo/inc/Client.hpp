#pragma once
#include "main.hpp"

class Client
{
	private:

	int _id;
	int _fd;
	std::string _in;
	std::string _out;

	Client();
	Client(const Client&);
	Client& operator=(const Client&);

	public:

	Client(int id, int fd);
	~Client();

	int getId() const;
	int getFd() const;
};
