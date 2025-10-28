#pragma once
#include "main.hpp"
#include "Client.hpp"

enum ChannelMode
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

	// attributes
	const std::string _name;
	std::string _topic;
	std::string _key;
	size_t _limit;
	int _mode;
	std::set<Client*> _members;
	std::set<Client*> _operators;
	std::set<Client*> _invited;

	// disabled
	Channel();
	Channel(const Channel&);
	Channel& operator=(const Channel&);

public:

	// constructor, destructor
	Channel(const std::string& name);
	~Channel();

	// get, set, add, remove
	const std::string& getName() const;
	const std::string& getTopic() const;
	void setTopic(const std::string &topic);
	const std::string& getKey() const;
	void setKey(const std::string& key);
	size_t getLimit() const;
	void setLimit(size_t limit);
	void setMode(int flag);
	void unsetMode(int flag);
	bool hasMode(int flag) const;
	std::string getModesAsString() const;

	// get, add, remove <-> members
	const std::set<Client*>& getMembers() const;
	bool isMember(Client* client) const;
	void addMember(Client* client);
	void removeMember(Client* client);

	// get, add, remove <-> operators
	const std::set<Client*>& getOperators() const;
	bool isOperator(Client* client) const;
	void addOperator(Client* client);
	void removeOperator(Client* client);

	// <-> invited
	bool isInvited(Client* client) const;
	void invite(Client* client);
	void removeInvite(Client* client);
};
