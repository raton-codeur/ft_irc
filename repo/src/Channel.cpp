#include "Channel.hpp"

Channel::Channel(const std::string& name) : _name(name), _mode(0)
{}

Channel::~Channel()
{}

const std::string& Channel::getName() const
{
	return _name;
}

const std::string& Channel::getTopic() const
{
	return (_topic);
}

void Channel::setTopic(const std::string &topic)
{
	_topic = topic;
}

const std::string &Channel::getKey() const
{
	return _key;
}

void Channel::setKey(const std::string &key)
{
	_key = key;
}

size_t Channel::getLimit() const
{
	return _limit;
}

void Channel::setLimit(size_t limit)
{
	_limit = limit;
}

void Channel::setMode(int flag)
{
	_mode |= flag;
}

void Channel::unsetMode(int flag)
{
	_mode &= ~flag;
}

bool Channel::hasMode(int flag) const
{
	return (_mode & flag);
}

const std::set<Client *> &Channel::getMembers() const
{
	return _members;
}

bool Channel::isMember(Client *client) const
{
	return (_members.find(client) != _members.end());
}

void Channel::addMember(Client *client)
{
	_members.insert(client);
}

void Channel::removeMember(Client *client)
{
	_members.erase(client);
}

const std::set<Client*>& Channel::getOperators() const
{
	return _operators;
}

bool Channel::isOperator(Client *client) const
{
	return (_operators.find(client) != _operators.end());
}

void Channel::addOperator(Client *client)
{
	_operators.insert(client);
}

void Channel::removeOperator(Client *client)
{
	_operators.erase(client);
}

bool Channel::isInvited(Client *client) const
{
	return (_invited.find(client) != _invited.end());
}

void Channel::invite(Client *client)
{
	_invited.insert(client);
}

void Channel::removeInvite(Client *client)
{
	_invited.erase(client);
}
