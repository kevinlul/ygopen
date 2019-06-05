#include "encoder_impl_begin.hpp"

CASE(MSG_WIN,
	auto win = information->mutable_win();
	win->set_player(w->read<player_t>("player"));
	win->set_reason(w->read<uint8_t>("reason"));
);

#define CONFIRM_CARDS(t, size) \
	auto confirmCards = information->mutable_confirm_cards(); \
	confirmCards->set_type(Core::Msg::ConfirmCards::t); \
	w->seek(1, Buffer::seek_dir::cur, "player"); \
	CardSpawner Add1 = BIND_FUNC_TO_OBJ_PTR(confirmCards, add_cards); \
	ReadCardVector<count_t, s_loc_t, size>(w, Add1);
CASE(MSG_CONFIRM_DECKTOP,
	CONFIRM_CARDS(CONFIRM_DECKTOP, s_seq_t)
);
CASE(MSG_CONFIRM_CARDS,
	CONFIRM_CARDS(CONFIRM_MULTIPLE, seq_t)
);
#undef CONFIRM_CARDS

CASE(MSG_SHUFFLE_DECK,
	auto shuffleLoc = information->mutable_shuffle_location();
	shuffleLoc->set_player(w->read<player_t>("player"));
	shuffleLoc->set_location(0x01); // LOCATION_DECK
	// NOTE: empty decklist
);

CASE(MSG_SHUFFLE_HAND,
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
);

CASE(MSG_SWAP_GRAVE_DECK,
	auto miscAction = information->mutable_misc_action();
	miscAction->set_type(Core::Msg::MiscAction::ACTION_SWAP_GY_WITH_DECK);
	miscAction->set_player(w->read<player_t>("player"));
);

CASE(MSG_SHUFFLE_SET_CARD,
	auto shuffleSet = information->mutable_shuffle_set_cards();
	w->seek(1, Buffer::seek_dir::cur, "location");
	CardSpawner Add1 = BIND_FUNC_TO_OBJ_PTR(shuffleSet, add_cards_previous);
	CardSpawner Add2 = BIND_FUNC_TO_OBJ_PTR(shuffleSet, add_cards_current);
	auto count = w->read<s_count_t>(".size()");
	for(decltype(count) i = 0; i < count; i++)
		READ_INFO_LOC_CORE(w, Add1());
	for(decltype(count) i = 0; i < count; i++)
		READ_INFO_LOC_CORE(w, Add2());
);

CASE(MSG_DECK_TOP,
	auto updateCard = information->mutable_update_card();
	updateCard->set_reason(Core::Msg::UpdateCard::REASON_DECK_TOP);
	auto previous = updateCard->mutable_previous();
	previous->set_controller(w->read<player_t>("player"));
	previous->set_sequence(w->read<s_seq_t>("sequence"));
	previous->set_location(0x01); // LOCATION_DECK
	auto current = updateCard->mutable_current();
	ToCardCode(w->read<code_t>("card code"), current);
	// NOTE: maybe move the dirty bit to the position parameter?
);

CASE(MSG_SHUFFLE_EXTRA,
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
);

CASE(MSG_NEW_TURN,
	auto newTurn = information->mutable_new_turn();
	newTurn->set_turn_player(w->read<player_t>("turn player"));
);

CASE(MSG_NEW_PHASE,
	auto newPhase = information->mutable_new_phase();
	newPhase->set_phase(w->read<uint16_t>("phase"));
);

CASE(MSG_CONFIRM_EXTRATOP,
	auto confirmCards = information->mutable_confirm_cards();
	confirmCards->set_type(Core::Msg::ConfirmCards::CONFIRM_EXTRATOP);
	w->seek(1, Buffer::seek_dir::cur, "player");
	CardSpawner Add1 = BIND_FUNC_TO_OBJ_PTR(confirmCards, add_cards);
	ReadCardVector<count_t, s_loc_t, s_seq_t>(w, Add1);
);

CASE(MSG_MOVE,
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
);

CASE(MSG_POS_CHANGE,
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
);

CASE(MSG_SET,
	auto updateCard = information->mutable_update_card();
	updateCard->set_reason(Core::Msg::UpdateCard::REASON_SET);
	auto previous = updateCard->mutable_previous();
	auto current = updateCard->mutable_current();
	ToCardCode(w->read<code_t>("card code"), current);
	previous->set_controller(w->read<player_t>("controller"));
	previous->set_location(w->read<s_loc_t>("location"));
	previous->set_sequence(w->read<seq_t>("sequence"));
	current->set_position(w->read<pos_t>("current position"));
);

