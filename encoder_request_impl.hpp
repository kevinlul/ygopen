#include "encoder_impl_begin.hpp"

CASE(MSG_SELECT_BATTLECMD,
	auto selectCmd = specific->mutable_request()->mutable_select_cmd();
	selectCmd->set_type(Core::Msg::SelectCmd::COMMAND_BATTLE);
	auto Add1 = BIND_FUNC_TO_OBJ_PTR(selectCmd, add_cards_w_effect);
	auto ReadEffectDesc = [](Buffer::ibufferw& w, Core::Data::CardInfo* card)
	{
		ToEffectDesc(w->read<ed_t>("effectdesc"), card->mutable_effect_desc());
	};
	ReadCardVector<count_t, s_loc_t, seq_t>(w, Add1, ReadEffectDesc);
	auto Add2 = BIND_FUNC_TO_OBJ_PTR(selectCmd, add_cards_can_attack);
	auto ReadAtkDirectly = [](Buffer::ibufferw& w, Core::Data::CardInfo* card)
	{
		card->set_can_attack_directly(w->read<uint8_t>("can_attack_directly"));
	};
	ReadCardVector<count_t, s_loc_t, s_seq_t>(w, Add2, ReadAtkDirectly);
	selectCmd->set_able_to_mp2(w->read<uint8_t>("to_mp2"));
	selectCmd->set_able_to_ep(w->read<uint8_t>("to_ep"));
);

CASE(MSG_SELECT_IDLECMD,
	auto selectCmd = specific->mutable_request()->mutable_select_cmd();
	selectCmd->set_type(Core::Msg::SelectCmd::COMMAND_IDLE);
	auto Add1 = BIND_FUNC_TO_OBJ_PTR(selectCmd, add_cards_summonable);
	ReadCardVector<count_t, s_loc_t, seq_t>(w, Add1);
	auto Add2 = BIND_FUNC_TO_OBJ_PTR(selectCmd, add_cards_spsummonable);
	ReadCardVector<count_t, s_loc_t, seq_t>(w, Add2);
	auto Add3 = BIND_FUNC_TO_OBJ_PTR(selectCmd, add_cards_repositionable);
	ReadCardVector<count_t, s_loc_t, s_seq_t>(w, Add3);
	auto Add4 = BIND_FUNC_TO_OBJ_PTR(selectCmd, add_cards_msetable);
	ReadCardVector<count_t, s_loc_t, seq_t>(w, Add4);
	auto Add5 = BIND_FUNC_TO_OBJ_PTR(selectCmd, add_cards_ssetable);
	ReadCardVector<count_t, s_loc_t, seq_t>(w, Add5);
	auto Add6 = BIND_FUNC_TO_OBJ_PTR(selectCmd, add_cards_w_effect);
	auto ReadEffectDesc = [](Buffer::ibufferw& w, Core::Data::CardInfo* card)
	{
		ToEffectDesc(w->read<ed_t>("effectdesc"), card->mutable_effect_desc());
	};
	ReadCardVector<count_t, s_loc_t, seq_t>(w, Add6, ReadEffectDesc);
	selectCmd->set_able_to_bp(w->read<uint8_t>("to_bp"));
	selectCmd->set_able_to_ep(w->read<uint8_t>("to_ep"));
	selectCmd->set_can_shuffle(w->read<uint8_t>("can_shuffle"));
);

CASE(MSG_SELECT_EFFECTYN,
	auto selectYesNo = specific->mutable_request()->mutable_select_yesno();
	auto card = selectYesNo->mutable_card();
	ToCardCode(w->read<code_t>("card code"), card);
	READ_INFO_LOC_CORE(w, card);
	ToEffectDesc(w->read<ed_t>("effectdesc"), card->mutable_effect_desc());
);

CASE(MSG_SELECT_YESNO,
	auto selectYesNo = specific->mutable_request()->mutable_select_yesno();
	ToEffectDesc(w->read<ed_t>("effectdesc"), selectYesNo->mutable_effect());
);

CASE(MSG_SELECT_OPTION,
	auto selectOption = specific->mutable_request()->mutable_select_option();
	auto Add1 = BIND_FUNC_TO_OBJ_PTR(selectOption, add_effects);
	auto count = w->read<s_count_t>("number of effects");
	for(decltype(count) i = 0; i < count; i++)
		ToEffectDesc(w->read<ed_t>("effectdesc", (int)count), Add1());
);

