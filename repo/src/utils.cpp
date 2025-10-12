#include "main.hpp"

void perror_and_throw(const char* message)
{
	if (message)
		throw std::runtime_error(std::string(message) + ": " + std::strerror(errno));
	else
		throw std::runtime_error(std::strerror(errno));
}

void error_and_throw(const char* message)
{
	if (message)
		throw std::runtime_error(message);
	else
		throw std::runtime_error("error");
}
