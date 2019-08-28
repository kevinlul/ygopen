#include "encoder_impl_begin.hpp"

CASE(MSG_HINT,
	auto hint = specific->mutable_information()->mutable_hint();
	const int type = w->read<uint8_t>("hint type");
	hint->set_type(type);
	specific->set_player(w->read<player_t>("player"));
	hint->set_data(w->read<uint64_t>("hint data"));
	// TODO: handle each hint
);

CASE(MSG_MISSED_EFFECT,
	auto miscAction = specific->mutable_information()->mutable_misc_action();
	miscAction->set_type(Core::Msg::MiscAction::ACTION_MISSED_TIMING);
	auto card = miscAction->mutable_card();
	READ_INFO_LOC_CORE(w, card);
	ToCardCode(w->read<code_t>("card code"), card);
);

#ifdef FILTERING
default: { return false; break; }
#endif // FILTERING

#include "encoder_impl_end.hpp"
