#include "encoder_impl_begin.hpp"

CASE_FIRST(MSG_SELECT_BATTLECMD)
#ifndef FILTERING
	auto selectCmd = specific->mutable_request()->mutable_select_cmd();
	
	selectCmd->set_type(Core::Msg::SelectCmd::COMMAND_BATTLE);

	CardSpawner add_w_effect = BIND_FUNC_TO_OBJ_PTR(selectCmd, add_cards_w_effect);
	InlineCardRead ReadEffectDesc = [](Buffer::ibufferw& wrapper, Core::Data::CardInfo* card)
	{
		ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc"), card->mutable_effect_desc());
	};
	ReadCardVector<cardcount_t, small_location_t, sequence_t>(wrapper, add_w_effect, nullptr, ReadEffectDesc);
	
	CardSpawner add_can_attack = BIND_FUNC_TO_OBJ_PTR(selectCmd, add_cards_can_attack);
	InlineCardRead ReadAtkDirectly = [](Buffer::ibufferw& wrapper, Core::Data::CardInfo* card)
	{
		card->set_can_attack_directly(wrapper->read<uint8_t>("can_attack_directly"));
	};
	ReadCardVector<cardcount_t, small_location_t, small_sequence_t>(wrapper, add_can_attack, nullptr, ReadAtkDirectly);
	
	selectCmd->set_able_to_mp2(wrapper->read<uint8_t>("to_mp2"));
	selectCmd->set_able_to_ep(wrapper->read<uint8_t>("to_ep"));
#endif // FILTERING
CASE(MSG_SELECT_IDLECMD)
#ifndef FILTERING
	auto selectCmd = specific->mutable_request()->mutable_select_cmd();

	selectCmd->set_type(Core::Msg::SelectCmd::COMMAND_IDLE);

	CardSpawner add_summonable = BIND_FUNC_TO_OBJ_PTR(selectCmd, add_cards_summonable);
	ReadCardVector<cardcount_t, small_location_t, sequence_t>(wrapper, add_summonable);

	CardSpawner add_spsummonable = BIND_FUNC_TO_OBJ_PTR(selectCmd, add_cards_spsummonable);
	ReadCardVector<cardcount_t, small_location_t, sequence_t>(wrapper, add_spsummonable);

	CardSpawner add_repositionable = BIND_FUNC_TO_OBJ_PTR(selectCmd, add_cards_repositionable);
	ReadCardVector<cardcount_t, small_location_t, small_sequence_t>(wrapper, add_repositionable);

	CardSpawner add_msetable = BIND_FUNC_TO_OBJ_PTR(selectCmd, add_cards_msetable);
	ReadCardVector<cardcount_t, small_location_t, sequence_t>(wrapper, add_msetable);

	CardSpawner add_ssetable = BIND_FUNC_TO_OBJ_PTR(selectCmd, add_cards_ssetable);
	ReadCardVector<cardcount_t, small_location_t, sequence_t>(wrapper, add_ssetable);

	CardSpawner add_w_effect = BIND_FUNC_TO_OBJ_PTR(selectCmd, add_cards_w_effect);
	InlineCardRead ReadEffectDesc = [](Buffer::ibufferw& wrapper, Core::Data::CardInfo* card)
	{
		ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc"), card->mutable_effect_desc());
	};
	ReadCardVector<cardcount_t, small_location_t, sequence_t>(wrapper, add_w_effect, nullptr, ReadEffectDesc);

	selectCmd->set_able_to_bp(wrapper->read<uint8_t>("to_bp"));
	selectCmd->set_able_to_ep(wrapper->read<uint8_t>("to_ep"));
	selectCmd->set_can_shuffle(wrapper->read<uint8_t>("can_shuffle"));
#endif // FILTERING
CASE(MSG_SELECT_EFFECTYN)
#ifndef FILTERING
	auto selectYesNo = specific->mutable_request()->mutable_select_yesno();

	auto card = selectYesNo->mutable_card();
	ToCardCode(wrapper->read<cardcode_t>("card code"), card);
	ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, card);
	ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc"), card->mutable_effect_desc());
