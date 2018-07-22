#ifndef __BANLIST_HPP__
#define __BANLIST_HPP__
#include <set>
#include <string>

class Banlist
{
	int mode;
public:
	enum {MODE_BLACKLIST, MODE_WHITELIST};

	Banlist();

	std::set<int> semilimited;
	std::set<int> limited;
	std::set<int> forbidden;

	std::set<int> whitelist;

	int GetMode() const;

	bool FromJSON(std::string& str);
};

#endif // __BANLIST_HPP__
