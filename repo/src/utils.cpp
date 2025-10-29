#include "main.hpp"

void perrorAndThrow(const char* message)
{
	if (message)
		throw std::runtime_error(std::string(message) + ": " + std::strerror(errno));
	else
		throw std::runtime_error(std::strerror(errno));
}

void errorAndThrow(const char* message)
{
	if (message)
		throw std::runtime_error(message);
	else
		throw std::runtime_error("error");
}

static bool has_white_spaces(const std::string& str)
{
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (std::isspace(static_cast<unsigned char>(str[i])))
			return true;
	}
	return false;
}

bool isValidPassword(const std::string& password)
{
	return password.length() > 0 && password.length() <= 32 && !has_white_spaces(password);
}