#endif // FILTERING
CASE(MSG_SELECT_YESNO)
#ifndef FILTERING
	auto selectYesNo = specific->mutable_request()->mutable_select_yesno();

	ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc"), selectYesNo->mutable_effect());
#endif // FILTERING
CASE(MSG_SELECT_OPTION)
#ifndef FILTERING
	auto selectOption = specific->mutable_request()->mutable_select_option();

	auto count = wrapper->read<uint8_t>("number of effects");
	for(decltype(count) i = 0; i < count; i++)
		ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc ", (int)count), selectOption->add_effects());
#endif // FILTERING
CASE(MSG_SELECT_CARD)
#ifndef FILTERING
	auto selectCards = specific->mutable_request()->mutable_select_cards();

	selectCards->set_type(Core::Msg::SelectCards::SELECTION_EFFECT);

	selectCards->set_can_cancel(wrapper->read<uint8_t>("can_cancel"));
	selectCards->set_can_accept(false);

	selectCards->set_min(wrapper->read<uint32_t>("min"));
	selectCards->set_max(wrapper->read<uint32_t>("max"));

	CardSpawner add_selectable = BIND_FUNC_TO_OBJ_PTR(selectCards, add_cards_selectable);
	ReadCardVector<cardcount_t, small_location_t, sequence_t, position_t>(wrapper, add_selectable);
