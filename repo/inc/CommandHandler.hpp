#pragma once
#include "main.hpp"

class Client;
class Server;

class CommandHandler
{
private:

	// constant
	static const size_t _MAX_LINE_LENGTH = 512;

	// typedef
	typedef void (CommandHandler::*CommandFunction)(Client&, const std::vector<std::string>&);

	// attributes
	Server& _server;
	std::map<std::string, CommandFunction> _commands;

	// commands
	void ping(Client& client, const std::vector<std::string>& args);
	void cap(Client& client, const std::vector<std::string>& args);
	void pass(Client& client, const std::vector<std::string>& args);
	void nick(Client& client, const std::vector<std::string>& args);
	void user(Client& client, const std::vector<std::string>& args);
	void join(Client& client, const std::vector<std::string>& args);
	void part(Client& client, const std::vector<std::string>& args);
	void privmsg(Client& client, const std::vector<std::string>& args);
	void kick(Client& client, const std::vector<std::string>& args);
	void invite(Client& client, const std::vector<std::string>& args);
	void topic(Client& client, const std::vector<std::string>& args);
	void mode(Client& client, const std::vector<std::string>& args);
	void quit(Client& client, const std::vector<std::string>& args);

	// utils
	bool checkRegistered(Client& client);
	void partSingleChannel(Client& client, const std::string& channel_name, const std::string& reason = "");

	// disabled
	CommandHandler();
	CommandHandler(const CommandHandler&);
	CommandHandler& operator=(const CommandHandler&);

public:

	// constructor, destructor
	CommandHandler(Server& server);
	~CommandHandler();

	// main
	void parseAndExecute(Client& client, std::string& inbox);
};
