#pragma once
#include "main.hpp"
#include "Server.hpp"
#include "CommandHandler.hpp"

class Client
{
private:

	// constants
	static const size_t _BUFFER_RECV_SIZE = 1024;
	static const size_t _MAX_READ_PER_CYCLE = 8192;
	static const size_t _MAX_CLIENT_INPUT_BUFFER = 16384;

	// attributes
	Server& _server;
	const int _fd;
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
	bool _toDisconnect;
	bool _hardDisconnect;
	bool _welcomeSent;
	std::set<std::string> _channels;

	// disabled
	Client();
	Client(const Client&);
	Client& operator=(const Client&);

public:

	// constructors, destructor
	Client(Server& server, int fd);
	~Client();

	// get, set, add, remove
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
	const std::set<std::string>& getChannels() const;
	std::string getPrefix() const;
	bool isToDisconnect() const;
	bool isHardDisconnect() const;
	void markToDisconnect();
	void markHardDisconnect();

	// others
	bool isReadyforWelcome() const;
	void sendWelcome(const std::string& hostname);
	bool hasWelcomeBeenSent() const;
	void markWelcomeSent();
	void send(const std::string& message);
	void handlePOLLIN(CommandHandler& cmdHandler);
	void handlePOLLOUT();
};
