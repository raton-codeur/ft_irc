#include "CommandHandler.hpp"
#include "Client.hpp"

CommandHandler::CommandHandler()
{
	// _commands["CAP"] = &CommandHandler::cap;
	// _commands["PASS"] = &CommandHandler::pass;
	// _commands["NICK"] = &CommandHandler::nick;
	// _commands["USER"] = &CommandHandler::user;
	// _commands["JOIN"] = &CommandHandler::join;
	// _commands["PRIVMSG"] = &CommandHandler::privmsg;
	// _commands["INVITE"] = &CommandHandler::invite;
	// _commands["KICK"] = &CommandHandler::kick;
	// _commands["PART"] = &CommandHandler::part;
	// _commands["TOPIC"] = &CommandHandler::topic;
	// _commands["PING"] = &CommandHandler::ping;
	// _commands["MODE"] = &CommandHandler::mode;
}

CommandHandler::~CommandHandler()
{}











static std::string toUpper(const std::string& s)
{
	std::string result(s);
    for (size_t i = 0; i < s.size(); ++i)
        result[i] = std::toupper(static_cast<unsigned char>(s[i]));
    return result;
}

static std::vector<std::string> split(const std::string& line)
{
	std::vector<std::string> result;
	std::istringstream iss(line);
	std::string word;

	while (iss >> word)
	{
		if (word[0] == ':')
		{
			std::string remaining;
			std::getline(iss, remaining);
			result.push_back(word.substr(1) + remaining);
			break;
		}
		result.push_back(word);
	}
	return result;
}

void CommandHandler::parseAndExecute(Client& client, std::string& inbox)
{
	size_t startLine = 0, endLine;
	std::string line, command;
	std::vector<std::string> args;

	endLine = inbox.find("\n");
	while (endLine != std::string::npos)
	{
		if (endLine - startLine + 1 > _MAX_LINE_LENGTH)
		{
			std::cout << "client (fd " << client.getFd() << "): disconnected (line too long)" << std::endl;
			client.markToDisconnect();
			return;
		}
		if (endLine > 0 && inbox[endLine - 1] == '\r')
			line = inbox.substr(startLine, endLine - startLine - 1);
		else
			line = inbox.substr(startLine, endLine - startLine);
		args = split(line);
		for (size_t i = 0; i < args.size(); ++i)
			std::cout << "arg[" << i << "]: <" << args[i] << ">" << std::endl;
		if (!args.empty())
		{
			command = toUpper(args[0]);
			std::map<std::string, CommandFunction>::iterator it = _commands.find(command);
			if (it != _commands.end())
				(this->*(it->second))(client, args);
			else
				std::cout << args[0] << ": unknown command" << std::endl;
		}
		startLine = endLine + 1;
		endLine = inbox.find("\n", startLine);
	}
	inbox.erase(0, startLine);
}

















































// void CommandHandler::cap(Client &client, const std::vector<std::string> &args)
// {
// 	if (args.size() < 2)
// 	{
// 		client.sendMessage(":" + _server.getHostname() + " 461 CAP :Not enough parameters");
// 		return;
// 	}
// 	std::string subcommand = args[1];
// 	subcommand = toUpper(subcommand);
// 	if (subcommand == "LS")
// 		client.sendMessage(":" + _server.getHostname() + " CAP * LS :multi-prefix");
// 	else if (subcommand == "REQ")
// 	{
// 		std::string requested = (args.size() >= 3 ? args[2] : "");
// 		std::string nick = client.getNickname().empty() ? "*" : client.getNickname();
// 		if (requested.find("multi-prefix") != std::string::npos)
// 			client.sendMessage(":" + _server.getHostname() + " CAP " + nick + " ACK :" + requested);
// 		else
// 			client.sendMessage(":" + _server.getHostname() + " CAP " + nick + " NAK :" + requested);
// 	}
// 	else if (subcommand == "END")
// 		client.sendWelcome(_server.getHostname());
// 	else
// 		client.sendMessage(":" + _server.getHostname() + " 410 " + client.getNickname() + " " + subcommand + " :Unknown CAP subcommand");

// }

// void CommandHandler::pass(Client &client, const std::vector<std::string> &args)
// {
// 	if (client.isRegistered())
// 	{
// 		client.sendMessage(":" + _server.getHostname() + " 462 " + client.getNickname() + " :You may not reregister");
// 		return;
// 	}
// 	if (args.size() < 2)
// 	{
// 		client.sendMessage(":" + _server.getHostname() + " 461 PASS :Not enough parameters");
// 		return;
// 	}
// 	if (args[1] != client.getServer().getPassword())
// 	{
// 		client.sendMessage(":" + _server.getHostname() + " 464 :Password incorrect");
// 		client.markToDisconnect();
// 		return;
// 	}
// 	client.setPasswordOk();
// }

