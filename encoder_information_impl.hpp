#include "encoder_impl_begin.hpp"

CASE_FIRST(MSG_WIN)
#ifndef FILTERING
	auto win = information->mutable_win();
	
	win->set_player(w->read<player_t>("player"));
	win->set_reason(w->read<uint8_t>("reason"));
#endif // FILTERING
CASE(MSG_CONFIRM_DECKTOP)
#ifndef FILTERING
	auto confirmCards = information->mutable_confirm_cards();
	
	confirmCards->set_type(Core::Msg::ConfirmCards::CONFIRM_DECKTOP);
	
	w->seek(1, Buffer::seek_dir::cur, "player");
	
	CardSpawner Add1 = BIND_FUNC_TO_OBJ_PTR(confirmCards, add_cards);
	ReadCardVector<count_t, s_loc_t, s_seq_t>(w, Add1);
#endif // FILTERING
CASE(MSG_CONFIRM_CARDS)
#ifndef FILTERING
	auto confirmCards = information->mutable_confirm_cards();
	
	confirmCards->set_type(Core::Msg::ConfirmCards::CONFIRM_CARDS);
	
	w->seek(1, Buffer::seek_dir::cur, "player");
	
	CardSpawner Add1 = BIND_FUNC_TO_OBJ_PTR(confirmCards, add_cards);
	ReadCardVector<count_t, s_loc_t, seq_t>(w, Add1);
#endif // FILTERING
CASE(MSG_SHUFFLE_DECK)
#ifndef FILTERING
	auto shuffleLoc = information->mutable_shuffle_location();
	
	shuffleLoc->set_player(w->read<player_t>("player"));
	
	shuffleLoc->set_location(0x01); // LOCATION_DECK
	
	// NOTE: empty decklist
#endif // FILTERING
CASE(MSG_SHUFFLE_HAND)
#ifndef FILTERING
	auto shuffleLoc = information->mutable_shuffle_location();
	
	shuffleLoc->set_player(w->read<player_t>("player"));
	
	shuffleLoc->set_location(0x02); // LOCATION_HAND
	
	CardSpawner Add1 = BIND_FUNC_TO_OBJ_PTR(shuffleLoc, add_shuffled_cards);
	auto count = w->read<count_t>(".size()");
	for(decltype(count) i = 0; i < count; i++)
	{
		Core::Data::CardInfo* card = Add1();
		ToCardCode(w->read<code_t>("card code ", (int)i), card);
		card->set_location(0x02); // LOCATION_HAND
	}
#endif // FILTERING
CASE(MSG_SWAP_GRAVE_DECK)
#ifndef FILTERING
	auto miscAction = information->mutable_misc_action();
	
	miscAction->set_type(Core::Msg::MiscAction::ACTION_SWAP_GY_WITH_DECK);
	
	miscAction->set_player(w->read<player_t>("player"));
#endif // FILTERING
CASE(MSG_SHUFFLE_SET_CARD)
#ifndef FILTERING
	auto shuffleSet = information->mutable_shuffle_set_cards();
	w->seek(1, Buffer::seek_dir::cur, "location");
	
	CardSpawner Add1 = BIND_FUNC_TO_OBJ_PTR(shuffleSet, add_cards_previous);
	CardSpawner Add2 = BIND_FUNC_TO_OBJ_PTR(shuffleSet, add_cards_current);
	auto count = w->read<s_count_t>(".size()");
	for(decltype(count) i = 0; i < count; i++)
		READ_INFO_LOC_CORE(w, Add1());
	for(decltype(count) i = 0; i < count; i++)
		READ_INFO_LOC_CORE(w, Add2());
#endif // FILTERING
CASE(MSG_REVERSE_DECK)
#ifndef FILTERING
	auto miscAction = information->mutable_misc_action();
	
	miscAction->set_type(Core::Msg::MiscAction::ACTION_REVERSE_DECK);
#endif // FILTERING
CASE(MSG_DECK_TOP)
#ifndef FILTERING
	auto updateCard = information->mutable_update_card();
	
	updateCard->set_reason(Core::Msg::UpdateCard::REASON_DECK_TOP);
	
	auto previous = updateCard->mutable_previous();
	previous->set_controller(w->read<player_t>("player"));
	previous->set_sequence(w->read<s_seq_t>("sequence"));
	previous->set_location(0x01); // LOCATION_DECK
	
	auto current = updateCard->mutable_current();
	ToCardCode(w->read<code_t>("card code"), current);
	// NOTE: maybe move the dirty bit to the position parameter?
