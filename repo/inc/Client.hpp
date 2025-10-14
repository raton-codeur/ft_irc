#pragma once
#include "main.hpp"

class Client
{
	private:

	int _id;
	int _fd;

	std::string _nickname;
	std::string _username;
	std::string _hostname;
	std::string _realname;
	bool 		_authenticated;

	std::string _in;
	std::string _out;

	std::set<std::string> _channels;

	Client();
	Client(const Client&);
	Client& operator=(const Client&);

	public:

	Client(int id, int fd, std::string &clientIP);
	~Client();

	int getId() const;
	int getFd() const;

	const std::string &getNick() const;
	void setNick(const std::string &nick);

	const std::string &getUser() const;
	void setUser(const std::string &user);

	bool isAuthenticated() const;
	void setAuthenticated(bool auth);

	void addChannel(const std::string &name);
	void removeChannel(const std::string &name);
	bool isInChannel(const std::string &name) const;

	std::string getPrefix() const;
	void initHostName();
};