// static bool isValidNickname(const std::string& nickname)
// {
// 	if (nickname.empty() || nickname.size() > 9)
// 		return false;
// 	char first = nickname[0];
// 	if (!std::isalpha(static_cast<unsigned char>(first)) &&
// 		first != '[' && first != ']' && first != '\\' && first != '^' &&
// 		first != '{' && first != '}' && first != '-' )
// 		return false;
// 	for (size_t i = 1; i < nickname.size(); ++i)
// 	{
// 		char c = nickname[i];
// 		if (!std::isalnum(static_cast<unsigned char>(c)) &&
// 			c != '[' && c != ']' && c != '\\' && c != '^' &&
// 			c != '{' && c != '}' && c != '-' )
// 			return false;
// 	}
// 	return true;
// }

// void CommandHandler::nick(Client& client, const std::vector<std::string>& args)
// {
// 	std::cout << "NICK command received" << std::endl;
// 	if (args.size() < 2)
// 	{
// 		client.sendMessage(":" + _server.getHostname() + " 431 :No nickname given");
// 		return;
// 	}
// 	std::string new_nick = args[1];
// 	if (!isValidNickname(new_nick))
// 	{
// 		client.sendMessage(":" + _server.getHostname() + " 432 " + new_nick + " :Erroneous nickname");
// 		return;
// 	}
// 	if (_server.getClientByNick(new_nick) != nullptr)
// 	{
// 		client.sendMessage(":" + _server.getHostname() + " 433 " + new_nick + " :Nickname is already in use");
// 		return;
// 	}
// 	std::string old_nick = client.getNickname();
// 	if (!old_nick.empty())
// 		_server.removeClientFromNickMap(old_nick);
// 	client.setNickname(new_nick);
// 	_server.addClientToNickMap(new_nick, &client);
// 	client.sendWelcome(_server.getHostname());
// 	if (!old_nick.empty())
// 	{
// 		std::string msg = ":" + old_nick + "!" + client.getUsername() + "@" + client.getHostname() + " NICK :" + new_nick;
// 		_server.notifyClients(client.getChannels(), msg, &client);
// 	}
// }

// void CommandHandler::user(Client& client, const std::vector<std::string>& args)
// {
// 	std::cout << "USER command received" << std::endl;
// 	if (client.isRegistered())
// 	{
// 		client.sendMessage(":" + _server.getHostname() + " 462 " + client.getNickname() + " :You may not reregister");
// 		return;
// 	}
// 	if(args.size() < 5)
// 	{
// 		client.sendMessage(":" + _server.getHostname() + " 461 USER :Not enough parameters");
// 		return;
// 	}
// 	std::string username = args[1];
// 	std::string realname;
// 	if (!args[4].empty() && args[4][0] == ':')
// 		realname = args[4].substr(1);
// 	else
// 		realname = args[4];
// 	client.setUsername(username);
// 	client.setRealname(realname);
// 	client.sendWelcome(_server.getHostname());
// }

// static std::vector<std::string> splitByChar(const std::string& str, char delim)
// {
// 	std::vector<std::string> result;
// 	std::string token;
// 	std::istringstream ss(str);

// 	while (std::getline(ss, token, delim))
// 	{
// 		if (!token.empty())
// 			result.push_back(token);
// 	}
// 	return result;
// }

// static void joinSingleChannel(Client &client, const std::string &channel_name, const std::string &key, Server &server)
// {
// 	if (channel_name.empty()) return;

// 	if (channel_name[0] != '#' && channel_name[0] != '&')
// 	{
// 		client.sendMessage(":" + server.getHostname() + " 476 " + channel_name + " :Invalid channel name");
// 		return;
// 	}

// 	Channel* channel = server.getOrCreateChannel(channel_name);

// 	if (channel->hasMode(INVITE_ONLY) && !channel->isInvited(&client))
// 	{
// 		client.sendMessage(":" + server.getHostname() + " 473 " + channel_name + " :Cannot join channel (+i)");
// 		return;
// 	}

// 	if (channel->hasMode(KEY) && channel->getKey() != key)
// 	{
// 		client.sendMessage(":" + server.getHostname() + " 475 " + channel_name + " :Cannot join channel (+k)");
// 		return;
// 	}

// 	if (channel->hasMode(LIMIT) && channel->getClients().size() >= channel->getLimit())
// 	{
// 		client.sendMessage(":" + server.getHostname() + " 471 " + channel_name + " :Cannot join channel (+l)");
// 		return;
// 	}

// 	channel->addMember(&client);
// 	client.addToChannel(channel_name);
// 	channel->removeInvite(&client);

// 	if (channel->getClients().size() == 1)
// 		channel->addOperator(&client);

