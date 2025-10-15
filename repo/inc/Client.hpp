#pragma once
#include "main.hpp"

class Client
{
private:

	int _fd;
	std::string _nickname;
	std::string _username;
	std::string _realname;
	std::string _hostname;
	bool _registered;
	std::string _in;
	std::string _out;
	std::set<std::string> _channels;

	Client();
	Client(const Client&);
	Client& operator=(const Client&);

public:

	Client(int fd);
	~Client();

	int getFd() const;
	const std::string& getNickname() const;
	void setNickname(const std::string& nickname);
	const std::string& getUsername() const;
	void setUsername(const std::string& username);
	bool isRegistered() const;
	void setRegistered();
	void addToChannel(const std::string& name);
	void removeFromChannel(const std::string& name);
	bool isInChannel(const std::string& name) const;
	std::string getPrefix() const;
};
