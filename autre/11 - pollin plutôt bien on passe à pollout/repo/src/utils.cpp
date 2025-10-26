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