CASE(MSG_SELECT_CARD,
	auto selectCards = specific->mutable_request()->mutable_select_cards();
	selectCards->set_type(Core::Msg::SelectCards::SELECTION_EFFECT);
	selectCards->set_can_cancel(w->read<uint8_t>("can_cancel"));
	selectCards->set_can_accept(false);
	selectCards->set_min(w->read<uint32_t>("min"));
	selectCards->set_max(w->read<uint32_t>("max"));
	auto Add1 = BIND_FUNC_TO_OBJ_PTR(selectCards, add_cards_selectable);
	ReadCardVector<count_t, s_loc_t, seq_t, pos_t>(w, Add1);
);

CASE(MSG_SELECT_CHAIN,
	auto selectToChain = specific->mutable_request()->mutable_select_to_chain();
	auto count = w->read<count_t>("number of chains");
	w->seek(1, Buffer::seek_dir::cur, "spe_count");
	selectToChain->set_forced(w->read<uint8_t>("forced"));
	w->seek(8, Buffer::seek_dir::cur, "hint_timing x 2");
	for(decltype(count) i = 0; i < count; i++)
	{
		w->log("chain card ", (int)i);
		auto chainCard = selectToChain->add_cards_w_effect();
		auto card = chainCard->mutable_card();
		chainCard->set_effect_desc_type(w->read<uint8_t>("EDESC ", (int)i));
		ToCardCode(w->read<code_t>("card code"), card);
		READ_INFO_LOC_CORE(w, card);
		ToEffectDesc(w->read<ed_t>("effectdesc"), card->mutable_effect_desc());
	}
);

CASE(MSG_SELECT_PLACE, // case MSG_SELECT_DISFIELD:
	// NOTE:
	// The available positions are always current player places first
	// and then opponent available places
	auto selectPlaces = specific->mutable_request()->mutable_select_places();
	selectPlaces->set_min(w->read<uint8_t>("count (min)"));
	std::bitset<32> flag(w->read<uint32_t>("flag"));
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
);

CASE(MSG_SELECT_POSITION,
	auto selectPos = specific->mutable_request()->mutable_select_position();
	auto card = selectPos->mutable_card();
	ToCardCode(w->read<code_t>("card code"), card);
	card->set_position(w->read<s_pos_t>("positions"));
);

CASE(MSG_SELECT_TRIBUTE,
	auto selectCards = specific->mutable_request()->mutable_select_cards();
	selectCards->set_type(Core::Msg::SelectCards::SELECTION_TRIBUTE);
	selectCards->set_can_cancel(w->read<uint8_t>("can_cancel"));
	selectCards->set_can_accept(false);
	selectCards->set_min(w->read<uint32_t>("min"));
	selectCards->set_max(w->read<uint32_t>("max"));
	auto Add1 = BIND_FUNC_TO_OBJ_PTR(selectCards, add_cards_selectable);
	auto ReadReleaseParam = [](Buffer::ibufferw& w, Core::Data::CardInfo* card)
	{
		card->set_tribute_count(w->read<uint8_t>("release_param"));
	};
	ReadCardVector<count_t, s_loc_t, seq_t, void>(w, Add1, ReadReleaseParam);
);

CASE(MSG_SORT_CHAIN, // case MSG_SORT_CARD:
	auto sortCards = specific->mutable_request()->mutable_sort_cards();
	auto Add1 = BIND_FUNC_TO_OBJ_PTR(sortCards, add_cards_to_sort);
	ReadCardVector<count_t, s_loc_t, seq_t, pos_t>(w, Add1);
);

CASE(MSG_SELECT_COUNTER,
	auto selectCards = specific->mutable_request()->mutable_select_cards();
	selectCards->set_type(Core::Msg::SelectCards::SELECTION_COUNTER);
	selectCards->set_can_cancel(false);
	selectCards->set_can_accept(false);
	auto counter = selectCards->mutable_counter();
	counter->set_type(w->read<uint16_t>("counter_type"));
	counter->set_count(w->read<uint16_t>("counter_type"));
	auto Add1 = BIND_FUNC_TO_OBJ_PTR(selectCards, add_cards_selectable);
	auto ReadCounterCount = [](Buffer::ibufferw& w, Core::Data::CardInfo* card)
	{
		card->mutable_counter()->set_count(w->read<uint16_t>("counter count"));
	};
	ReadCardVector<count_t, s_loc_t, s_seq_t, void>(w, Add1, ReadCounterCount);
	// TODO: Set right counter type to each card?
);

