#include "CommandHandler.hpp"
#include "main.hpp"
#include "Server.hpp"

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

void CommandHandler::processClientBuffer(Client *client , CommandHandler &cmdHandler)
{
	std::string &in = client->getIn();
	size_t pos;

	while (true)
	{
		pos = in.find("\r\n");
		if (pos == std::string::npos)
			pos = in.find('\n'); 

		if (pos == std::string::npos)
			break;

		std::string line = in.substr(0, pos);

		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		in.erase(0, pos + ((in[pos] == '\r' && in[pos + 1] == '\n') ? 2 : 1));

		if (line.empty())
			continue;

		std::cout << "→ Command received: [" << line << "]" << std::endl;

		cmdHandler.handleCommand(client, line);
	}
}

void CommandHandler::handleCommand(Client *client, std::string line)
{
	std::vector<std::string> args = _split(line);

	for (size_t i = 0; i < args.size(); ++i)
		std::cout << "Arg " << i << ": [" << args[i] << "]" << std::endl;
	if (args.empty())
		return;
	std::string cmd = toUpper(args[0]);
	std::map<std::string, CommandFunction>::iterator it = _commands.find(cmd);
	if (it != _commands.end())
		(this->*(it->second))(*client, args);
	else
		std::cout << args[0] << " :Unknown command" << std::endl;
}

void CommandHandler::pass(Client& client, const std::vector<std::string>& args)
{
	if (client.isRegistered())
	{
		client.sendMessage(":" + _server.getHostname() + " 462 " + client.getNickname() + " :You may not reregister");
		return;
	}
	if (args.size() < 2)
	{
		client.sendMessage(":" + _server.getHostname() + " 461 PASS :Not enough parameters");
		return;
	}
	if (args[1] != client.getServer().getPassword())
	{
		client.sendMessage(":" + _server.getHostname() + " 464 :Password incorrect");
		client.markToDisconnect();
		return;
	}
	client.setPasswordOk();
}

static bool isValidNickname(const std::string& nickname)
{
	if (nickname.empty() || nickname.size() > 9)
		return false;
	char first = nickname[0];
	if (!std::isalpha(static_cast<unsigned char>(first)) &&
		first != '[' && first != ']' && first != '\\' && first != '^' &&
		first != '{' && first != '}' && first != '-' )
		return false;
	for (size_t i = 1; i < nickname.size(); ++i)
	{
		char c = nickname[i];
		if (!std::isalnum(static_cast<unsigned char>(c)) &&
			c != '[' && c != ']' && c != '\\' && c != '^' &&
			c != '{' && c != '}' && c != '-' )
			return false;
	}
	return true;
}

void CommandHandler::nick(Client& client, const std::vector<std::string>& args)
{
	std::cout << "NICK command received" << std::endl;
	if (args.size() < 2)
	{
		client.sendMessage(":" + _server.getHostname() + " 431 :No nickname given");
		return;
	}
	std::string new_nick = args[1];
	if (!isValidNickname(new_nick))
	{
		client.sendMessage(":" + _server.getHostname() + " 432 " + new_nick + " :Erroneous nickname");
		return;
	}
	if (_server.getClientByNick(new_nick) != nullptr)
	{
		client.sendMessage(":" + _server.getHostname() + " 433 " + new_nick + " :Nickname is already in use");
		return;
	}
	std::string old_nick = client.getNickname();
	if (!old_nick.empty())
		_server.removeClientFromNickMap(old_nick);
	client.setNickname(new_nick);
	_server.addClientToNickMap(new_nick, &client);
	if (client.hasUsername() && client.isPasswordOk() && !client.isRegistered())
	{
		client.setRegistered();
		client.sendMessage(":" + _server.getHostname() + " 001 " + client.getNickname() + " :Welcome to the IRC network By Qhauuy & Jteste");
	}
	std::string msg = ":" + old_nick + "!" + client.getUsername() + "@" + client.getHostname() + " NICK :" + new_nick;
	_server.notifyClients(client.getChannels(), msg, &client);
}

void CommandHandler::user(Client& client, const std::vector<std::string>& args)
{
	std::cout << "USER command received" << std::endl;
	if (client.isRegistered())
	{
		client.sendMessage(":" + _server.getHostname() + " 462 " + client.getNickname() + " :You may not reregister");
		return;
	}
	if(args.size() < 5)
	{
		client.sendMessage(":" + _server.getHostname() + " 461 USER :Not enough parameters");
		return;
	}
	std::string username = args[1];
	std::string realname;
	if (!args[4].empty() && args[4][0] == ':')
		realname = args[4].substr(1);
	else
		realname = args[4];
	client.setUsername(username);
	client.setRealname(realname);
	if (!client.getNickname().empty() && client.isPasswordOk() && !client.isRegistered())
	{
		client.setRegistered();
		client.sendMessage(":" + _server.getHostname() + " 001 " + client.getNickname() + " :Welcome to the IRC network By Qhauuy & Jteste");
	}
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