CASE(MSG_SWAP,
	auto swapCards = information->mutable_swap_cards();
	auto ReadCardInfo = [&w](Core::Data::CardInfo* card)
	{
		ToCardCode(w->read<code_t>("card code"), card);
		READ_INFO_LOC_CORE(w, card);
	};
	ReadCardInfo(swapCards->mutable_card1());
	ReadCardInfo(swapCards->mutable_card2());
);

CASE(MSG_FIELD_DISABLED,
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
);

#define SUMMON_CARD1(t) \
	auto summonCard = information->mutable_summon_card(); \
	summonCard->set_type(Core::Msg::SummonCard::t); \
	auto card = summonCard->mutable_card(); \
	ToCardCode(w->read<code_t>("card code"), card); \
	READ_INFO_LOC_CORE(w, card);
#define SUMMON_CARD2(t) \
	auto summonCard = information->mutable_summon_card(); \
	summonCard->set_type(Core::Msg::SummonCard::t);
CASE(MSG_SUMMONING,
	SUMMON_CARD1(SUMMON_NORMAL)
);
CASE(MSG_SPSUMMONING,
	SUMMON_CARD1(SUMMON_SPECIAL)
);
CASE(MSG_FLIPSUMMONING,
	SUMMON_CARD1(SUMMON_FLIP)
);
CASE(MSG_SUMMONED,
	SUMMON_CARD2(SUMMON_NORMAL)
);
CASE(MSG_SPSUMMONED,
	SUMMON_CARD2(SUMMON_SPECIAL)
);
CASE(MSG_FLIPSUMMONED,
	SUMMON_CARD2(SUMMON_FLIP)
);
#undef SUMMON_CARD1
#undef SUMMON_CARD2

#define CHAIN_ACTION(a) \
	auto chainAction = information->mutable_chain_action(); \
	chainAction->set_type(Core::Msg::ChainAction::a); \
	chainAction->set_chain_number(w->read<uint8_t>("chain num"));
CASE(MSG_CHAINED,
	CHAIN_ACTION(ACTION_CHAINED)
);
CASE(MSG_CHAIN_SOLVING,
	CHAIN_ACTION(ACTION_CHAIN_SOLVING)
);
CASE(MSG_CHAIN_SOLVED,
	CHAIN_ACTION(ACTION_CHAIN_SOLVED)
);
CASE(MSG_CHAIN_NEGATED,
	CHAIN_ACTION(ACTION_CHAIN_NEGATED)
);
CASE(MSG_CHAIN_DISABLED,
	CHAIN_ACTION(ACTION_CHAIN_DISABLED)
);
#undef CHAIN_ACTION

CASE(MSG_CHAINING,
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
);

CASE(MSG_CHAIN_END,
	auto chainAction = information->mutable_chain_action();
	chainAction->set_type(Core::Msg::ChainAction::ACTION_CHAIN_END);
);

CASE(MSG_DRAW,
	auto draw = information->mutable_draw();
	draw->set_player(w->read<player_t>("player"));
	auto Add1 = BIND_FUNC_TO_OBJ_PTR(draw, add_cards);
	auto count = w->read<count_t>("count");
	for(decltype(count) i = 0; i < count; i++)
		ToCardCode(w->read<code_t>("card code ", (int)i), Add1());
);

#define LP_CHANGE(t) \
	auto lpChange = information->mutable_lp_change(); \
	lpChange->set_type(Core::Msg::LpChange::t); \
	lpChange->set_player(w->read<player_t>("player")); \
	lpChange->set_amount(w->read<uint32_t>("amount"));
CASE(MSG_DAMAGE,
	LP_CHANGE(CHANGE_DAMAGE)
);
CASE(MSG_RECOVER,
	LP_CHANGE(CHANGE_RECOVER)
);
CASE(MSG_LPUPDATE,
	LP_CHANGE(CHANGE_BECOME)
);
CASE(MSG_PAY_LPCOST,
	LP_CHANGE(CHANGE_PAY)
);
#undef LP_CHANGE

#define SELECTED1(t, ...) \
	auto selectedCards = information->mutable_selected_cards(); \
	selectedCards->set_type(Core::Msg::SelectedCards::t); \
	__VA_ARGS__ \
	auto Add1 = BIND_FUNC_TO_OBJ_PTR(selectedCards, add_cards); \
	auto count = w->read<count_t>("count"); \
	for(decltype(count) i = 0; i < count; i++) \
		READ_INFO_LOC_CORE(w, Add1());
#define SELECTED2(t) \
	auto selectedCards = information->mutable_selected_cards(); \
	selectedCards->set_type(Core::Msg::SelectedCards::t); \
	READ_INFO_LOC_CORE(w, selectedCards->mutable_targeting_card()); \
	READ_INFO_LOC_CORE(w, selectedCards->add_cards());
