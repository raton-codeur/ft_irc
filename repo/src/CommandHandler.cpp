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
	size_t i = 0;
    size_t n = input.size();

	while (i < n && input[i] == ' ')
        ++i;

	if (i < n && input[i] == ':')
	{
        while (i < n && input[i] != ' ')
            ++i;
        while (i < n && input[i] == ' ')
            ++i;
    }

	while (i < n)
	{
		if (input[i] == ':')
		{
			result.push_back(input.substr(i + 1));
			break;
		}
		size_t j = i;
		while (j < n && input[j] != ' ')
			++j;
		result.push_back(input.substr(i, j - i));
		i = j;
		while (i < n && input[i] == ' ')
			++i;
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

void CommandHandler::handleCommand(Client& client, const std::string& input)
{
	std::vector<std::string> args = _split(input);
	if (args.empty())
		return;
	std::string cmd = toUpper(args[0]);
	std::map<std::string, CommandFunction>::iterator it = _commands.find(cmd);
	if (it != _commands.end())
		(this->*(it->second))(client, args);
	else
		std::cout << args[0] << " :Unknown command" << std::endl;
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
