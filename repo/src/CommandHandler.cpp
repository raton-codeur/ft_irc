#include "CommandHandler.hpp"
#include "main.hpp"


CommandHandler::CommandHandler(Server &server) : _server(server)
{
	_commands["PASS"] = &CommandHandler::PASS;
	_commands["NICK"] = &CommandHandler::NICK;
	_commands["USER"] = &CommandHandler::USER;
	_commands["JOIN"] = &CommandHandler::JOIN;
	_commands["PRIVMSG"] = &CommandHandler::PRIVMSG;
	_commands["INVITE"] = &CommandHandler::INVITE;
	_commands["KICK"] = &CommandHandler::KICK;
	_commands["PART"] = &CommandHandler::PART;
	_commands["TOPIC"] = &CommandHandler::TOPIC;
}

CommandHandler::~CommandHandler()
{
}

std::vector<std::string> _IRCsplit(const std::string &str)
{
	std::vector<std::string> result;
	std::istringstream iss(str);
	std::string word;

	while (iss >> word)
	{
		if (word[0] == ':')
		{
			std::string rest;
			std::getline(iss, rest);
			result.push_back(word.substr(1) + rest);
			break;
		}
		result.push_back(word);
	}
	return result;
}

std::string CommandHandler::_toUpper(const std::string &s) const
{
	std::string res = s;
	for (size_t i = 0; i < res.size(); ++i)
	{
		if (res[i] >= 'a' && res[i] <= 'z')
			res[i] -= 32;
	}
	return res;
}


void CommandHandler::handleCommand(Client &client, const std::string &input)
{
	std::vector<std::string> args = _IRCsplit(input);
	if (args.empty())
	return;
	std::string cmd = _toUpper(args[0]);
	std::map<std::string, CommandFunction>::iterator it = _commands.find(cmd);
	if (it != _commands.end())
	(this->*(it->second))(client, args);
	else
	std::cout << "Unknown command: " << cmd << std::endl;
}

void CommandHandler::PASS(Client &client, const std::vector<std::string> &args)
{
}

void CommandHandler::NICK(Client &client, const std::vector<std::string> &args)
{
}

void CommandHandler::USER(Client &client, const std::vector<std::string> &args)
{
}

void CommandHandler::JOIN(Client &client, const std::vector<std::string> &args)
{
}

void CommandHandler::PRIVMSG(Client &client, const std::vector<std::string> &args)
{
}

void CommandHandler::INVITE(Client &client, const std::vector<std::string> &args)
{
}

void CommandHandler::KICK(Client &client, const std::vector<std::string> &args)
{
}

void CommandHandler::PART(Client &client, const std::vector<std::string> &args)
{
}

void CommandHandler::TOPIC(Client &client, const std::vector<std::string> &args)
{
}
