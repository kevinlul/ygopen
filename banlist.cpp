#include "banlist.hpp"

#include <cstdio>
#include <exception>
#include <nlohmann/json.hpp>

namespace YGOpen
{

Banlist::Banlist() : mode(MODE_BLACKLIST)
{
	
}

int Banlist::GetMode() const
{
	return mode;
}

bool Banlist::FromJSON(nlohmann::json& j)
{
	std::vector<int> v;

	v = j.at("semilimited").get<std::vector<int>>();
	semilimited = std::set<int>(v.begin(), v.end());

	v = j.at("limited").get<std::vector<int>>();
	limited = std::set<int>(v.begin(), v.end());

	v = j.at("forbidden").get<std::vector<int>>();
	forbidden = std::set<int>(v.begin(), v.end());

	std::printf("Loaded banlist %s\n", j["name"].get<std::string>().c_str());

	return true;
}

} // namespace YGOpen
