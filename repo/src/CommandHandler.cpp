#include "CommandHandler.hpp"
#include "main.hpp"

std::vector<std::string> _IRCsplit(const std::string &str)
{
	std::vector<std::string> result;
	std::istringstream iss(str);
	std::string word;

	while (iss >> word)
	{
		if (word[0] == ':') // tout ce qui suit devient un seul argument
		{
			std::string rest;
			std::getline(iss, rest);           // récupère le reste de la ligne
			result.push_back(word.substr(1) + rest); // on enlève les ":" et concatène
			break;                             // on a tout récupéré
		}
		result.push_back(word);                 // sinon, juste l'ajouter au vecteur
	}
	return result;
}

CommandHandler::CommandHandler(Server &server) : _server(server)
{
	_commands["PASS"] = &CommandHandler::PASS;
	_commands["NICK"] = &CommandHandler::NICK;
	_commands["USER"] = &CommandHandler::USER;
	_commands["JOIN"] = &CommandHandler::JOIN;
	_commands["PRIVMSG"] = &CommandHandler::PRIVMSG;
}

CommandHandler::~CommandHandler()
{
}
