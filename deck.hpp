#ifndef DECK_HPP
#define DECK_HPP
#include "deck.pb.h"
#include "serializable.hpp"

namespace YGOpen
{

// class CardDatabase;
class Banlist;

class Deck : public Serializable<Deck, Proto::Deck>
{
public:
// 	Proto::DeckError CheckCards(const CardDatabase& dbm) const;
	Proto::DeckError CheckLimits(const Proto::DeckLimits& limits) const;
	Proto::DeckError CheckBanlist(const Banlist& banlist) const;
};

} // namespace YGOpen

#endif // DECK_HPP
