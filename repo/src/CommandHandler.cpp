#include "CommandHandler.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"

CommandHandler::CommandHandler(Server& server) : _server(server)
{
	_commands["PING"] = &CommandHandler::ping;
	_commands["CAP"] = &CommandHandler::cap;
	_commands["PASS"] = &CommandHandler::pass;
	_commands["NICK"] = &CommandHandler::nick;
	_commands["USER"] = &CommandHandler::user;
	_commands["JOIN"] = &CommandHandler::join;
	_commands["PART"] = &CommandHandler::part;
	_commands["PRIVMSG"] = &CommandHandler::privmsg;
	_commands["KICK"] = &CommandHandler::kick;
	_commands["INVITE"] = &CommandHandler::invite;
	_commands["TOPIC"] = &CommandHandler::topic;
	_commands["MODE"] = &CommandHandler::mode;
	_commands["QUIT"] = &CommandHandler::quit;
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
	size_t i = 0;
	size_t len = line.size();

	while (i < len)
	{
		while (i < len && std::isspace(static_cast<unsigned char>(line[i])))
			i++;
		if (i >= len)
			break;

		if (line[i] == ':')
		{
			result.push_back(line.substr(i + 1));
			break;
		}

		size_t start = i;
		while (i < len && !std::isspace(static_cast<unsigned char>(line[i])))
			i++;
		result.push_back(line.substr(start, i - start));
	}
	// Le savais tu, irssi est le seul client a ajouter un : avant le nickname en cas de reponse a un privmsg
	// ce qui soit dit en passant est contre la RFC 1459, qui demande un format de message comme ceci :<command> [ <params> ] [ :<trailing> ]
	// ce morceau de code n'est la que pour accommoder ce client malfaisant.
	if (result.size() == 2 && (result[0] == "PRIVMSG") && result[1].find(' ') != std::string::npos)
	{
		size_t space = result[1].find(' ');
		std::string target = result[1].substr(0, space);
		std::string message = result[1].substr(space + 1);
		if (!target.empty() && target[0] == ':')
			target.erase(0, 1);
		if (!message.empty() && message[0] == ':')
			message.erase(0, 1);
		result[1] = target;
		result.push_back(message);
	}

	if (result.size() > 1 && result[1].size() > 1 && result[1][0] == ':')
		result[1].erase(0, 1);

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
			client.setSoftDisconnect("client (fd " + std::to_string(client.getFd()) + "): soft disconnect: line too long", "server: disconnected: line too long");
			return;
		}
		if (endLine > 0 && inbox[endLine - 1] == '\r')
			line = inbox.substr(startLine, endLine - startLine - 1);
		else
			line = inbox.substr(startLine, endLine - startLine);
		args = split(line);
		if (!args.empty())
		{
			command = toUpper(args[0]);
			std::map<std::string, CommandFunction>::iterator it = _commands.find(command);
			if (it != _commands.end())
			{
				std::cout << "received: <" << command << "> ";
				for (size_t i = 1; i < args.size(); ++i)
					std::cout << "<" << args[i] << "> ";
				std::cout << std::endl;
				(this->*(it->second))(client, args);
			}
			else
			{
				std::cout << "received (unknown): <" << command << "> ";
				for (size_t i = 1; i < args.size(); ++i)
					std::cout << "<" << args[i] << "> ";
				std::cout << std::endl;
				client.send(":" + _server.getHostname() + " 421 " + client.getNickname() + " " + command + " :Unknown command");
			}
		}
		startLine = endLine + 1;
		endLine = inbox.find("\n", startLine);
	}
	inbox.erase(0, startLine);
}

bool CommandHandler::checkRegistered(Client& client)
{
	if(!client.isRegistered())
	{
		client.send(":" + _server.getHostname() + " 451 " + client.getNickname() + " :You have not registered");
		return false;
	}
	return true;
}

