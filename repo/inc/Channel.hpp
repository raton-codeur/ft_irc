#pragma once
#include "main.hpp"
#include "Client.hpp"

enum e_mode
{
	NONE = 0,
	INVITE_ONLY = 1,
	PROTECTED_TOPIC = 1 << 1,
	KEY = 1 << 2,
	LIMIT = 1 << 3
};

class Channel
{
private:

	std::string _name;
	std::string _topic;
	std::string _key;
	size_t _limit;
	int _mode;
	std::set<Client*> _members;
	std::set<Client*> _operators;
	std::set<Client*> _invited;

	Channel(const Channel&);
	Channel& operator=(const Channel&);

public:

	Channel(const std::string& name);
	~Channel();

	const std::string& getName() const;
	void setMode(int flag);
	void unsetMode(int flag);
	bool hasMode(int flag) const;
	void setTopic(const std::string &topic);
	const std::string& getTopic() const;
	void addMember(Client* client);
	void removeMember(Client* client);
	bool isMember(Client* client) const;
	void addOperator(Client* client);
	void removeOperator(Client* client);
	bool isOperator(Client* client) const;
	void invite(Client* client);
	void removeInvite(Client* client);
	bool isInvited(Client* client) const;
};
