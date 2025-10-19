#include "CommandHandler.hpp"
#include "main.hpp"

CommandHandler::CommandHandler(Server& server) : _server(server)
{
	_commands["PASS"] = &CommandHandler::pass;
	_commands["NICK"] = &CommandHandler::nick;
	_commands["USER"] = &CommandHandler::user;
	_commands["JOIN"] = &CommandHandler::join;
	_commands["PRIVMSG"] = &CommandHandler::privmsg;
	_commands["INVITE"] = &CommandHandler::invite;
	_commands["KICK"] = &CommandHandler::kick;
	_commands["PART"] = &CommandHandler::part;
	_commands["TOPIC"] = &CommandHandler::topic;
}

CommandHandler::~CommandHandler()
{}

std::vector<std::string> CommandHandler::_split(const std::string& input)
{
	std::vector<std::string> result;
	std::istringstream iss(input);
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

static std::string toUpper(std::string& s)
{
	std::string result(s);
    for (size_t i = 0; i < s.size(); ++i)
        result[i] = std::toupper(static_cast<unsigned char>(s[i]));
    return result;
}

int CommandHandler::handleCommand(Client* client)
{
	std::string line;
	std::stringstream ss(client->getIn());
	std::getline(ss, line);
	client->getIn() = client->getIn().substr(line.size() + 1);
	std::vector<std::string> args = _split(line);
	for (size_t i = 0; i < args.size(); ++i)
		std::cout << "arg[" << i << "] = " << args[i] << std::endl;
	if (args.empty())
		return 0;
	std::string cmd = toUpper(args[0]);
	std::map<std::string, CommandFunction>::iterator it = _commands.find(cmd);
	if (it != _commands.end())
		(this->*(it->second))(*client, args);
	else
		std::cout << args[0] << " :Unknown command" << std::endl;
	return 0;
}

void CommandHandler::pass(Client& client, const std::vector<std::string>& args)
{
	(void)client;
	(void)args;
	std::cout << "PASS command received" << std::endl;
}

void CommandHandler::nick(Client& client, const std::vector<std::string>& args)
{
	(void)client;
	(void)args;
	std::cout << "NICK command received" << std::endl;
}

void CommandHandler::user(Client& client, const std::vector<std::string>& args)
{
	(void)client;
	(void)args;
	std::cout << "USER command received" << std::endl;
}

void CommandHandler::join(Client& client, const std::vector<std::string>& args)
{
	(void)client;
	(void)args;
	std::cout << "JOIN command received" << std::endl;
}

void CommandHandler::privmsg(Client& client, const std::vector<std::string>& args)
{
	(void)client;
	(void)args;
	std::cout << "PRIVMSG command received" << std::endl;
}

void CommandHandler::invite(Client& client, const std::vector<std::string>& args)
{
	(void)client;
	(void)args;
	std::cout << "INVITE command received" << std::endl;
}

void CommandHandler::kick(Client& client, const std::vector<std::string>& args)
{
	(void)client;
	(void)args;
	std::cout << "KICK command received" << std::endl;
}

void CommandHandler::part(Client& client, const std::vector<std::string>& args)
{
	(void)client;
	(void)args;
	std::cout << "PART command received" << std::endl;
}

void CommandHandler::topic(Client& client, const std::vector<std::string>& args)
{
	(void)client;
	(void)args;
	std::cout << "TOPIC command received" << std::endl;
}
