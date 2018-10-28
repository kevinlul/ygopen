#ifndef __BANLIST_HPP__
#define __BANLIST_HPP__
#include <set>
#include <string>

#include <nlohmann/json.hpp>

namespace YGOpen
{

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

	bool FromJSON(nlohmann::json& j);
};

} // namespace YGOpen

#endif // __BANLIST_HPP__
