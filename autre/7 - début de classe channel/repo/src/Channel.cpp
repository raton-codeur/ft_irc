#include "Channel.hpp"

Channel::Channel(const std::string& name) : _name(name), _mode(0)
{}

Channel::~Channel()
{}

const std::string& Channel::getName() const
{
	return _name;
}
