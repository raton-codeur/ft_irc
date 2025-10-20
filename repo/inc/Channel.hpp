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
	void  setName(const std::string& name);

	const std::string& getKey() const;
	void setKey(const std::string& key);

	const size_t& getLimit() const;
	void setLimit(const size_t& limit);

	void setMode(int flag);
	void unsetMode(int flag);
	bool hasMode(int flag) const;

	const std::string& getTopic() const;
	void setTopic(const std::string &topic);
	
	void addMember(Client* client);
	void removeMember(Client* client);
	bool isMember(Client* client) const;

	void addOperator(Client* client);
	void removeOperator(Client* client);
	bool isOperator(Client* client) const;
	std::set<Client*> getOperators() const;

	void invite(Client* client);
	void removeInvite(Client* client);
	bool isInvited(Client* client) const;

	const std::set<Client*>& getClients() const;
};
