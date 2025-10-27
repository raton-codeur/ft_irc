#pragma once
#include "main.hpp"

class Client;

class CommandHandler
{
private:

	// constants
	static const size_t _MAX_LINE_LENGTH = 512;

	// types
	typedef void (CommandHandler::*CommandFunction)(Client&, const std::vector<std::string>&);

	// attributes
	std::map<std::string, CommandFunction> _commands;

	// commands
	void cap(Client& client, const std::vector<std::string>& args);
	void pass(Client& client, const std::vector<std::string>& args);
	void nick(Client& client, const std::vector<std::string>& args);
	void user(Client& client, const std::vector<std::string>& args);
	void join(Client& client, const std::vector<std::string>& args);
	void privmsg(Client& client, const std::vector<std::string>& args);
	void invite(Client& client, const std::vector<std::string>& args);
	void kick(Client& client, const std::vector<std::string>& args);
	void part(Client& client, const std::vector<std::string>& args);
	void topic(Client& client, const std::vector<std::string>& args);
	void ping(Client& client, const std::vector<std::string>& args);
	void mode(Client& client, const std::vector<std::string>& args);

	// disabled
	CommandHandler(const CommandHandler&);
	CommandHandler& operator=(const CommandHandler&);

public:

	// constructors, destructor
	CommandHandler();
	~CommandHandler();

	// main method
	void parseAndExecute(Client& client, std::string& inbox);
};
