#pragma once

#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "main.hpp"

class CommandHandler
{
private:

	typedef void (CommandHandler::*CommandFunction)(Client &, const std::vector<std::string> &);
	Server &_server;
	std::map<std::string, CommandFunction> _commands;
	std::vector<std::string> _IRCsplit(const std::string &str);
	std::string _toUpper(const std::string &s) const;

	void PASS(Client &client, const std::vector<std::string> &args);
	void NICK(Client &client, const std::vector<std::string> &args);
	void USER(Client &client, const std::vector<std::string> &args);
	void JOIN(Client &client, const std::vector<std::string> &args);
	void PRIVMSG(Client &client, const std::vector<std::string> &args);
	void INVITE(Client &client, const std::vector<std::string> &args);
	void KICK(Client &client, const std::vector<std::string> &args);
	void PART(Client &client, const std::vector<std::string> &args);
	void TOPIC(Client &client, const std::vector<std::string> &args);

public: 
	CommandHandler(Server &server);
	~CommandHandler();
	void handleCommand(Client &client, const std::string &input);
};
