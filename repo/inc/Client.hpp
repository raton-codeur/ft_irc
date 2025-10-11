#pragma once
#include "main.hpp"

class Client
{
	private:

	int _id;
	int _fd;
	int _i_poll;

	Client();
	Client(const Client&);
	Client& operator=(const Client&);

	public:

	Client(int id, int fd);
	~Client();

	int getId() const;
	int getFd() const;
	int getIndexPoll() const;

	void setIndexPoll(int i_poll);
};
