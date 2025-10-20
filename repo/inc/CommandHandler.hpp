#pragma once
#include "Client.hpp"
#include "Channel.hpp"
#include "main.hpp"

class Server;

class CommandHandler
{
private:

	typedef void (CommandHandler::*CommandFunction)(Client&, const std::vector<std::string>&);

	Server& _server;
	std::map<std::string, CommandFunction> _commands;

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

	std::vector<std::string> _split(const std::string& input);

	CommandHandler();
	CommandHandler(const CommandHandler&);
	CommandHandler& operator=(const CommandHandler&);

public:

	CommandHandler(Server& server);
	~CommandHandler();

	void handleCommand(Client* client, std::string line);
	void processClientBuffer(Client *client, CommandHandler &cmdHandler);

	bool checkRegistered(Client& client);
};