CASE(MSG_SELECT_SUM,
	auto selectCards = specific->mutable_request()->mutable_select_cards();
	if(w->read<uint8_t>("select_mode"))
		selectCards->set_type(Core::Msg::SelectCards::SELECTION_SUM);
	else
		selectCards->set_type(Core::Msg::SelectCards::SELECTION_SUM_EXACTLY);
	selectCards->set_sum(w->read<uint32_t>("acc (sum)"));
	selectCards->set_min(w->read<uint32_t>("min"));
	selectCards->set_max(w->read<uint32_t>("max"));
	auto ReadSumParam = [](Buffer::ibufferw& w, Core::Data::CardInfo* card)
	{
		card->set_sum_param(w->read<uint32_t>("sum param"));
	};
	auto Add1 = BIND_FUNC_TO_OBJ_PTR(selectCards, add_cards_must_select);
	ReadCardVector<count_t, s_loc_t, seq_t, void>(w, Add1, ReadSumParam);
	auto Add2 = BIND_FUNC_TO_OBJ_PTR(selectCards, add_cards_selectable);
	ReadCardVector<count_t, s_loc_t, seq_t, void>(w, Add2, ReadSumParam);
);

CASE(MSG_SELECT_UNSELECT_CARD,
	auto selectCards = specific->mutable_request()->mutable_select_cards();
	selectCards->set_can_accept(w->read<uint8_t>("buttonok (can_accept)"));
	selectCards->set_can_cancel(w->read<uint8_t>("cancelable"));
	selectCards->set_min(w->read<uint32_t>("min"));
	selectCards->set_max(w->read<uint32_t>("max"));
	auto Add1 = BIND_FUNC_TO_OBJ_PTR(selectCards, add_cards_selectable);
	ReadCardVector<count_t, s_loc_t, seq_t, pos_t>(w, Add1);
	auto Add2 = BIND_FUNC_TO_OBJ_PTR(selectCards, add_cards_unselectable);
	ReadCardVector<count_t, s_loc_t, seq_t, pos_t>(w, Add2);
);

CASE(MSG_ROCK_PAPER_SCISSORS,
	specific->mutable_request()->set_select_rps(true);
);

CASE(MSG_ANNOUNCE_RACE,
	auto declareMisc = specific->mutable_request()->mutable_declare_misc();
	declareMisc->set_type(Core::Msg::DeclareMisc::DECLARE_RACE);
	declareMisc->set_count(w->read<uint8_t>("count"));
	declareMisc->add_available(w->read<uint32_t>("races available"));
);

CASE(MSG_ANNOUNCE_ATTRIB,
	auto declareMisc = specific->mutable_request()->mutable_declare_misc();
	declareMisc->set_type(Core::Msg::DeclareMisc::DECLARE_ATTRIBUTE);
	declareMisc->set_count(w->read<uint8_t>("count"));
	declareMisc->add_available(w->read<uint32_t>("attrs available"));
);

#define f(t)do{if(types & t)declareCard->add_opcodes(t);}while(0)
CASE(MSG_ANNOUNCE_CARD,
	auto declareCard = specific->mutable_request()->mutable_declare_card();
	auto types = w->read<uint32_t>("card types to declare from");
	if(types != 0x7) // monster, spell or trap
	{
		f(0x1); // TYPE_MONSTER
		f(0x2); // TYPE_SPELL
		f(0x4); // TYPE_TRAP
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
);
#undef f

CASE(MSG_ANNOUNCE_NUMBER,
	auto declareMisc = specific->mutable_request()->mutable_declare_misc();
	declareMisc->set_type(Core::Msg::DeclareMisc::DECLARE_NUMBER);
	auto count = w->read<s_count_t>("count");
	for(decltype(count) i = 0; i < count; i++)
		declareMisc->add_available(w->read<uint64_t>("number ", (int)i));
);

CASE(MSG_ANNOUNCE_CARD_FILTER,
	auto declareCard = specific->mutable_request()->mutable_declare_card();
	auto count = w->read<s_count_t>("count");
	for(decltype(count) i = 0; i < count; i++)
		declareCard->add_opcodes(w->read<uint64_t>("opcode ", (int)i));
);

#ifdef FILTERING
default: { return false; break; }
#endif // FILTERING

#include "encoder_impl_end.hpp"
