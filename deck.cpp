#include "deck.hpp"

#include <unordered_map>
#include "banlist.hpp"

namespace YGOpen
{

Proto::DeckError Deck::CheckLimits(const Proto::DeckLimits& limits) const
{
	Proto::DeckError error;
#define F(deck, enumerator) \
	if(pbMsg.deck##_size() < limits.deck().min()) \
	{ \
		auto ool = error.mutable_out_of_limits(); \
		ool->set_deck_type(enumerator); \
		ool->set_limit_type(Proto::DeckError::OutOfLimits::LT_UNDER); \
		ool->set_expected(limits.deck().min()); \
		ool->set_got(pbMsg.deck##_size()); \
		return error; \
	} \
	if(pbMsg.main_size() > limits.deck().max()) \
	{ \
		auto ool = error.mutable_out_of_limits(); \
		ool->set_deck_type(enumerator); \
		ool->set_limit_type(Proto::DeckError::OutOfLimits::LT_OVER); \
		ool->set_expected(limits.deck().max()); \
		ool->set_got(pbMsg.deck##_size()); \
		return error; \
	}
	F(main, Proto::DeckError::OutOfLimits::DT_MAIN_DECK)
	F(extra, Proto::DeckError::OutOfLimits::DT_EXTRA_DECK)
	F(side, Proto::DeckError::OutOfLimits::DT_SIDE_DECK)
#undef F
	error.set_no_error(true);
	return error;
}

Proto::DeckError Deck::CheckBanlist(const Banlist& banlist) const
{
	Proto::DeckError error;
	
	// TODO
	
	error.set_no_error(true);
	return error;
}

} // namespace YGOpen
