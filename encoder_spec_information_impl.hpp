#include "encoder_impl_begin.hpp"

CASE_FIRST(MSG_HINT)
#ifndef FILTERING
	auto hint = specific->mutable_information()->mutable_hint();
	
	const int type = w->read<s_count_t>("hint type");
	hint->set_type(type);
	
	specific->set_player(w->read<player_t>("player"));
	
	hint->set_data(w->read<uint64_t>("hint data"));
	// TODO: handle each hint
#endif // FILTERING
CASE(MSG_MISSED_EFFECT)
#ifndef FILTERING
	auto miscAction = specific->mutable_information()->mutable_misc_action();
	miscAction->set_type(Core::Msg::MiscAction::ACTION_MISSED_TIMING);
	
	auto card = miscAction->mutable_card();
	
	ReadCardLocInfo<player_t, s_loc_t, seq_t, pos_t>(w, card);
	ToCardCode(w->read<code_t>("card code"), card);
#endif // FILTERING
CASE_FINAL()
#ifdef FILTERING
default: { return false; break; }
#endif // FILTERING

#include "encoder_impl_end.hpp"