// 	std::string join_msg = ":" + client.getPrefix() + " JOIN :" + channel_name;
// 	server.notifyClients(channel, join_msg, &client);

// 	if (channel->getTopic().empty())
// 		client.sendMessage(":" + server.getHostname() + " 331 " + client.getNickname() + " " + channel_name + " :No topic is set");
// 	else
// 		client.sendMessage(":" + server.getHostname() + " 332 " + client.getNickname() + " " + channel_name + " :" + channel->getTopic());

// 	server.sendNamesList(client, channel);
// }

// void CommandHandler::join(Client& client, const std::vector<std::string>& args)
// {
// 	if (!checkRegistered(client))
// 		return;

// 	std::cout << "JOIN command received" << std::endl;

// 	if (args.size() < 2)
// 	{
// 		client.sendMessage(":" + _server.getHostname() + " 461 JOIN :Not enough parameters");
// 		return;
// 	}
// 	std::vector<std::string> channels = splitByChar(args[1], ',');
// 	std::vector<std::string> keys;
// 	if (args.size() >= 3)
// 		keys = splitByChar(args[2], ',');
// 	for (size_t i = 0; i < channels.size(); ++i)
// 	{
// 		std::string key = (i < keys.size()) ? keys[i] : "";
// 		joinSingleChannel(client, channels[i], key, _server);
// 	}
// }

// void CommandHandler::ping(Client &client, const std::vector<std::string> &args)
// {
// 	if (args.size() < 2)
// 	{
// 		client.sendMessage(":" + _server.getHostname() + " 409 " + client.getNickname() + " :No origin specified");
// 		return;
// 	}
// 	client.sendMessage("PONG :" + args[1]);
// }

// static void partSingleChannel(Client &client, const std::string &channel_name, Server &server)
// {
// 	Channel* channel = server.getChannel(channel_name);
// 	if (!channel)
// 	{
// 		client.sendMessage(":" + server.getHostname() + " 403 " + channel_name + " :No such channel");
// 		return;
// 	}

// 	if (!channel->isMember(&client))
// 	{
// 		client.sendMessage(":" + server.getHostname() + " 442 " + channel_name + " :You're not on that channel");
// 		return;
// 	}

// 	channel->removeMember(&client);
// 	if (channel->isOperator(&client))
// 		channel->removeOperator(&client);
// 	client.removeFromChannel(channel_name);

// 	std::string part_msg = ":" + client.getPrefix() + " PART :" + channel_name;
// 	server.notifyClients(channel, part_msg, &client);
// 	if (!channel->getClients().empty() && channel->getOperators().empty())
// 	{
// 		Client *new_op = *(channel->getClients().begin());
// 		channel->addOperator(new_op);
// 		std::string op_msg = ":" + new_op->getPrefix() + " MODE " + channel->getName() + " +o " + new_op->getNickname();
//     	server.notifyClients(channel, op_msg, nullptr);
// 	}
// 	if (channel->getClients().empty())
// 		server.deleteChannel(channel_name);
// }

// void CommandHandler::part(Client& client, const std::vector<std::string>& args)
// {
// 	if (!checkRegistered(client))
// 	return;
// 	std::cout << "PART command received" << std::endl;
// 	if (args.size() < 2)
// 	{
// 		client.sendMessage(":" + _server.getHostname() + " 461 PART :Not enough parameters");
// 		return;
// 	}
// 	std::vector<std::string> channels = splitByChar(args[1], ',');
// 	for (size_t i = 0; i < channels.size(); ++i)
// 		partSingleChannel(client, channels[i], _server);
// }

// void CommandHandler::mode(Client &client, const std::vector<std::string> &args)
// {
// 	// if (!checkRegistered(client))
// 	// 	return;
// 	// if (args.size() < 2)
// 	// {
// 	// 	client.sendMessage(":" + _server.getHostname() + " 461 MODE :Not enough parameters");
// 	// 	return;
// 	// }
// 	( void )client;
// 	( void )args;
// 	std::cout << "MODE command received" << std::endl;
// }

// void CommandHandler::privmsg(Client& client, const std::vector<std::string>& args)
// {
// 	(void)client;
// 	(void)args;
// 	std::cout << "PRIVMSG command received" << std::endl;
// }

// void CommandHandler::invite(Client& client, const std::vector<std::string>& args)
// {
// 	(void)client;
// 	(void)args;
// 	std::cout << "INVITE command received" << std::endl;
// }

// void CommandHandler::kick(Client& client, const std::vector<std::string>& args)
// {
// 	(void)client;
// 	(void)args;
// 	std::cout << "KICK command received" << std::endl;
// }


// void CommandHandler::topic(Client& client, const std::vector<std::string>& args)
// {
// 	(void)client;
// 	(void)args;
// 	std::cout << "TOPIC command received" << std::endl;
// }