#endif // FILTERING
CASE(MSG_SHUFFLE_EXTRA)
#ifndef FILTERING
	auto shuffleLoc = information->mutable_shuffle_location();
	
	shuffleLoc->set_player(w->read<player_t>("player"));
	
	shuffleLoc->set_location(0x40); // LOCATION_EXTRA
	
	CardSpawner Add1 = BIND_FUNC_TO_OBJ_PTR(shuffleLoc, add_shuffled_cards);
	auto count = w->read<count_t>(".size()");
	for(decltype(count) i = 0; i < count; i++)
	{
		Core::Data::CardInfo* card = Add1();
		ToCardCode(w->read<code_t>("card code ", (int)i), card);
		card->set_location(0x40); // LOCATION_EXTRA
	}
#endif // FILTERING
CASE(MSG_NEW_TURN)
#ifndef FILTERING
	auto newTurn = information->mutable_new_turn();
	
	newTurn->set_turn_player(w->read<player_t>("turn player"));
#endif // FILTERING
CASE(MSG_NEW_PHASE)
#ifndef FILTERING
	auto newPhase = information->mutable_new_phase();
	
	newPhase->set_phase(w->read<uint16_t>("phase"));
#endif // FILTERING
CASE(MSG_CONFIRM_EXTRATOP)
#ifndef FILTERING
	auto confirmCards = information->mutable_confirm_cards();
	
	confirmCards->set_type(Core::Msg::ConfirmCards::CONFIRM_EXTRATOP);
	
	w->seek(1, Buffer::seek_dir::cur, "player");
	
	CardSpawner Add1 = BIND_FUNC_TO_OBJ_PTR(confirmCards, add_cards);
	ReadCardVector<count_t, s_loc_t, s_seq_t>(w, Add1);
#endif // FILTERING
CASE(MSG_MOVE)
#ifndef FILTERING
	auto updateCard = information->mutable_update_card();
	
	updateCard->set_reason(Core::Msg::UpdateCard::REASON_MOVE);
	
	const auto cardCode = w->read<code_t>("card code");
	
	auto ReadCardInfo = [&w, cardCode](Core::Data::CardInfo* card)
	{
		ToCardCode(cardCode, card);
		READ_INFO_LOC_CORE(w, card);
	};
	
	ReadCardInfo(updateCard->mutable_previous());
	ReadCardInfo(updateCard->mutable_current());
	
	updateCard->set_core_reason(w->read<uint32_t>("core reason"));
#endif // FILTERING
CASE(MSG_POS_CHANGE)
#ifndef FILTERING
	auto updateCard = information->mutable_update_card();
	
	updateCard->set_reason(Core::Msg::UpdateCard::REASON_POS_CHANGE);
	
	auto previous = updateCard->mutable_previous();
	auto current = updateCard->mutable_current();
	
	ToCardCode(w->read<code_t>("card code"), current);
	
	previous->set_controller(w->read<player_t>("controller"));
	previous->set_location(w->read<s_loc_t>("location"));
	previous->set_sequence(w->read<s_seq_t>("sequence"));
	previous->set_position(w->read<s_pos_t>("position"));
	current->set_position(w->read<s_pos_t>("current position"));
#endif // FILTERING
CASE(MSG_SET)
#ifndef FILTERING
	auto updateCard = information->mutable_update_card();
	
	updateCard->set_reason(Core::Msg::UpdateCard::REASON_SET);
	
	auto previous = updateCard->mutable_previous();
	auto current = updateCard->mutable_current();
	
	ToCardCode(w->read<code_t>("card code"), current);
	
	previous->set_controller(w->read<player_t>("controller"));
	previous->set_location(w->read<s_loc_t>("location"));
	previous->set_sequence(w->read<seq_t>("sequence"));
	current->set_position(w->read<pos_t>("current position"));
#endif // FILTERING
CASE(MSG_SWAP)
#ifndef FILTERING
	auto swapCards = information->mutable_swap_cards();
	
	auto ReadCardInfo = [&w](Core::Data::CardInfo* card)
	{
		ToCardCode(w->read<code_t>("card code"), card);
		READ_INFO_LOC_CORE(w, card);
	};
	
	ReadCardInfo(swapCards->mutable_card1());
	ReadCardInfo(swapCards->mutable_card2());