CASE(MSG_RANDOM_SELECTED,
	SELECTED1(SELECTION_RANDOM, w->seek(1, Buffer::seek_dir::cur, "player");)
);
CASE(MSG_BECOME_TARGET,
	SELECTED1(SELECTION_BECOME)
);
CASE(MSG_CARD_TARGET,
	SELECTED2(SELECTION_CARD_TARGET)
);
CASE(MSG_CANCEL_TARGET,
	SELECTED2(SELECTION_CARD_DETARGET)
);
#undef SELECTED1
#undef SELECTED2

#define COUNTER_CHANGE(t) \
	auto counterChange = information->mutable_counter_change(); \
	counterChange->set_type(Core::Msg::CounterChange::t); \
	auto counter = counterChange->mutable_counter(); \
	auto place = counterChange->mutable_place(); \
	counter->set_type(w->read<uint16_t>("counter type")); \
	place->set_controller(w->read<player_t>("player")); \
	place->set_location(w->read<s_loc_t>("location")); \
	place->set_sequence(w->read<s_seq_t>("sequence")); \
	counter->set_count(w->read<uint16_t>("counter count"));
CASE(MSG_ADD_COUNTER,
	COUNTER_CHANGE(CHANGE_ADD)
);
CASE(MSG_REMOVE_COUNTER,
	COUNTER_CHANGE(CHANGE_REMOVE)
);
#undef COUNTER_CHANGE

CASE(MSG_ATTACK,
	auto onAttack = information->mutable_on_attack();
	onAttack->set_status(Core::Msg::OnAttack::STATUS_ATTACK_DECLARATION);
	READ_INFO_LOC_CORE(w, onAttack->mutable_attacker());
	auto atkTarget = onAttack->mutable_attack_target();
	READ_INFO_LOC_CORE(w, atkTarget);
	// clear card if location is empty
	if(atkTarget->location() == 0 && atkTarget->sequence() == 0)
		onAttack->clear_attack_target();
);

CASE(MSG_BATTLE,
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
);

#define MISC_ACTION(t) \
	auto miscAction = information->mutable_misc_action(); \
	miscAction->set_type(Core::Msg::MiscAction::t);
CASE(MSG_REVERSE_DECK,
	MISC_ACTION(ACTION_REVERSE_DECK)
);
CASE(MSG_ATTACK_DISABLED,
	MISC_ACTION(ACTION_ATTACK_NEGATED)
);
CASE(MSG_DAMAGE_STEP_START,
	MISC_ACTION(ACTION_DAMAGE_STEP_START)
);
CASE(MSG_DAMAGE_STEP_END,
	MISC_ACTION(ACTION_DAMAGE_STEP_END)
);
#undef MISC_ACTION

#define RESULT(t) \
	auto result = information->mutable_result(); \
	result->set_type(Core::Msg::Result::t); \
	result->set_player(w->read<player_t>("player")); \
	auto count = w->read<s_count_t>("count"); \
	for(decltype(count) i = 0; i < count; i++) \
		result->add_results(w->read<uint8_t>("result ", (int)i));
CASE(MSG_TOSS_COIN,
	RESULT(RESULT_TOSS_COIN)
);
CASE(MSG_TOSS_DICE,
	RESULT(RESULT_TOSS_DICE)
);
#undef RESULT

CASE(MSG_HAND_RES,
	auto result = information->mutable_result();
	result->set_type(Core::Msg::Result::RESULT_RPS);
	auto handResults = w->read<uint8_t>("hand results");
	result->add_results((handResults & 0x3) - 1);
	result->add_results(((handResults >> 2) & 0x3) - 1);
);

CASE(MSG_CARD_HINT,
	auto cardHint = information->mutable_card_hint();
	auto card = cardHint->mutable_card();
	READ_INFO_LOC_CORE(w, card);
	cardHint->set_type(w->read<uint8_t>("hint type"));
	cardHint->set_data(w->read<uint64_t>("hint data"));
);

CASE(MSG_PLAYER_HINT,
	auto playerHint = information->mutable_player_hint();
	playerHint->set_player(w->read<player_t>("player"));
	playerHint->set_type(w->read<uint8_t>("hint type"));
	playerHint->set_data(w->read<uint64_t>("hint data"));
);

CASE(MSG_TAG_SWAP,
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
);

CASE(MSG_MATCH_KILL,
	information->set_match_killer(w->read<code_t>("card code"));
);

#ifdef FILTERING
default: { return false; break; }
#endif // FILTERING

#include "encoder_impl_end.hpp"
