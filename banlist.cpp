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

bool Banlist::FromJSON(std::string& str)
{
	nlohmann::json j;
	try
	{
		j = nlohmann::json::parse(str);
	}
	catch(std::exception& e)
	{
		std::printf("Exception ocurred when parsing JSON file: %s\n", e.what());
		return false;
	}

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