#endif // FILTERING
CASE(MSG_FIELD_DISABLED)
#ifndef FILTERING
	auto disableZones = information->mutable_disable_zones();
	
	std::bitset<32> flag(w->read<uint32_t>("flag"));

	// NOTE: different from MSG_SELECT_PLACE.
	// The flag checkings are reversed.
	auto ExtractPlaces = [&disableZones, &flag](int player, int indexStart)
	{
		int i = indexStart;
		
		auto addPlace = [&disableZones, &player](int location, int sequence)
		{
			auto place = disableZones->add_places();
			place->set_controller(player);
			place->set_location(location);
			place->set_sequence(sequence);
		};

		// Monster zones, including extra monster zones
		// NOTE: both players have extra monster zone
		for(int sequence = 0;i < indexStart + 7;i++,sequence++)
			if(flag[i])
				addPlace(0x04, sequence); // LOCATION_MZONE

		i++; // UNUSED BIT

		// Spell zones
		for(int sequence = 0;i < indexStart + 7 + 1 + 5;i++,sequence++)
			if(flag[i])
				addPlace(0x08, sequence); // LOCATION_SZONE

		// Field zone
		if(flag[i])
			addPlace(0x0100, 0); // LOCATION_FZONE
		i++;

		// Pendulum zones
		for(int sequence = 0;i < indexStart + 7 + 1 + 5 + 1;i++,sequence++)
			if(flag[i])
				addPlace(0x0200, sequence); // LOCATION_SZONE
	};
	
	ExtractPlaces(0, 0);
	ExtractPlaces(1, 16);
#endif // FILTERING
CASE(MSG_SUMMONING)
#ifndef FILTERING
	auto summonCard = information->mutable_summon_card();
	
	summonCard->set_type(Core::Msg::SummonCard::SUMMON_NORMAL);
	
	auto card = summonCard->mutable_card();
	ToCardCode(w->read<code_t>("card code"), card);
	READ_INFO_LOC_CORE(w, card);
#endif // FILTERING
CASE(MSG_SUMMONED)
#ifndef FILTERING
	auto summonCard = information->mutable_summon_card();
	
	summonCard->set_type(Core::Msg::SummonCard::SUMMON_NORMAL);
#endif // FILTERING
CASE(MSG_SPSUMMONING)
#ifndef FILTERING
	auto summonCard = information->mutable_summon_card();
	
	summonCard->set_type(Core::Msg::SummonCard::SUMMON_SPECIAL);
	
	auto card = summonCard->mutable_card();
	ToCardCode(w->read<code_t>("card code"), card);
	READ_INFO_LOC_CORE(w, card);
#endif // FILTERING
CASE(MSG_SPSUMMONED)
#ifndef FILTERING
	auto summonCard = information->mutable_summon_card();
	
	summonCard->set_type(Core::Msg::SummonCard::SUMMON_SPECIAL);
#endif // FILTERING
CASE(MSG_FLIPSUMMONING)
#ifndef FILTERING
	auto summonCard = information->mutable_summon_card();
	
	summonCard->set_type(Core::Msg::SummonCard::SUMMON_FLIP);
	
	auto card = summonCard->mutable_card();
	ToCardCode(w->read<code_t>("card code"), card);
	READ_INFO_LOC_CORE(w, card);
#endif // FILTERING
CASE(MSG_FLIPSUMMONED)
#ifndef FILTERING
	auto summonCard = information->mutable_summon_card();
	
	summonCard->set_type(Core::Msg::SummonCard::SUMMON_FLIP);
#endif // FILTERING
CASE(MSG_CHAINING)
#ifndef FILTERING
	auto chainAction = information->mutable_chain_action();
	chainAction->set_type(Core::Msg::ChainAction::ACTION_CHAINING);
	
	auto card = chainAction->mutable_card();
	ToCardCode(w->read<code_t>("card code"), card);
	READ_INFO_LOC_CORE(w, card);
	
	auto place = chainAction->mutable_place();
	place->set_controller(w->read<player_t>("player"));
	place->set_location(w->read<s_loc_t>("location"));
	place->set_sequence(w->read<seq_t>("sequence"));
	
	ToEffectDesc(w->read<ed_t>("effectdesc"), chainAction->mutable_ed());
	
	chainAction->set_chain_number(w->read<uint32_t>("chain num"));
