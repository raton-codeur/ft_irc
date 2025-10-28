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
	short& _pollEvents;
	std::string _inbox;
	char _bufferRecv[_BUFFER_RECV_SIZE];
	std::deque<std::string> _outbox;
	size_t _outboxOffset;
	std::string _nickname;
	std::string _username;
	std::string _realname;
	std::string _hostname;
	bool _registered;
	bool _passwordOk;
	bool _softDisconnect;
	bool _hardDisconnect;
	std::string _disconnectLog;
	bool _password_ok;
	bool _to_disconnect;
	std::string _in;
	std::string _out;
	std::set<std::string> _channels;

	// utils
	void removePOLLIN();
	void removePOLLOUT();
	void addPOLLOUT();

	// disabled
	Client();
	Client(const Client&);
	Client& operator=(const Client&);

public:

	// constructor, destructor
	Client(Server& server, int fd, short& pollEvents);
	~Client();

	// get, set
	int getFd() const;
	const std::string& getNickname() const;
	void setNickname(const std::string& nickname);
	const std::string& getUsername() const;
	void setUsername(const std::string& username);
	bool hasUsername() const;
	const std::string& getRealname() const;
	void setRealname(const std::string& realname);
	std::string getHostname() const;
	std::string getPrefix() const;

	// get, set <-> init
	bool isRegistered() const;
	void setRegistered();
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
	bool hasWelcomeBeenSent() const;
	void markWelcomeSent();

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
