#ifndef BANLIST_HPP
#define BANLIST_HPP
#include "banlist.pb.h"
#include "serializable.hpp"

namespace YGOpen
{

class Deck;

class Banlist : public Serializable<Banlist, Proto::Banlist>
{
public:
	const std::string& GetId() const;
	bool IsWhitelist() const;
	const std::string& GetTitle() const;
	const std::string& GetNotes() const;
private:
	friend Deck;
};

} // namespace YGOpen

#endif // BANLIST_HPP