#endif // FILTERING
CASE(MSG_CHAINED)
#ifndef FILTERING
	auto chainAction = information->mutable_chain_action();
	chainAction->set_type(Core::Msg::ChainAction::ACTION_CHAINED);
	chainAction->set_chain_number(w->read<uint8_t>("chain num"));
#endif // FILTERING
CASE(MSG_CHAIN_SOLVING)
#ifndef FILTERING
	auto chainAction = information->mutable_chain_action();
	chainAction->set_type(Core::Msg::ChainAction::ACTION_CHAIN_SOLVING);
	chainAction->set_chain_number(w->read<uint8_t>("chain num"));
#endif // FILTERING
CASE(MSG_CHAIN_SOLVED)
#ifndef FILTERING
	auto chainAction = information->mutable_chain_action();
	chainAction->set_type(Core::Msg::ChainAction::ACTION_CHAIN_SOLVED);
	chainAction->set_chain_number(w->read<uint8_t>("chain num"));
#endif // FILTERING
CASE(MSG_CHAIN_END)
#ifndef FILTERING
	auto chainAction = information->mutable_chain_action();
	chainAction->set_type(Core::Msg::ChainAction::ACTION_CHAIN_END);
#endif // FILTERING
CASE(MSG_CHAIN_NEGATED)
#ifndef FILTERING
	auto chainAction = information->mutable_chain_action();
	chainAction->set_type(Core::Msg::ChainAction::ACTION_CHAIN_NEGATED);
	chainAction->set_chain_number(w->read<uint8_t>("chain num"));
#endif // FILTERING
CASE(MSG_CHAIN_DISABLED)
#ifndef FILTERING
	auto chainAction = information->mutable_chain_action();
	chainAction->set_type(Core::Msg::ChainAction::ACTION_CHAIN_DISABLED);
	chainAction->set_chain_number(w->read<uint8_t>("chain num"));
#endif // FILTERING
CASE(MSG_RANDOM_SELECTED)
#ifndef FILTERING
	auto selectedCards = information->mutable_selected_cards();
	selectedCards->set_type(Core::Msg::SelectedCards::SELECTION_RANDOM);
	
	w->seek(1, Buffer::seek_dir::cur, "player");
	
	auto Add1 = BIND_FUNC_TO_OBJ_PTR(selectedCards, add_cards);
	auto count = w->read<count_t>("count");
	for(decltype(count) i = 0; i < count; i++)
		READ_INFO_LOC_CORE(w, Add1());
#endif // FILTERING
CASE(MSG_BECOME_TARGET)
#ifndef FILTERING
	auto selectedCards = information->mutable_selected_cards();
	selectedCards->set_type(Core::Msg::SelectedCards::SELECTION_BECOME);
	
	auto Add1 = BIND_FUNC_TO_OBJ_PTR(selectedCards, add_cards);
	auto count = w->read<count_t>("count");
	for(decltype(count) i = 0; i < count; i++)
		READ_INFO_LOC_CORE(w, Add1());
#endif // FILTERING
CASE(MSG_DRAW)
#ifndef FILTERING
	auto draw = information->mutable_draw();
	draw->set_player(w->read<player_t>("player"));
	
	auto Add1 = BIND_FUNC_TO_OBJ_PTR(draw, add_cards);
	auto count = w->read<count_t>("count");
	for(decltype(count) i = 0; i < count; i++)
		ToCardCode(w->read<code_t>("card code", (int)i), Add1());
#endif // FILTERING
CASE(MSG_DAMAGE)
#ifndef FILTERING
	auto lpChange = information->mutable_lp_change();
	lpChange->set_type(Core::Msg::LpChange::CHANGE_DAMAGE);
	lpChange->set_player(w->read<player_t>("player"));
	lpChange->set_amount(w->read<uint32_t>("amount"));
#endif // FILTERING
CASE(MSG_RECOVER)
#ifndef FILTERING
	auto lpChange = information->mutable_lp_change();
	lpChange->set_type(Core::Msg::LpChange::CHANGE_RECOVER);
	lpChange->set_player(w->read<player_t>("player"));
	lpChange->set_amount(w->read<uint32_t>("amount"));
