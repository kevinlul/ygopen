#include "encoder_impl_begin.hpp"

CASE_FIRST(MSG_HINT)
#ifndef FILTERING
	auto hint = specific->mutable_information()->mutable_hint();
	
	const int type = wrapper->read<uint8_t>("hint type");
	hint->set_type(type);
	
	specific->set_player(wrapper->read<player_t>("player"));
	
	hint->set_data(wrapper->read<uint64_t>("hint data"));
	// TODO: handle each hint
#endif // FILTERING
CASE(MSG_MISSED_EFFECT)
#ifndef FILTERING
	auto miscAction = specific->mutable_information()->mutable_misc_action();
	miscAction->set_type(Core::Msg::MiscAction::ACTION_MISSED_TIMING);
	
	auto card = miscAction->mutable_card();
	
	ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, 1, card);
	ToCardCode(wrapper->read<cardcode_t>("card code 1"), card);
#endif // FILTERING
CASE_FINAL()
#ifdef FILTERING
default: { return false; break; }
#endif // FILTERING

#include "encoder_impl_end.hpp"