void CommandHandler::cap(Client& client, const std::vector<std::string>& args)
{
	if (args.size() < 2)
	{
		client.send(":" + _server.getHostname() + " 461 CAP :Not enough parameters");
		return;
	}
	std::string subcommand = args[1];
	subcommand = toUpper(subcommand);
	if (subcommand == "LS")
		client.send(":" + _server.getHostname() + " CAP * LS :multi-prefix");
	else if (subcommand == "REQ")
	{
		std::string requested = (args.size() >= 3 ? args[2] : "");
		std::string nick = client.getNickname().empty() ? "*" : client.getNickname();
		if (requested.find("multi-prefix") != std::string::npos)
			client.send(":" + _server.getHostname() + " CAP " + nick + " ACK :" + requested);
		else
			client.send(":" + _server.getHostname() + " CAP " + nick + " NAK :" + requested);
	}
	else if (subcommand == "END")
		return;
	else
		client.send(":" + _server.getHostname() + " 410 " + client.getNickname() + " " + subcommand + " :Unknown CAP subcommand");
}

void CommandHandler::pass(Client& client, const std::vector<std::string>& args)
{
	if (client.isRegistered())
	{
		client.send(":" + _server.getHostname() + " 462 " + client.getNickname() + " :You may not reregister");
		return;
	}
	if (args.size() < 2)
	{
		client.send(":" + _server.getHostname() + " 461 PASS :Not enough parameters");
		return;
	}
	if (args[1] != _server.getPassword())
	{
		client.send(":" + _server.getHostname() + " 464 :Password incorrect");
		client.setSoftDisconnect();
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
	if (!client.isPasswordOk())
	{
		client.send(":" + _server.getHostname() + " 464 " + client.getNickname() + " :Password required");
		return;
	}

	if (args.size() < 2)
	{
		client.send(":" + _server.getHostname() + " 431 :No nickname given");
		return;
	}

	std::string new_nick = args[1];
	if (!isValidNickname(new_nick))
	{
		client.send(":" + _server.getHostname() + " 432 " + new_nick + " :Erroneous nickname");
		return;
	}

	if (_server.getClientByNick(new_nick) != NULL)
	{
		client.send(":" + _server.getHostname() + " 433 " + new_nick + " :Nickname is already in use");
		return;
	}

	std::string old_nick = client.getNickname();

	
	if (!old_nick.empty())
	{
		std::string prefix = ":" + old_nick;
		if (!client.getUsername().empty())
			prefix += "!" + client.getUsername() + "@" + client.getHostname();
		std::string msg = prefix + " NICK :" + new_nick;
		_server.notifyChannelMembers(client.getChannels(), msg, NULL);
		client.send(msg);
		_server.removeClientFromNickMap(old_nick);
		client.setNickname(new_nick);
		_server.addClientToNickMap(client);
	}
	else
	{
		client.setNickname(new_nick);
		_server.addClientToNickMap(client);
		client.tryRegisterClient();
	}
}

void CommandHandler::user(Client& client, const std::vector<std::string>& args)
{
	if (!client.isPasswordOk())
	{
		client.send(":" + _server.getHostname() + " 464 " + client.getNickname() + " :Password required");
		return;
	}
	if (client.isRegistered())
	{
		client.send(":" + _server.getHostname() + " 462 " + client.getNickname() + " :You may not reregister");
		return;
	}
	if(args.size() < 5)
	{
		client.send(":" + _server.getHostname() + " 461 USER :Not enough parameters");
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
	client.tryRegisterClient();
}

static std::vector<std::string> splitByChar(const std::string& str, char delim)
{
	std::vector<std::string> result;
	std::string token;
	std::istringstream ss(str);

	while (std::getline(ss, token, delim))
	{
		if (!token.empty())
		result.push_back(token);
	}
	return result;
}

void CommandHandler::partSingleChannel(Client& client, const std::string& channel_name, const std::string& reason)
{
	Channel* channel = _server.getChannel(channel_name);
	if (!channel)
	{
		client.send(":" + _server.getHostname() + " 403 " + channel_name + " :No such channel");
		return;
	}

	if (!channel->isMember(&client))
	{
		client.send(":" + _server.getHostname() + " 442 " + channel_name + " :You're not on that channel");
		return;
	}

	std::string part_msg = ":" + client.getPrefix() + " PART " + channel_name;
	if (!reason.empty())
		part_msg += " :" + reason;
	_server.notifyChannelMembers(channel, part_msg, NULL);

	channel->removeMember(&client);
	if (channel->isOperator(&client))
		channel->removeOperator(&client);
	client.removeFromChannel(channel_name);

	if (!channel->getMembers().empty() && channel->getOperators().empty())
	{
		Client *new_op = *(channel->getMembers().begin());
		channel->addOperator(new_op);
		std::string op_msg = ":" + new_op->getPrefix() + " MODE " + channel->getName() + " +o " + new_op->getNickname();
		_server.notifyChannelMembers(channel, op_msg, NULL);
	}
	if (channel->getMembers().empty())
		_server.deleteChannel(channel_name);
}

void CommandHandler::join(Client& client, const std::vector<std::string>& args)
{
	if (!checkRegistered(client))
	return;
	if (args.size() < 2 && args[1].empty())
	{
		std::set<std::string> channels = client.getChannels();
		if (channels.empty())
			return;
		else
		{
			for (std::set<std::string>::iterator it = channels.begin(); it != channels.end(); ++it)
				partSingleChannel(client, *it);
			return;
		}
	}
	if (args.size() < 2)
	{
		client.send(":" + _server.getHostname() + " 461 JOIN :Not enough parameters");
		return;
	}
	std::string channel_name = args[1];
	if (channel_name.empty())
		return;
	std::string key = "";
	if (args.size() >= 3)
		key = args[2];

	if (channel_name[0] != '#' && channel_name[0] != '&')
	{
		client.send(":" + _server.getHostname() + " 476 " + channel_name + " :Invalid channel name");
		return;
	}

	Channel* channel = _server.getOrCreateChannel(channel_name);

	if (channel->hasMode(INVITE_ONLY) && !channel->isInvited(&client))
	{
		client.send(":" + _server.getHostname() + " 473 " + channel_name + " :Cannot join channel (+i)");
		return;
	}

	if (channel->hasMode(KEY) && channel->getKey() != key)
	{
		client.send(":" + _server.getHostname() + " 475 " + channel_name + " :Cannot join channel (+k)");
		return;
	}
	if (channel->hasMode(LIMIT) && channel->getMembers().size() >= channel->getLimit())
	{
		client.send(":" + _server.getHostname() + " 471 " + channel_name + " :Cannot join channel (+l)");
		return;
	}

	channel->addMember(&client);
	client.addToChannel(channel_name);
	channel->removeInvite(&client);
	if (channel->getMembers().size() == 1)
		channel->addOperator(&client);

	std::string join_msg = ":" + client.getPrefix() + " JOIN :" + channel_name;
	_server.notifyChannelMembers(channel, join_msg, &client);

	if (channel->getTopic().empty())
		client.send(":" + _server.getHostname() + " 331 " + client.getNickname() + " " + channel_name + " :No topic is set");
	else
		client.send(":" + _server.getHostname() + " 332 " + client.getNickname() + " " + channel_name + " :" + channel->getTopic());

	_server.sendNamesList(client, channel);
}

void CommandHandler::ping(Client& client, const std::vector<std::string>& args)
{
	if (args.size() < 2)
	{
		client.send(":" + _server.getHostname() + " 409 " + client.getNickname() + " :No origin specified");
		return;
	}
	client.send("PONG :" + args[1]);
}

void CommandHandler::part(Client& client, const std::vector<std::string>& args)
{
	if (!checkRegistered(client))
	return;
	if (args.size() < 2)
	{
		client.send(":" + _server.getHostname() + " 461 PART :Not enough parameters");
		return;
	}
	std::vector<std::string> channels = splitByChar(args[1], ',');
	std::string reason;
	if (args.size() > 3)
		reason = args[3];

	for (size_t i = 0; i < channels.size(); ++i)
		partSingleChannel(client, channels[i], reason);
}

bool static checkKey(const std::string& key)
{
	if (key.empty() || key.size() > 32)
		return false;
	for (size_t i = 0; i < key.size(); i++)
	{
		char c = key[i];
		if (c == ' ' || c == ',' || c == ':')
			return false;
	}
	return true;
}

void CommandHandler::mode(Client& client, const std::vector<std::string>& args)
{
	if (!checkRegistered(client))
		return;

	if (args.size() < 2)
	{
		client.send(":" + _server.getHostname() + " 461 MODE :Not enough parameters");
		return;
	}

	const std::string& target = args[1];

	if (target.empty() || (target[0] != '#' && target[0] != '&'))
		return;

	Channel* channel = _server.getChannel(args[1]);

	if (!channel)
	{
		client.send(":" + _server.getHostname() + " 403 " + args[1] + " :No such channel");
		return;
	}

	if (!channel->isMember(&client))
	{
		client.send(":" + _server.getHostname() + " 442 " + args[1] + " :You're not on that channel");
		return;
	}

	if (args.size() == 2)
	{
		std::string modes = channel->getModesAsString();
		client.send(":" + _server.getHostname() + " 324 " + client.getNickname() + " " + channel->getName() + " :" + modes);
		return;
	}

	if (!channel->isOperator(&client))
	{
		client.send(":" + _server.getHostname() + " 482 " + args[1] + " :You're not channel operator");
		return;
	}

	std::string mode_changes = args[2];
	size_t index = 0;
	bool adding = true;
	std::vector<std::string> mode_params(args.begin() + 3, args.end());
	std::string mode_signs;
	std::string mode_args;
	char current_sign = '\0';
	for (size_t i = 0; i < mode_changes.size(); ++i)
	{
		char c = mode_changes[i];
		if (c == '+' || c == '-')
		{
			if (!mode_signs.empty())
        	{
            std::string msg = ":" + client.getPrefix() + " MODE " + channel->getName() + " " + current_sign + mode_signs + mode_args;
            _server.notifyChannelMembers(channel, msg, NULL);
            mode_signs.clear();
            mode_args.clear();
         	}
    		current_sign = c;
			adding = (c == '+');
        	continue;
		}
		switch (c)
		{
		case 'i':
			if (adding)
			{
				if (channel->hasMode(INVITE_ONLY))
				{
					client.send(":" + _server.getHostname() + " 467 " + channel->getName() + " :Channel is already invite-only");
					return;
				}
				channel->setMode(INVITE_ONLY);
			}
			else
				channel->unsetMode(INVITE_ONLY);
			mode_signs += 'i';
			break;
		case 't':
			if (adding)
			{
				if (channel->hasMode(PROTECTED_TOPIC))
				{
					client.send(":" + _server.getHostname() + " 467 " + channel->getName() + " :Channel topic is already protected");
					return;
				}
				channel->setMode(PROTECTED_TOPIC);
			}
			else
				channel->unsetMode(PROTECTED_TOPIC);
			mode_signs += 't';
			break;
		case 'k':
			if (adding)
			{
				if (index >= mode_params.size())
				{
					client.send(":" + _server.getHostname() + " 461 MODE :Not enough parameters");
					return;
				}
				if (!checkKey(mode_params[index]))
				{
					client.send(":" + _server.getHostname() + " 501 MODE :Invalid key parameter");
					return;
				}
				channel->setMode(KEY);
				channel->setKey(mode_params[index]);
				mode_args += " " + mode_params[index];
				index++;
			}
			else
			{
				channel->unsetMode(KEY);
				channel->setKey("");
			}
			mode_signs += 'k';
			break;
		case 'l':
			if (adding)
			{
				if (index >= mode_params.size())
				{
					client.send(":" + _server.getHostname() + " 461 MODE :Not enough parameters");
					return;
				}
				long limit;
				try
				{
					limit = std::stol(mode_params[index].c_str(), NULL, 10);
				}
				catch(...)
				{
					client.send(":" + _server.getHostname() + " 501 MODE :Invalid limit parameter");
					return;
				}

				if (limit <= 0 || limit > INT_MAX)
				{
					client.send(":" + _server.getHostname() + " 501 MODE :Invalid limit parameter");
					return;
				}
				channel->setMode(LIMIT);
				channel->setLimit(limit);
				mode_args += " " + mode_params[index];
				index++;
			}
			else
			{
				channel->unsetMode(LIMIT);
				channel->setLimit(0);
			}
			mode_signs += 'l';
			break;
		case 'o':
			if (index >= mode_params.size())
			{
				client.send(":" + _server.getHostname() + " 461 MODE :Not enough parameters");
				return;
			}
			{
				std::string target_nick = mode_params[index++];
				Client* target_client = _server.getClientByNick(target_nick);
				if (!target_client || !channel->isMember(target_client))
				{
					client.send(":" + _server.getHostname() + " 441 " + target_nick + " " + channel->getName() + " :They aren't on that channel");
					return;
				}
				if (adding)
				{
					if (channel->isOperator(target_client))
					{
						client.send(":" + _server.getHostname() + " 467 " + target_nick + " :is already a channel operator");
						return;
					}
					channel->addOperator(target_client);
				}
				else
					channel->removeOperator(target_client);
			}
			mode_signs += 'o';
			break;
		default:
			client.send(":" + _server.getHostname() + " 472 " + c + " :is unknown mode char to me");
			break;
		}
	}
	if (!mode_signs.empty())
	{
		std::string mode_msg = ":" + client.getPrefix() + " MODE " + channel->getName() + " " + current_sign + mode_signs + mode_args;
		_server.notifyChannelMembers(channel, mode_msg, NULL);
	}
}

void CommandHandler::privmsg(Client& client, const std::vector<std::string>& args)
{
	if (!checkRegistered(client))
		return;
	if (args.size() < 3)
	{
		client.send(":" + _server.getHostname() + " 461 PRIVMSG :Not enough parameters");
		return;
	}
	std::string target = args[1];
	std::string message = args[2];
	if (!target.empty() && ( target[0] == '#' || target[0] == '&'))
	{
		Channel* channel = _server.getChannel(target);
		if (!channel)
		{
			client.send(":" + _server.getHostname() + " 403 " + target + " :No such channel");
			return;
		}

		if (!channel->isMember(&client))
		{
			client.send(":" + _server.getHostname() + " 404 " + target + " :Cannot send to channel");
			return;
		}
		std::string msg = ":" + client.getPrefix() + " PRIVMSG " + target + " :" + message;
		_server.notifyChannelMembers(channel, msg, &client);
	}
	else
	{
		Client* target_client = _server.getClientByNick(target);
		if (!target_client)
		{
			client.send(":" + _server.getHostname() + " 401 " + target + " :No such nick");
			return;
		}
		std::string msg = ":" + client.getPrefix() + " PRIVMSG " + target + " :" + message;
		target_client->send(msg);
	}
}

void CommandHandler::invite(Client& client, const std::vector<std::string>& args)
{
	if (!checkRegistered(client))
		return;
	if (args.size() < 3)
	{
		client.send(":" + _server.getHostname() + " 461 INVITE :Not enough parameters");
		return;
	}
	std::string target_nick = args[1];
	std::string channel_name = args[2];
	Channel* channel = _server.getChannel(channel_name);
	if (!channel)
	{
		client.send(":" + _server.getHostname() + " 403 " + channel_name + " :No such channel");
		return;
	}
	if (!channel->isMember(&client))
	{
		client.send(":" + _server.getHostname() + " 442 " + channel_name + " :You're not on that channel");
		return;
	}
	if (channel->hasMode(INVITE_ONLY) && !channel->isOperator(&client))
	{
		client.send(":" + _server.getHostname() + " 482 " + channel_name + " :You're not channel operator");
		return;
	}
	Client* target_client = _server.getClientByNick(target_nick);
	if (!target_client)
	{
		client.send(":" + _server.getHostname() + " 401 " + target_nick + " :No such nick");
		return;
	}
	if (channel->isMember(target_client))
	{
		client.send(":" + _server.getHostname() + " 443 " + target_nick + " " + channel_name + " :is already on that channel");
		return;
	}
	channel->invite(target_client);
	client.send(":" + _server.getHostname() + " 341 " + client.getNickname() + " " + target_nick + " " + channel_name);
	target_client->send(":" + client.getPrefix() + " INVITE " + target_nick + " :" + channel_name);
}

void CommandHandler::kick(Client& client, const std::vector<std::string>& args)
{
	if (!checkRegistered(client))
		return;
	if (args.size() < 3)
	{
		client.send(":" + _server.getHostname() + " 461 KICK :Not enough parameters");
		return;
	}
	std::string channel_name = args[1];
	std::string target_nick = args[2];
	std::string reason;
	if (args.size() > 3)
		reason = args[3];

	Channel* channel = _server.getChannel(channel_name);
	if (!channel)
	{
		client.send(":" + _server.getHostname() + " 403 " + channel_name + " :No such channel");
		return;
	}
	if (!channel->isMember(&client))
	{
		client.send(":" + _server.getHostname() + " 442 " + channel_name + " :You're not on that channel");
		return;
	}
	if (!channel->isOperator(&client))
	{
		client.send(":" + _server.getHostname() + " 482 " + channel_name + " :You're not channel operator");
		return;
	}
	Client* target_client = _server.getClientByNick(target_nick);
	if (!target_client || !channel->isMember(target_client))
	{
		client.send(":" + _server.getHostname() + " 441 " + target_nick + " " + channel_name + " :They aren't on that channel");
		return;
	}

	std::string kick_msg = ":" + client.getPrefix() + " KICK " + channel_name + " " + target_nick;
	if (!reason.empty())
		kick_msg += " :" + reason;
	_server.notifyChannelMembers(channel, kick_msg, NULL);

	channel->removeMember(target_client);
	if (channel->isOperator(target_client))
		channel->removeOperator(target_client);
	target_client->removeFromChannel(channel_name);
}


void CommandHandler::topic(Client& client, const std::vector<std::string>& args)
{
	if(!checkRegistered(client))
		return;
	if (args.size() < 2)
	{
		client.send(":" + _server.getHostname() + " 461 TOPIC :Not enough parameters");
		return;
	}
	std::string channel_name = args[1];
	Channel* channel = _server.getChannel(channel_name);
	if (!channel)
	{
		client.send(":" + _server.getHostname() + " 403 " + channel_name + " :No such channel");
		return;
	}
	if (!channel->isMember(&client))
	{
		client.send(":" + _server.getHostname() + " 442 " + channel_name + " :You're not on that channel");
		return;
	}
	if (args.size() == 2)
	{
		if (channel->getTopic().empty())
			client.send(":" + _server.getHostname() + " 331 " + client.getNickname() + " " + channel_name + " :No topic is set");
		else
			client.send(":" + _server.getHostname() + " 332 " + client.getNickname() + " " + channel_name + " :" + channel->getTopic());
		return;
	}
	if (channel->hasMode(PROTECTED_TOPIC) && !channel->isOperator(&client))
	{
		client.send(":" + _server.getHostname() + " 482 " + channel_name + " :You're not channel operator");
		return;
	}
	std::string new_topic = args[2];
	channel->setTopic(new_topic);
	std::string msg = ":" + client.getPrefix() + " TOPIC " + channel_name + " :" + new_topic;
	_server.notifyChannelMembers(channel, msg, NULL);
}

void CommandHandler::quit(Client& client, const std::vector<std::string>& args)
{
	std::string quit_msg = "leaving";
	if (args.size() >= 2)
		quit_msg = args[1];
	std::string full_quit_msg = ":" + client.getPrefix() + " QUIT :" + quit_msg;
	_server.notifyChannelMembers(client.getChannels(), full_quit_msg, &client);
	client.setSoftDisconnect("client (fd " + std::to_string(client.getFd()) + ", i " + std::to_string(client.getI()) + "): soft disconnect: QUIT command received", "disconnected");
}