#endif // FILTERING
CASE(MSG_LPUPDATE)
#ifndef FILTERING
	auto lpChange = information->mutable_lp_change();
	lpChange->set_type(Core::Msg::LpChange::CHANGE_BECOME);
	lpChange->set_player(w->read<player_t>("player"));
	lpChange->set_amount(w->read<uint32_t>("amount"));
#endif // FILTERING
CASE(MSG_PAY_LPCOST)
#ifndef FILTERING
	auto lpChange = information->mutable_lp_change();
	lpChange->set_type(Core::Msg::LpChange::CHANGE_PAY);
	lpChange->set_player(w->read<player_t>("player"));
	lpChange->set_amount(w->read<uint32_t>("amount"));
#endif // FILTERING
CASE(MSG_CARD_TARGET)
#ifndef FILTERING
	auto selectedCards = information->mutable_selected_cards();
	selectedCards->set_type(Core::Msg::SelectedCards::SELECTION_CARD_TARGET);
	
	READ_INFO_LOC_CORE(w, selectedCards->mutable_targeting_card());
	READ_INFO_LOC_CORE(w, selectedCards->add_cards());
#endif // FILTERING
CASE(MSG_CANCEL_TARGET)
#ifndef FILTERING
	auto selectedCards = information->mutable_selected_cards();
	selectedCards->set_type(Core::Msg::SelectedCards::SELECTION_CARD_DETARGET);
	
	READ_INFO_LOC_CORE(w, selectedCards->mutable_targeting_card());
	READ_INFO_LOC_CORE(w, selectedCards->add_cards());
#endif // FILTERING
CASE(MSG_ADD_COUNTER)
#ifndef FILTERING
	auto counterChange = information->mutable_counter_change();
	counterChange->set_type(Core::Msg::CounterChange::CHANGE_ADD);
	
	auto counter = counterChange->mutable_counter();
	auto place = counterChange->mutable_place();
	counter->set_type(w->read<uint16_t>("counter type"));
	place->set_controller(w->read<player_t>("player"));
	place->set_location(w->read<s_loc_t>("location"));
	place->set_sequence(w->read<s_seq_t>("sequence"));
	counter->set_count(w->read<uint16_t>("counter count"));
#endif // FILTERING
CASE(MSG_REMOVE_COUNTER)
#ifndef FILTERING
	auto counterChange = information->mutable_counter_change();
	counterChange->set_type(Core::Msg::CounterChange::CHANGE_REMOVE);
	
	auto counter = counterChange->mutable_counter();
	auto place = counterChange->mutable_place();
	counter->set_type(w->read<uint16_t>("counter type"));
	place->set_controller(w->read<player_t>("player"));
	place->set_location(w->read<s_loc_t>("location"));
	place->set_sequence(w->read<s_seq_t>("sequence"));
	counter->set_count(w->read<uint16_t>("counter count"));
#endif // FILTERING
CASE(MSG_ATTACK)
#ifndef FILTERING
	auto onAttack = information->mutable_on_attack();
	onAttack->set_status(Core::Msg::OnAttack::STATUS_ATTACK_DECLARATION);
	
	READ_INFO_LOC_CORE(w, onAttack->mutable_attacker());
	
	auto atkTarget = onAttack->mutable_attack_target();
	READ_INFO_LOC_CORE(w, atkTarget);
	
	// clear card if location is empty
	if(atkTarget->location() == 0 && atkTarget->sequence() == 0)
		onAttack->clear_attack_target();
#endif // FILTERING
CASE(MSG_BATTLE)
#ifndef FILTERING
	auto onAttack = information->mutable_on_attack();
	onAttack->set_status(Core::Msg::OnAttack::STATUS_ATTACK_DECLARATION);
	
	auto attacker = onAttack->mutable_attacker();
	auto atkTarget = onAttack->mutable_attack_target();
	
	READ_INFO_LOC_CORE(w, attacker);
	attacker->set_atk(w->read<uint32_t>("attacker atk"));
	attacker->set_def(w->read<uint32_t>("attacker def"));
	w->seek(1, Buffer::seek_dir::cur, "attacker battle destroyed");
	
	READ_INFO_LOC_CORE(w, atkTarget);
	atkTarget->set_atk(w->read<uint32_t>("attack_target atk"));
	atkTarget->set_def(w->read<uint32_t>("attack_target def"));
	w->seek(1, Buffer::seek_dir::cur, "attack_target battle destroyed");
	
	// clear card if location is empty
	if(atkTarget->location() == 0 && atkTarget->sequence() == 0)
		onAttack->clear_attack_target();