#endif // FILTERING
CASE(MSG_SELECT_CHAIN)
#ifndef FILTERING
	auto selectToChain = specific->mutable_request()->mutable_select_to_chain();

	auto count = wrapper->read<uint32_t>("number of chains");

	wrapper->seek(1, Buffer::seek_dir::cur, "spe_count");

	selectToChain->set_forced(wrapper->read<uint8_t>("forced"));

	wrapper->seek(8, Buffer::seek_dir::cur, "hint_timing x 2");

	// NOTE: can probably be improved upon but i cannot think
	// of a better way of doing it right now
	for(decltype(count) i = 0; i < count; i++)
	{
		wrapper->log("chain card", (int)i);
		auto chainCard = selectToChain->add_cards_w_effect();

		chainCard->set_effect_desc_type(wrapper->read<uint8_t>("EDESC ", (int)i));
		auto card = chainCard->mutable_card();

		ToCardCode(wrapper->read<cardcode_t>("card code"), card);
		ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, card);
		ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc"), card->mutable_effect_desc());
	}
}
break;
case MSG_SELECT_PLACE:
// case MSG_SELECT_DISFIELD:
{
	// NOTE:
	// The available positions are always current player places first
	// and then opponent available places
	auto selectPlaces = specific->mutable_request()->mutable_select_places();

	selectPlaces->set_min(wrapper->read<uint8_t>("count (min)"));

	std::bitset<32> flag(wrapper->read<uint32_t>("flag"));

	auto ExtractPlaces = [&selectPlaces, &flag](int player, int indexStart)
	{
		int i = indexStart;
		
		auto addPlace = [&selectPlaces, &player](int location, int sequence)
		{
			auto place = selectPlaces->add_places();
			place->set_controller(player);
			place->set_location(location);
			place->set_sequence(sequence);
		};

		// Monster zones, including extra monster zones
		// NOTE: both players have extra monster zone
		for(int sequence = 0;i < indexStart + 7;i++,sequence++)
			if(!flag[i])
				addPlace(0x04, sequence); // LOCATION_MZONE

		i++; // UNUSED BIT

		// Spell zones
		for(int sequence = 0;i < indexStart + 7 + 1 + 5;i++,sequence++)
			if(!flag[i])
				addPlace(0x08, sequence); // LOCATION_SZONE

		// Field zone
		if(!flag[i])
			addPlace(0x0100, 0); // LOCATION_FZONE
		i++;

		// Pendulum zones
		for(int sequence = 0;i < indexStart + 7 + 1 + 5 + 1;i++,sequence++)
			if(!flag[i])
				addPlace(0x0200, sequence); // LOCATION_SZONE
	};

	ExtractPlaces(0, 0);
	ExtractPlaces(1, 16);
#endif // FILTERING
CASE(MSG_SELECT_POSITION)
#ifndef FILTERING
	auto selectPosition = specific->mutable_request()->mutable_select_position();
	
	auto card = selectPosition->mutable_card();
	
	ToCardCode(wrapper->read<cardcode_t>("card code"), card);
	card->set_position(wrapper->read<small_position_t>("positions"));
#endif // FILTERING
CASE(MSG_SELECT_TRIBUTE)
#ifndef FILTERING
	auto selectCards = specific->mutable_request()->mutable_select_cards();

	selectCards->set_type(Core::Msg::SelectCards::SELECTION_TRIBUTE);

	selectCards->set_can_cancel(wrapper->read<uint8_t>("can_cancel"));
	selectCards->set_can_accept(false);

	selectCards->set_min(wrapper->read<uint32_t>("min"));
	selectCards->set_max(wrapper->read<uint32_t>("max"));

	CardSpawner add_selectable = BIND_FUNC_TO_OBJ_PTR(selectCards, add_cards_selectable);
	InlineCardRead ReadReleaseParam = [](Buffer::ibufferw& wrapper, Core::Data::CardInfo* card)
	{
		card->set_tribute_count(wrapper->read<uint8_t>("release_param"));
	};
	ReadCardVector<cardcount_t, small_location_t, sequence_t, void>(wrapper, add_selectable, nullptr, ReadReleaseParam);
}
break;
// case MSG_SORT_CARD:
case MSG_SORT_CHAIN:
{
	auto sortCards = specific->mutable_request()->mutable_sort_cards();
	CardSpawner add_cards_to_sort = BIND_FUNC_TO_OBJ_PTR(sortCards, add_cards_to_sort);
	ReadCardVector<cardcount_t, small_location_t, sequence_t, position_t>(wrapper, add_cards_to_sort);
#endif // FILTERING
CASE(MSG_SELECT_COUNTER)
#ifndef FILTERING
	auto selectCards = specific->mutable_request()->mutable_select_cards();

	selectCards->set_type(Core::Msg::SelectCards::SELECTION_COUNTER);

	selectCards->set_can_cancel(false);
	selectCards->set_can_accept(false);
	
	auto counter = selectCards->mutable_counter();
	counter->set_type(wrapper->read<uint16_t>("counter_type"));
	counter->set_count(wrapper->read<uint16_t>("counter_type"));
	
	CardSpawner add_cards_selectable = BIND_FUNC_TO_OBJ_PTR(selectCards, add_cards_selectable);
	InlineCardRead ReadCounterCount = [](Buffer::ibufferw& wrapper, Core::Data::CardInfo* card)
	{
		card->mutable_counter()->set_count(wrapper->read<uint16_t>("counter count"));
	};
	ReadCardVector<cardcount_t, small_location_t, small_sequence_t, void>(wrapper, add_cards_selectable, nullptr, ReadCounterCount);
	
	// TODO: Set right counter type to each card?
#endif // FILTERING
CASE(MSG_SELECT_SUM)
#ifndef FILTERING
	auto selectCards = specific->mutable_request()->mutable_select_cards();
	
	if(wrapper->read<uint8_t>("select_mode"))
		selectCards->set_type(Core::Msg::SelectCards::SELECTION_SUM);
	else
		selectCards->set_type(Core::Msg::SelectCards::SELECTION_SUM_EXACTLY);
	
	selectCards->set_sum(wrapper->read<uint32_t>("acc (sum)"));
	
	selectCards->set_min(wrapper->read<uint32_t>("min"));
	selectCards->set_max(wrapper->read<uint32_t>("max"));
	
	InlineCardRead ReadSumParam = [](Buffer::ibufferw& wrapper, Core::Data::CardInfo* card)
	{
		card->set_sum_param(wrapper->read<uint32_t>("sum param"));
	};
	
	CardSpawner add_cards_must_select = BIND_FUNC_TO_OBJ_PTR(selectCards, add_cards_must_select);
	ReadCardVector<cardcount_t, small_location_t, sequence_t, void>(wrapper, add_cards_must_select, nullptr, ReadSumParam);
	
	CardSpawner add_cards_selectable = BIND_FUNC_TO_OBJ_PTR(selectCards, add_cards_selectable);
	ReadCardVector<cardcount_t, small_location_t, sequence_t, void>(wrapper, add_cards_selectable, nullptr, ReadSumParam);
#endif // FILTERING
CASE(MSG_SELECT_UNSELECT_CARD)
#ifndef FILTERING
	auto selectCards = specific->mutable_request()->mutable_select_cards();
	
	selectCards->set_can_accept(wrapper->read<uint8_t>("buttonok (can_accept)"));
	selectCards->set_can_cancel(wrapper->read<uint8_t>("cancelable"));
	selectCards->set_min(wrapper->read<uint32_t>("min"));
	selectCards->set_max(wrapper->read<uint32_t>("max"));
	
	CardSpawner add_selectable = BIND_FUNC_TO_OBJ_PTR(selectCards, add_cards_selectable);
	ReadCardVector<cardcount_t, small_location_t, sequence_t, position_t>(wrapper, add_selectable);
	
	CardSpawner add_unselectable = BIND_FUNC_TO_OBJ_PTR(selectCards, add_cards_unselectable);
	ReadCardVector<cardcount_t, small_location_t, sequence_t, position_t>(wrapper, add_unselectable);
#endif // FILTERING
CASE(MSG_ROCK_PAPER_SCISSORS)
#ifndef FILTERING
	specific->mutable_request()->set_select_rps(true);
#endif // FILTERING
CASE(MSG_ANNOUNCE_RACE)
#ifndef FILTERING
	auto declareMisc = specific->mutable_request()->mutable_declare_misc();
	declareMisc->set_type(Core::Msg::DeclareMisc::DECLARE_RACE);
	
	declareMisc->set_count(wrapper->read<uint8_t>("count"));
	
	declareMisc->add_available(wrapper->read<uint32_t>("races available"));
#endif // FILTERING
CASE(MSG_ANNOUNCE_ATTRIB)
#ifndef FILTERING
	auto declareMisc = specific->mutable_request()->mutable_declare_misc();
	declareMisc->set_type(Core::Msg::DeclareMisc::DECLARE_ATTRIBUTE);
	
	declareMisc->set_count(wrapper->read<uint8_t>("count"));
	
	declareMisc->add_available(wrapper->read<uint32_t>("attrs available"));
#endif // FILTERING
CASE(MSG_ANNOUNCE_CARD)
#ifndef FILTERING
	auto declareCard = specific->mutable_request()->mutable_declare_card();
	
	auto types = wrapper->read<uint32_t>("card types to declare from");
	
	if(types != 0x7) // monster, spell or trap
	{
#define f(t)do{if(types & t)declareCard->add_opcodes(t);}while(0)
		f(0x1); // TYPE_MONSTER
		f(0x2); // TYPE_SPELL
		f(0x4); // TYPE_TRAP
#undef f
		if(declareCard->opcodes_size() == 2)
		{
			declareCard->add_opcodes(0x40000102); // ISTYPE
			declareCard->add_opcodes(0x40000102); // ISTYPE
			declareCard->add_opcodes(0x40000005); // OR
		}
		else // must be 1
		{
			declareCard->add_opcodes(0x40000102); // ISTYPE
		}
	}
#endif // FILTERING
CASE(MSG_ANNOUNCE_NUMBER)
#ifndef FILTERING
	auto declareMisc = specific->mutable_request()->mutable_declare_misc();
	declareMisc->set_type(Core::Msg::DeclareMisc::DECLARE_NUMBER);
	
	auto count = wrapper->read<uint8_t>("count");
	for(decltype(count) i = 0; i < count; i++)
		declareMisc->add_available(wrapper->read<uint64_t>("number ", (int)i));
#endif // FILTERING
CASE(MSG_ANNOUNCE_CARD_FILTER)
#ifndef FILTERING
	auto declareCard = specific->mutable_request()->mutable_declare_card();
	
	auto count = wrapper->read<uint8_t>("count");
	for(decltype(count) i = 0; i < count; i++)
		declareCard->add_opcodes(wrapper->read<uint64_t>("opcode ", (int)i));
#endif // FILTERING
CASE_FINAL()
#ifdef FILTERING
default: { return false; break; }
#endif // FILTERING

#include "encoder_impl_end.hpp"
