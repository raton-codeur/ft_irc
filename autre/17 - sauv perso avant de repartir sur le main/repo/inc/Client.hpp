#pragma once
#include "main.hpp"

class Server;
class CommandHandler;

class Client
{
private:

	// constants
	static const size_t _BUFFER_RECV_SIZE = 1024;
	static const size_t _MAX_RECV_PER_CLIENT = 2048;
	static const size_t _MAX_SEND_PER_CLIENT = 2048;

	// attributes
	Server& _server;
	const int _fd;
	size_t _i;
	std::string _inbox;
	char _bufferRecv[_BUFFER_RECV_SIZE];
	std::deque<std::string> _outbox;
	size_t _outboxOffset;
	std::string _nickname;
	std::string _username;
	std::string _realname;
	std::string _hostname;
	bool _passwordOk;
	bool _registered;
	bool _softDisconnect;
	bool _hardDisconnect;
	std::string _disconnectLog;
	std::set<std::string> _channels;

	// others
	void sendWelcome();

	// disabled
	Client();
	Client(const Client&);
	Client& operator=(const Client&);

public:

	// constructor, destructor
	Client(Server& server, int fd, size_t i);
	~Client();

	// get, set
	int getFd() const;
	size_t getI() const;
	void setI(size_t i);
	const std::string& getNickname() const;
	void setNickname(const std::string& nickname);
	const std::string& getUsername() const;
	void setUsername(const std::string& username);
	bool hasUsername() const;
	const std::string& getRealname() const;
	void setRealname(const std::string& realname);
	std::string getHostname() const;
	std::string getPrefix() const;

	// <-> init
	bool isPasswordOk() const;
	void setPasswordOk();
	bool isRegistered() const;
	void setRegistered();
	void tryRegisterClient();

	// <-> channels
	const std::set<std::string>& getChannels() const;
	bool isInChannel(const std::string& name) const;
	void addToChannel(const std::string& name);
	void removeFromChannel(const std::string& name);

	// <-> disconnect
	bool isSoftDisconnect() const;
	bool isHardDisconnect() const;
	void setSoftDisconnect(const std::string& logServer = "", const std::string& logClient = "");
	void setHardDisconnect(const std::string& logServer = "");
	bool hasEmptyOutbox() const;
	void printDisconnectLog() const;

	// others
	void send(const std::string& message);
	void handlePOLLIN(CommandHandler& cmdHandler);
	void handlePOLLOUT();
};