#endif // FILTERING
CASE(MSG_ATTACK_DISABLED)
#ifndef FILTERING
	auto miscAction = information->mutable_misc_action();
	miscAction->set_type(Core::Msg::MiscAction::ACTION_ATTACK_NEGATED);
#endif // FILTERING
CASE(MSG_DAMAGE_STEP_START)
#ifndef FILTERING
	auto miscAction = information->mutable_misc_action();
	miscAction->set_type(Core::Msg::MiscAction::ACTION_DAMAGE_STEP_START);
#endif // FILTERING
CASE(MSG_DAMAGE_STEP_END)
#ifndef FILTERING
	auto miscAction = information->mutable_misc_action();
	miscAction->set_type(Core::Msg::MiscAction::ACTION_DAMAGE_STEP_END);
#endif // FILTERING
CASE(MSG_TOSS_COIN)
#ifndef FILTERING
	auto result = information->mutable_result();
	result->set_type(Core::Msg::Result::RESULT_TOSS_COIN);
	
	result->set_player(w->read<player_t>("player"));
	
	auto count = w->read<uint8_t>("count");
	for(decltype(count) i = 0; i < count; i++)
		result->add_results(w->read<uint8_t>("result ", (int)i));
#endif // FILTERING
CASE(MSG_TOSS_DICE)
#ifndef FILTERING
	auto result = information->mutable_result();
	result->set_type(Core::Msg::Result::RESULT_TOSS_DICE);
	
	result->set_player(w->read<player_t>("player"));
	
	auto count = w->read<uint8_t>("count");
	for(decltype(count) i = 0; i < count; i++)
		result->add_results(w->read<uint8_t>("result ", (int)i));
#endif // FILTERING
CASE(MSG_HAND_RES)
#ifndef FILTERING
	auto result = information->mutable_result();
	result->set_type(Core::Msg::Result::RESULT_RPS);
	
	auto handResults = w->read<uint8_t>("hand results");
	result->add_results((handResults & 0x3) - 1);
	result->add_results(((handResults >> 2) & 0x3) - 1);
#endif // FILTERING
CASE(MSG_CARD_HINT)
#ifndef FILTERING
	auto cardHint = information->mutable_card_hint();
	
	auto card = cardHint->mutable_card();
	READ_INFO_LOC_CORE(w, card);
	
	cardHint->set_type(w->read<uint8_t>("hint type"));
	cardHint->set_data(w->read<uint64_t>("hint data"));
#endif // FILTERING
CASE(MSG_PLAYER_HINT)
#ifndef FILTERING
	auto playerHint = information->mutable_player_hint();
	
	playerHint->set_player(w->read<player_t>("player"));
	playerHint->set_type(w->read<uint8_t>("hint type"));
	playerHint->set_data(w->read<uint64_t>("hint data"));
#endif // FILTERING
CASE(MSG_TAG_SWAP)
#ifndef FILTERING
	auto swapPlayer = information->mutable_swap_player();
	
	swapPlayer->set_player(w->read<player_t>("player"));
	swapPlayer->set_main_deck_count(w->read<count_t>("main deck count"));
	
	auto extraCount = w->read<count_t>("extra deck count");
	w->seek(sizeof(count_t), Buffer::seek_dir::cur, "face-up pendulums");
	auto handCount = w->read<count_t>("hand deck count");
	
	ToCardCode(w->read<code_t>("decktop"), swapPlayer->mutable_decktop());
	
	auto Add1 = BIND_FUNC_TO_OBJ_PTR(swapPlayer, add_hand_cards);
	auto Add2 = BIND_FUNC_TO_OBJ_PTR(swapPlayer, add_extra_cards);
	
	for(decltype(handCount) i = 0; i < handCount; i++)
		ToCardCode(w->read<code_t>("hand card ", (int)i), Add1());
	
	for(decltype(extraCount) i = 0; i < extraCount; i++)
		ToCardCode(w->read<code_t>("extra card ", (int)i), Add2());
#endif // FILTERING
CASE(MSG_MATCH_KILL)
#ifndef FILTERING
	information->set_match_killer(w->read<code_t>("card code"));
#endif // FILTERING
CASE_FINAL()
#ifdef FILTERING
default: { return false; break; }
#endif // FILTERING

#include "encoder_impl_end.hpp"
