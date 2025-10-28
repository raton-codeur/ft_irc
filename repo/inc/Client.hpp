#pragma once
#include "main.hpp"

class Server;

class Client
{
private:

	Server& _server;
	int _fd;
	std::string _nickname;
	std::string _username;
	std::string _realname;
	std::string _hostname;
	bool _registered;
	bool _password_ok;
	bool _to_disconnect;
	std::string _in;
	std::string _out;
	std::set<std::string> _channels;

	Client();
	Client(const Client&);
	Client& operator=(const Client&);

public:

	Client(Server& server, int fd);
	~Client();

	int getFd() const;
	const std::string& getUsername() const;
	void setUsername(const std::string& username);
	bool hasUsername() const;
	const std::string& getRealname() const;
	void setRealname(const std::string& realname);
	const std::string& getNickname() const;
	void setNickname(const std::string& nickname);
	std::string getHostname() const;
	bool isRegistered() const;
	void setRegistered();
	void addToChannel(const std::string& name);
	void removeFromChannel(const std::string& name);
	bool isInChannel(const std::string& name) const;
	bool isPasswordOk() const;
	void setPasswordOk();
	bool toDisconnect() const;
	void markToDisconnect();
	std::string getPrefix() const;
	std::string& getIn();
	Server& getServer();
	const std::set<std::string>& getChannels() const;
	void tryRegisterClient(const std::string& hostname);
	void sendWelcome(const std::string& hostname);
	void sendMessage(const std::string& message) const;
};
