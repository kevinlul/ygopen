#include "banlist.hpp"

namespace YGOpen
{

const std::string& Banlist::GetId() const
{
	return pbMsg.id();
}

bool Banlist::IsWhitelist() const
{
	return pbMsg.whitelist_size() > 0;
}

const std::string& Banlist::GetTitle() const
{
	return pbMsg.title();
}

const std::string& Banlist::GetNotes() const
{
	return pbMsg.notes();
}

} // namespace YGOpen
