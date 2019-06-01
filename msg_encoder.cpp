#include <functional>
#include <bitset>


#include "msg_codec.hpp"
#include "buffer.hpp"
#include "core_message.hpp"

namespace YGOpen
{
// Types that represent the right size type for what they hold
using player_t = uint8_t; // used for controller too
using cardcount_t = uint32_t;
using cardcode_t = uint32_t;
using location_t = uint32_t;
using sequence_t = uint32_t;
using position_t = uint32_t;
using effectdesc_t = uint64_t;

using field_cardcount_t = uint8_t;

// Types that should be changed (increased) in far future
using small_location_t = uint8_t;
using small_sequence_t = uint8_t;
using small_position_t = uint8_t;

// NOTE: write info location sizes:
// controller = 1 byte (player_t)
// location = 1 byte (small_location_t)
// sequence = 4 bytes (sequence_t)
// position = 4 bytes (position_t)

// reads a effectdesc_t value and places the right content on the given EffectDesc
void ToEffectDesc(const effectdesc_t ed, Core::Data::EffectDesc* msg)
{
	msg->set_code(ed >> 4);
	msg->set_string_id(ed & 0xF);
}

// reads a card code and sets the right values of a Data.CardInfo message
void ToCardCode(const cardcode_t code, Core::Data::CardInfo* card)
{
	card->set_code(code & 0x7FFFFFFF); // Do not include last bit
	card->set_bit(code & 0x80000000);
}

// Function prototype that returns a CardInfo (newly created most likely).
// The returned card should be inside of a array,
// managed by a GMsg; normally the [gmsg]->add_* functions
using CardSpawner = std::function<Core::Data::CardInfo*()>;

// Binds a function from a object pointer, without needing to write the full object path.
// NOTE: Can be improved but this works right now.
#define BindFromPointer(object, member) std::bind(&std::remove_pointer<decltype(object)>::type::member, object)

// Function prototype that reads data from a given wrapper and places the right content on the given card.
using InlineCardRead = std::function<void(Buffer::ibufferw&, const int, Core::Data::CardInfo*)>;

// Used to tell ReadCardLocInfo to not read a specific value, at compile time
struct do_not_read_t {};

// Reads a single card location info from the given wrapper
template<typename Controller, typename Location, typename Sequence, typename Position>
inline void ReadCardLocInfo(Buffer::ibufferw& wrapper, const int count, Core::Data::CardInfo* card)
{
	// Controller
	if constexpr(!std::is_same<do_not_read_t, Controller>())
		card->set_controller(wrapper->read<Controller>("controller ", count));

	// Location
	if constexpr(!std::is_same<do_not_read_t, Location>())
		card->set_location(wrapper->read<Location>("location ", count));

	// Sequence
	if constexpr(!std::is_same<do_not_read_t, Sequence>())
		card->set_sequence(wrapper->read<Sequence>("sequence ", count));

	// Position
	if constexpr(!std::is_same<do_not_read_t, Position>())
	{
		const auto pos = wrapper->read<Position>("position ", count);
		if(card->location() & 0x80) // if the card is overlay
		{
			card->set_overlay_sequence(pos);
			card->set_position(0x2); // face-up attack
		}
		else
		{
			card->set_position(pos);
		}
	}
}

// Reads a card vector from the given wrapper.
// a CardSpawner must be given, which should give a new card each time it is called
// aditionally, two functions before and after the default card read can be given, in case aditional info should be read
template<typename Count, typename Location, typename Sequence, typename Position = do_not_read_t>
void ReadCardVector(Buffer::ibufferw& wrapper, CardSpawner cs, InlineCardRead bcr = nullptr, InlineCardRead acr = nullptr)
{
	const Count count = wrapper->read<Count>(".size()");
	for(Count i = 0; i < count; i++)
	{
		Core::Data::CardInfo* card = cs();
		
		// Before Card Read
		if(bcr) bcr(wrapper, i, card);

		// Card Code & Dirty Bit
		ToCardCode(wrapper->read<cardcode_t>("card code ", (int)i), card);

		// Location Info
		ReadCardLocInfo<player_t, Location, Sequence, Position>(wrapper, i, card);

		// After Card Read
		if(acr) acr(wrapper, i, card);
	}
}

struct MsgEncoder::impl
{
	Buffer::ibuffer ib{};
	
	bool isMatchKill{};
	uint32_t matchKillCardId{};
};

MsgEncoder::MsgEncoder() : pimpl(new impl())
{}

MsgEncoder::~MsgEncoder() = default;

inline bool MsgEncoder::SpecificRequestMsg(Core::AnyMsg& msg, const int msgType)
{
	bool encoded;
	Buffer::ibufferw wrapper(&pimpl->ib);

	auto specific = msg.mutable_specific();
	specific->set_player(wrapper->read<player_t>("player"));

	switch(msgType)
	{
		case MSG_SELECT_BATTLECMD:
		{
			auto selectCmd = specific->mutable_request()->mutable_select_cmd();
			
			selectCmd->set_type(Core::Msg::SelectCmd::COMMAND_BATTLE);

			CardSpawner add_w_effect = BindFromPointer(selectCmd, add_cards_w_effect);
			InlineCardRead ReadEffectDesc = [](Buffer::ibufferw& wrapper, const int count, Core::Data::CardInfo* card)
			{
				ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc ", count), card->mutable_effect_desc());
			};
			ReadCardVector<cardcount_t, small_location_t, sequence_t>(wrapper, add_w_effect, nullptr, ReadEffectDesc);
			
			CardSpawner add_can_attack = BindFromPointer(selectCmd, add_cards_can_attack);
			InlineCardRead ReadAtkDirectly = [](Buffer::ibufferw& wrapper, const int count, Core::Data::CardInfo* card)
			{
				card->set_can_attack_directly(wrapper->read<uint8_t>("can_attack_directly ", count));
			};
			ReadCardVector<cardcount_t, small_location_t, small_sequence_t>(wrapper, add_can_attack, nullptr, ReadAtkDirectly);
			
			selectCmd->set_able_to_mp2(wrapper->read<uint8_t>("to_mp2"));
			selectCmd->set_able_to_ep(wrapper->read<uint8_t>("to_ep"));
			
			encoded = true;
		}
		break;
		case MSG_SELECT_IDLECMD:
		{
			auto selectCmd = specific->mutable_request()->mutable_select_cmd();

			selectCmd->set_type(Core::Msg::SelectCmd::COMMAND_IDLE);

			CardSpawner add_summonable = BindFromPointer(selectCmd, add_cards_summonable);
			ReadCardVector<cardcount_t, small_location_t, sequence_t>(wrapper, add_summonable);

			CardSpawner add_spsummonable = BindFromPointer(selectCmd, add_cards_spsummonable);
			ReadCardVector<cardcount_t, small_location_t, sequence_t>(wrapper, add_spsummonable);

			CardSpawner add_repositionable = BindFromPointer(selectCmd, add_cards_repositionable);
			ReadCardVector<cardcount_t, small_location_t, small_sequence_t>(wrapper, add_repositionable);

			CardSpawner add_msetable = BindFromPointer(selectCmd, add_cards_msetable);
			ReadCardVector<cardcount_t, small_location_t, sequence_t>(wrapper, add_msetable);

			CardSpawner add_ssetable = BindFromPointer(selectCmd, add_cards_ssetable);
			ReadCardVector<cardcount_t, small_location_t, sequence_t>(wrapper, add_ssetable);

			CardSpawner add_w_effect = BindFromPointer(selectCmd, add_cards_w_effect);
			InlineCardRead ReadEffectDesc = [](Buffer::ibufferw& wrapper, const int count, Core::Data::CardInfo* card)
			{
				ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc ", count), card->mutable_effect_desc());
			};
			ReadCardVector<cardcount_t, small_location_t, sequence_t>(wrapper, add_w_effect, nullptr, ReadEffectDesc);

			selectCmd->set_able_to_bp(wrapper->read<uint8_t>("to_bp"));
			selectCmd->set_able_to_ep(wrapper->read<uint8_t>("to_ep"));
			selectCmd->set_can_shuffle(wrapper->read<uint8_t>("can_shuffle"));
			
			encoded = true;
		}
		break;
		case MSG_SELECT_EFFECTYN:
		{
			auto selectYesNo = specific->mutable_request()->mutable_select_yesno();

			auto card = selectYesNo->mutable_card();
			ToCardCode(wrapper->read<cardcode_t>("card code"), card);
			ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, 0, card);
			ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc"), card->mutable_effect_desc());
		}
		break;
		case MSG_SELECT_YESNO:
		{
			auto selectYesNo = specific->mutable_request()->mutable_select_yesno();

			ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc"), selectYesNo->mutable_effect());
			
			encoded = true;
		}
		break;
		case MSG_SELECT_OPTION:
		{
			auto selectOption = specific->mutable_request()->mutable_select_option();

			const auto count = wrapper->read<uint8_t>("number of effects");
			for(int i = 0; i < count; i++)
				ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc ", (int)count), selectOption->add_effects());
			
			encoded = true;
		}
		break;
		case MSG_SELECT_CARD:
		{
			auto selectCards = specific->mutable_request()->mutable_select_cards();

			selectCards->set_type(Core::Msg::SelectCards::SELECTION_EFFECT);

			selectCards->set_can_cancel(wrapper->read<uint8_t>("can_cancel"));
			selectCards->set_can_accept(false);

			selectCards->set_min(wrapper->read<uint32_t>("min"));
			selectCards->set_max(wrapper->read<uint32_t>("max"));

			CardSpawner add_selectable = BindFromPointer(selectCards, add_cards_selectable);
			ReadCardVector<cardcount_t, small_location_t, sequence_t, position_t>(wrapper, add_selectable);
			
			encoded = true;
		}
		break;
		case MSG_SELECT_CHAIN:
		{
			auto selectToChain = specific->mutable_request()->mutable_select_to_chain();

			const auto count = wrapper->read<uint32_t>("number of chains");

			wrapper->seek(1, Buffer::seek_dir::cur, "spe_count");

			selectToChain->set_forced(wrapper->read<uint8_t>("forced"));

			wrapper->seek(8, Buffer::seek_dir::cur, "hint_timing x 2");

			// I use a normal loop to collect info for this one
			// NOTE: can probably be improved upon but i cannot think
			// of a better way of doing it right now
			for(int i = 0; i < count; i++)
			{
				auto chainCard = selectToChain->add_cards_w_effect();

				chainCard->set_effect_desc_type(wrapper->read<uint8_t>("EDESC ", (int)i));
				auto card = chainCard->mutable_card();

				ToCardCode(wrapper->read<cardcode_t>("card code ", (int)i), card);
				ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, i, card);
				ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc ", (int)i), card->mutable_effect_desc());
			}
			
			encoded = true;
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

			auto ExtractPlaces = [&selectPlaces, &flag](const int player, const int indexStart)
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
			
			encoded = true;
		}
		break;
		case MSG_SELECT_POSITION:
		{
			auto selectPosition = specific->mutable_request()->mutable_select_position();
			
			auto card = selectPosition->mutable_card();
			
			ToCardCode(wrapper->read<cardcode_t>("card code"), card);
			card->set_position(wrapper->read<small_position_t>("positions"));
			
			encoded = true;
		}
		break;
		case MSG_SELECT_TRIBUTE:
		{
			auto selectCards = specific->mutable_request()->mutable_select_cards();

			selectCards->set_type(Core::Msg::SelectCards::SELECTION_TRIBUTE);

			selectCards->set_can_cancel(wrapper->read<uint8_t>("can_cancel"));
			selectCards->set_can_accept(false);

			selectCards->set_min(wrapper->read<uint32_t>("min"));
			selectCards->set_max(wrapper->read<uint32_t>("max"));

			CardSpawner add_selectable = BindFromPointer(selectCards, add_cards_selectable);
			InlineCardRead ReadReleaseParam = [](Buffer::ibufferw& wrapper, const int count, Core::Data::CardInfo* card)
			{
				card->set_tribute_count(wrapper->read<uint8_t>("release_param ", count));
			};
			ReadCardVector<cardcount_t, small_location_t, sequence_t, do_not_read_t>(wrapper, add_selectable, nullptr, ReadReleaseParam);
			
			encoded = true;
		}
		break;
		// case MSG_SORT_CARD:
		case MSG_SORT_CHAIN:
		{
			auto sortCards = specific->mutable_request()->mutable_sort_cards();
			CardSpawner add_cards_to_sort = BindFromPointer(sortCards, add_cards_to_sort);
			ReadCardVector<cardcount_t, small_location_t, sequence_t, position_t>(wrapper, add_cards_to_sort);
			
			encoded = true;
		}
		break;
		case MSG_SELECT_COUNTER:
		{
			auto selectCards = specific->mutable_request()->mutable_select_cards();

			selectCards->set_type(Core::Msg::SelectCards::SELECTION_COUNTER);

			selectCards->set_can_cancel(false);
			selectCards->set_can_accept(false);
			
			auto counter = selectCards->mutable_counter();
			counter->set_type(wrapper->read<uint16_t>("counter_type"));
			counter->set_count(wrapper->read<uint16_t>("counter_type"));
			
			CardSpawner add_cards_selectable = BindFromPointer(selectCards, add_cards_selectable);
			InlineCardRead ReadCounterCount = [](Buffer::ibufferw& wrapper, const int count, Core::Data::CardInfo* card)
			{
				card->mutable_counter()->set_count(wrapper->read<uint16_t>("counter count ", count));
			};
			ReadCardVector<uint8_t /*TODO: update*/, small_location_t, small_sequence_t, do_not_read_t>(wrapper, add_cards_selectable, nullptr, ReadCounterCount);
			
			// TODO: Set right counter type to each card?
			
			encoded = true;
		}
		break;
		case MSG_SELECT_SUM:
		{
			// TODO: fix select_mode and player being reversed
			// TODO: figure out what select_mode is for.
			auto selectCards = specific->mutable_request()->mutable_select_cards();
			
			selectCards->set_type(Core::Msg::SelectCards::SELECTION_SUM);
			
			wrapper->seek(1, Buffer::seek_dir::cur, "player OR select_mode");
			
			selectCards->set_sum(wrapper->read<uint32_t>("acc (sum)"));
			
			selectCards->set_min(wrapper->read<uint32_t>("min"));
			selectCards->set_max(wrapper->read<uint32_t>("max"));
			
			InlineCardRead ReadSumParam = [](Buffer::ibufferw& wrapper, const int count, Core::Data::CardInfo* card)
			{
				card->set_sum_param(wrapper->read<uint32_t>("sum param ", count));
			};
			
			CardSpawner add_cards_must_select = BindFromPointer(selectCards, add_cards_must_select);
			ReadCardVector<cardcount_t, small_location_t, sequence_t, do_not_read_t>(wrapper, add_cards_must_select, nullptr, ReadSumParam);
			
			CardSpawner add_cards_selectable = BindFromPointer(selectCards, add_cards_selectable);
			ReadCardVector<cardcount_t, small_location_t, sequence_t, do_not_read_t>(wrapper, add_cards_selectable, nullptr, ReadSumParam);
			
			encoded = true;
		}
		break;
		case MSG_SELECT_UNSELECT_CARD:
		{
			auto selectCards = specific->mutable_request()->mutable_select_cards();
			
			selectCards->set_can_accept(wrapper->read<uint8_t>("buttonok (can_accept)"));
			selectCards->set_can_cancel(wrapper->read<uint8_t>("cancelable"));
			selectCards->set_min(wrapper->read<uint32_t>("min"));
			selectCards->set_max(wrapper->read<uint32_t>("max"));
			
			CardSpawner add_selectable = BindFromPointer(selectCards, add_cards_selectable);
			ReadCardVector<cardcount_t, small_location_t, sequence_t, position_t>(wrapper, add_selectable);
			
			CardSpawner add_unselectable = BindFromPointer(selectCards, add_cards_unselectable);
			ReadCardVector<cardcount_t, small_location_t, sequence_t, position_t>(wrapper, add_unselectable);
			
			encoded = true;
		}
		break;
		case MSG_ROCK_PAPER_SCISSORS:
		{
			specific->mutable_request()->set_select_rps(true);
			encoded = true;
		}
		break;
		case MSG_ANNOUNCE_RACE:
		{
			auto declareMisc = specific->mutable_request()->mutable_declare_misc();
			declareMisc->set_type(Core::Msg::DeclareMisc::DECLARE_RACE);
			
			declareMisc->set_count(wrapper->read<uint8_t>("count"));
			
			declareMisc->add_available(wrapper->read<uint32_t>("races available"));
			
			encoded = true;
		}
		break;
		case MSG_ANNOUNCE_ATTRIB:
		{
			auto declareMisc = specific->mutable_request()->mutable_declare_misc();
			declareMisc->set_type(Core::Msg::DeclareMisc::DECLARE_ATTRIBUTE);
			
			declareMisc->set_count(wrapper->read<uint8_t>("count"));
			
			declareMisc->add_available(wrapper->read<uint32_t>("attrs available"));
			
			encoded = true;
		}
		break;
		case MSG_ANNOUNCE_CARD:
		{
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
			
			encoded = true;
		}
		break;
		case MSG_ANNOUNCE_NUMBER:
		{
			auto declareMisc = specific->mutable_request()->mutable_declare_misc();
			declareMisc->set_type(Core::Msg::DeclareMisc::DECLARE_NUMBER);
			
			auto count = wrapper->read<uint8_t>("count");
			for(decltype(count) i = 0; i < count; i++)
				declareMisc->add_available(wrapper->read<uint64_t>("number ", (int)i));
			
			encoded = true;
		}
		break;
		case MSG_ANNOUNCE_CARD_FILTER:
		{
			auto declareCard = specific->mutable_request()->mutable_declare_card();
			
			auto count = wrapper->read<uint8_t>("count");
			for(decltype(count) i = 0; i < count; i++)
				declareCard->add_opcodes(wrapper->read<uint64_t>("opcode ", (int)i));
			
			encoded = true;
		}
		break;
	}
	
	return encoded;
}

inline bool MsgEncoder::SpecificInformationMsg(Core::AnyMsg& msg, const int msgType)
{
	bool encoded;
	
	Buffer::ibufferw wrapper(&pimpl->ib);
	
	auto specific = msg.mutable_specific();
	
	switch(msgType)
	{
		case MSG_HINT:
		{
			auto hint = specific->mutable_information()->mutable_hint();
			
			const int type = wrapper->read<uint8_t>("hint type");
			hint->set_type(type);
			
			specific->set_player(wrapper->read<player_t>("player"));
			
			hint->set_data(wrapper->read<uint64_t>("hint data"));
			// TODO: handle each hint
			encoded = true;
		}
		break;
		case MSG_MISSED_EFFECT:
		{
			auto miscAction = specific->mutable_information()->mutable_misc_action();
			miscAction->set_type(Core::Msg::MiscAction::ACTION_MISSED_TIMING);
			
			auto card = miscAction->mutable_card();
			
			ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, 1, card);
			ToCardCode(wrapper->read<cardcode_t>("card code 1"), card);
			
			encoded = true;
		}
		break;
	}
	
	return encoded;
}

inline bool MsgEncoder::InformationMsg(Core::AnyMsg& msg, const int msgType)
{
	bool encoded;
	
	Buffer::ibufferw wrapper{&pimpl->ib};
	
	auto information = msg.mutable_information();
	
	switch(msgType)
	{
		case MSG_WIN:
		{
			auto win = information->mutable_win();
			
			win->set_player(wrapper->read<player_t>("player"));
			win->set_reason(wrapper->read<uint8_t>("reason"));
			
			// this information is set when reading MSG_MATCH_KILL
			win->set_is_match_kill(pimpl->isMatchKill);
			if(pimpl->isMatchKill)
				win->set_code(pimpl->matchKillCardId);
			
			encoded = true;
		}
		break;
		case MSG_CONFIRM_DECKTOP:
		{
			auto confirmCards = information->mutable_confirm_cards();
			
			confirmCards->set_type(Core::Msg::ConfirmCards::CONFIRM_DECKTOP);
			
			wrapper->seek(1, Buffer::seek_dir::cur, "player");
			
			CardSpawner add_cards = BindFromPointer(confirmCards, add_cards);
			ReadCardVector<cardcount_t, small_location_t, small_sequence_t>(wrapper, add_cards);
			
			encoded = true;
		}
		break;
		case MSG_CONFIRM_CARDS:
		{
			auto confirmCards = information->mutable_confirm_cards();
			
			confirmCards->set_type(Core::Msg::ConfirmCards::CONFIRM_CARDS);
			
			wrapper->seek(1, Buffer::seek_dir::cur, "player");
			
			CardSpawner add_cards = BindFromPointer(confirmCards, add_cards);
			ReadCardVector<cardcount_t, small_location_t, sequence_t>(wrapper, add_cards);
			
			encoded = true;
		}
		break;
		case MSG_SHUFFLE_DECK:
		{
			auto shuffleLocation = information->mutable_shuffle_location();
			
			shuffleLocation->set_player(wrapper->read<player_t>("player"));
			
			shuffleLocation->set_location(0x01); // LOCATION_DECK
			
			// NOTE: empty decklist
			
			encoded = true;
		}
		break;
		case MSG_SHUFFLE_HAND:
		{
			auto shuffleLocation = information->mutable_shuffle_location();
			
			shuffleLocation->set_player(wrapper->read<player_t>("player"));
			
			shuffleLocation->set_location(0x02); // LOCATION_HAND
			
			auto count = wrapper->read<cardcount_t>(".size()");
			CardSpawner add_shuffled_cards = BindFromPointer(shuffleLocation, add_shuffled_cards);
			for(decltype(count) i = 0; i < count; i++)
			{
				Core::Data::CardInfo* card = add_shuffled_cards();
				ToCardCode(wrapper->read<cardcode_t>("card code ", (int)i), card);
				card->set_location(0x02); // LOCATION_HAND
			}
			
			encoded = true;
		}
		break;
		case MSG_SWAP_GRAVE_DECK:
		{
			auto miscAction = information->mutable_misc_action();
			
			miscAction->set_type(Core::Msg::MiscAction::ACTION_SWAP_GY_WITH_DECK);
			
			miscAction->set_player(wrapper->read<player_t>("player"));
			
			encoded = true;
		}
		break;
		case MSG_SHUFFLE_SET_CARD:
		{
			auto shuffleSetCards = information->mutable_shuffle_set_cards();
			wrapper->seek(1, Buffer::seek_dir::cur, "location");
			
			auto count = wrapper->read<field_cardcount_t>(".size()");
			CardSpawner add_cards_previous = BindFromPointer(shuffleSetCards, add_cards_previous);
			CardSpawner add_cards_current = BindFromPointer(shuffleSetCards, add_cards_current);
			for(decltype(count) i = 0; i < count; i++)
				ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, i, add_cards_previous());
			for(decltype(count) i = 0; i < count; i++)
				ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, i, add_cards_current());
			
			encoded = true;
		}
		break;
		case MSG_REVERSE_DECK:
		{
			auto miscAction = information->mutable_misc_action();
			
			miscAction->set_type(Core::Msg::MiscAction::ACTION_REVERSE_DECK);
			
			encoded = true;
		}
		break;
		case MSG_DECK_TOP:
		{
			auto updateCard = information->mutable_update_card();
			
			updateCard->set_reason(Core::Msg::UpdateCard::REASON_DECK_TOP);
			
			auto previous = updateCard->mutable_previous();
			previous->set_controller(wrapper->read<player_t>("player"));
			previous->set_sequence(wrapper->read<small_sequence_t>("sequence"));
			previous->set_location(0x01); // LOCATION_DECK
			
			auto current = updateCard->mutable_current();
			ToCardCode(wrapper->read<cardcode_t>("card code"), current);
			// NOTE: maybe move the dirty bit to the position parameter?
			
			encoded = true;
		}
		break;
		case MSG_SHUFFLE_EXTRA:
		{
			auto shuffleLocation = information->mutable_shuffle_location();
			
			shuffleLocation->set_player(wrapper->read<player_t>("player"));
			
			shuffleLocation->set_location(0x40); // LOCATION_EXTRA
			
			auto count = wrapper->read<cardcount_t>(".size()");
			CardSpawner add_shuffled_cards = BindFromPointer(shuffleLocation, add_shuffled_cards);
			for(decltype(count) i = 0; i < count; i++)
			{
				Core::Data::CardInfo* card = add_shuffled_cards();
				ToCardCode(wrapper->read<cardcode_t>("card code ", (int)i), card);
				card->set_location(0x40); // LOCATION_EXTRA
			}
			
			encoded = true;
		}
		break;
		case MSG_NEW_TURN:
		{
			auto newTurn = information->mutable_new_turn();
			
			newTurn->set_turn_player(wrapper->read<player_t>("turn player"));
			
			encoded = true;
		}
		break;
		case MSG_NEW_PHASE:
		{
			auto newPhase = information->mutable_new_phase();
			
			newPhase->set_phase(wrapper->read<uint16_t>("phase"));
			
			encoded = true;
		}
		break;
		case MSG_CONFIRM_EXTRATOP:
		{
			auto confirmCards = information->mutable_confirm_cards();
			
			confirmCards->set_type(Core::Msg::ConfirmCards::CONFIRM_EXTRATOP);
			
			wrapper->seek(1, Buffer::seek_dir::cur, "player");
			
			CardSpawner add_cards = BindFromPointer(confirmCards, add_cards);
			ReadCardVector<cardcount_t, small_location_t, small_sequence_t>(wrapper, add_cards);
			
			encoded = true;
		}
		break;
		case MSG_MOVE:
		{
			auto updateCard = information->mutable_update_card();
			
			updateCard->set_reason(Core::Msg::UpdateCard::REASON_MOVE);
			
			const auto cardCode = wrapper->read<cardcode_t>("card code");
			
			auto ReadCardInfo = [&wrapper, cardCode](Core::Data::CardInfo* card, const int val)
			{
				ToCardCode(cardCode, card);
				ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, val, card);
			};
			
			ReadCardInfo(updateCard->mutable_previous(), 0);
			ReadCardInfo(updateCard->mutable_current(), 1);
			
			updateCard->set_core_reason(wrapper->read<uint32_t>("core reason"));
			
			encoded = true;
		}
		break;
		case MSG_POS_CHANGE:
		{
			auto updateCard = information->mutable_update_card();
			
			updateCard->set_reason(Core::Msg::UpdateCard::REASON_POS_CHANGE);
			
			auto previous = updateCard->mutable_previous();
			auto current = updateCard->mutable_current();
			
			ToCardCode(wrapper->read<cardcode_t>("cardcode"), current);
			
			previous->set_controller(wrapper->read<player_t>("controller"));
			previous->set_location(wrapper->read<small_location_t>("location"));
			previous->set_sequence(wrapper->read<small_sequence_t>("sequence"));
			previous->set_position(wrapper->read<small_position_t>("position"));
			current->set_position(wrapper->read<small_position_t>("current position"));
			
			encoded = true;
		}
		break;
		case MSG_SET:
		{
			auto updateCard = information->mutable_update_card();
			
			updateCard->set_reason(Core::Msg::UpdateCard::REASON_SET);
			
			auto previous = updateCard->mutable_previous();
			auto current = updateCard->mutable_current();
			
			ToCardCode(wrapper->read<cardcode_t>("cardcode"), current);
			
			previous->set_controller(wrapper->read<player_t>("controller"));
			previous->set_location(wrapper->read<small_location_t>("location"));
			previous->set_sequence(wrapper->read<sequence_t>("sequence"));
			current->set_position(wrapper->read<position_t>("current position"));
			
			encoded = true;
		}
		break;
		case MSG_SWAP:
		{
			auto swapCards = information->mutable_swap_cards();
			
			auto ReadCardInfo = [&wrapper](Core::Data::CardInfo* card, const int val)
			{
				ToCardCode(wrapper->read<cardcode_t>("cardcode"), card);
				ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, val, card);
			};
			
			ReadCardInfo(swapCards->mutable_card1(), 0);
			ReadCardInfo(swapCards->mutable_card2(), 1);
			
			encoded = true;
		}
		break;
		case MSG_FIELD_DISABLED:
		{
			auto disableZones = information->mutable_disable_zones();
			
			std::bitset<32> flag(wrapper->read<uint32_t>("flag"));

			// NOTE: This one is different from the one used to handle MSG_SELECT_PLACE.
			// The flag checkings are reversed.
			auto ExtractPlaces = [&disableZones, &flag](const int player, const int indexStart)
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
			
			encoded = true;
		}
		break;
		case MSG_SUMMONING:
		{
			auto summonCard = information->mutable_summon_card();
			
			summonCard->set_type(Core::Msg::SummonCard::SUMMON_NORMAL);
			
			auto card = summonCard->mutable_card();
			ToCardCode(wrapper->read<cardcode_t>("cardcode"), card);
			ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, 0, card);
			
			encoded = true;
		}
		break;
		case MSG_SUMMONED:
		{
			auto summonCard = information->mutable_summon_card();
			
			summonCard->set_type(Core::Msg::SummonCard::SUMMON_NORMAL);
			
			encoded = true;
		}
		break;
		case MSG_SPSUMMONING:
		{
			auto summonCard = information->mutable_summon_card();
			
			summonCard->set_type(Core::Msg::SummonCard::SUMMON_SPECIAL);
			
			auto card = summonCard->mutable_card();
			ToCardCode(wrapper->read<cardcode_t>("cardcode"), card);
			ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, 0, card);
			
			encoded = true;
		}
		break;
		case MSG_SPSUMMONED:
		{
			auto summonCard = information->mutable_summon_card();
			
			summonCard->set_type(Core::Msg::SummonCard::SUMMON_SPECIAL);
			
			encoded = true;
		}
		break;
		case MSG_FLIPSUMMONING:
		{
			auto summonCard = information->mutable_summon_card();
			
			summonCard->set_type(Core::Msg::SummonCard::SUMMON_FLIP);
			
			auto card = summonCard->mutable_card();
			ToCardCode(wrapper->read<cardcode_t>("cardcode"), card);
			ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, 0, card);
			
			encoded = true;
		}
		break;
		case MSG_FLIPSUMMONED:
		{
			auto summonCard = information->mutable_summon_card();
			
			summonCard->set_type(Core::Msg::SummonCard::SUMMON_FLIP);
			
			encoded = true;
		}
		break;
		case MSG_CHAINING:
		{
			auto chainAction = information->mutable_chain_action();
			chainAction->set_type(Core::Msg::ChainAction::ACTION_CHAINING);
			
			auto card = chainAction->mutable_card();
			ToCardCode(wrapper->read<cardcode_t>("cardcode"), card);
			ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, 0, card);
			
			auto place = chainAction->mutable_place();
			place->set_controller(wrapper->read<player_t>("player"));
			place->set_location(wrapper->read<small_location_t>("location"));
			place->set_sequence(wrapper->read<sequence_t>("sequence"));
			
			ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc"), chainAction->mutable_ed());
			
			chainAction->set_chain_number(wrapper->read<uint32_t>("chain num"));
			
			encoded = true;
		}
		break;
		case MSG_CHAINED:
		{
			auto chainAction = information->mutable_chain_action();
			chainAction->set_type(Core::Msg::ChainAction::ACTION_CHAINED);
			chainAction->set_chain_number(wrapper->read<uint8_t>("chain num"));
			
			encoded = true;
		}
		break;
		case MSG_CHAIN_SOLVING:
		{
			auto chainAction = information->mutable_chain_action();
			chainAction->set_type(Core::Msg::ChainAction::ACTION_CHAIN_SOLVING);
			chainAction->set_chain_number(wrapper->read<uint8_t>("chain num"));
			
			encoded = true;
		}
		break;
		case MSG_CHAIN_SOLVED:
		{
			auto chainAction = information->mutable_chain_action();
			chainAction->set_type(Core::Msg::ChainAction::ACTION_CHAIN_SOLVED);
			chainAction->set_chain_number(wrapper->read<uint8_t>("chain num"));
			
			encoded = true;
		}
		break;
		case MSG_CHAIN_END:
		{
			auto chainAction = information->mutable_chain_action();
			chainAction->set_type(Core::Msg::ChainAction::ACTION_CHAIN_END);
			
			encoded = true;
		}
		break;
		case MSG_CHAIN_NEGATED:
		{
			auto chainAction = information->mutable_chain_action();
			chainAction->set_type(Core::Msg::ChainAction::ACTION_CHAIN_NEGATED);
			chainAction->set_chain_number(wrapper->read<uint8_t>("chain num"));
			
			encoded = true;
		}
		break;
		case MSG_CHAIN_DISABLED:
		{
			auto chainAction = information->mutable_chain_action();
			chainAction->set_type(Core::Msg::ChainAction::ACTION_CHAIN_DISABLED);
			chainAction->set_chain_number(wrapper->read<uint8_t>("chain num"));
			
			encoded = true;
		}
		break;
		case MSG_RANDOM_SELECTED:
		{
			auto selectedCards = information->mutable_selected_cards();
			selectedCards->set_type(Core::Msg::SelectedCards::SELECTION_RANDOM);
			
			wrapper->seek(1, Buffer::seek_dir::cur, "player");
			
			auto count = wrapper->read<cardcount_t>("count");
			for(decltype(count) i = 0; i < count; i++)
				ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, i, selectedCards->add_cards());
			
			encoded = true;
		}
		break;
		case MSG_BECOME_TARGET:
		{
			auto selectedCards = information->mutable_selected_cards();
			selectedCards->set_type(Core::Msg::SelectedCards::SELECTION_BECOME);
			
			auto count = wrapper->read<cardcount_t>("count");
			for(decltype(count) i = 0; i < count; i++)
				ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, i, selectedCards->add_cards());
			
			encoded = true;
		}
		break;
		case MSG_DRAW:
		{
			auto draw = information->mutable_draw();
			draw->set_player(wrapper->read<player_t>("player"));
			
			auto count = wrapper->read<cardcount_t>("count");
			for(decltype(count) i = 0; i < count; i++)
				ToCardCode(wrapper->read<cardcode_t>("cardcode", (int)i), draw->add_cards());
			
			encoded = true;
		}
		break;
		case MSG_DAMAGE:
		{
			auto lpChange = information->mutable_lp_change();
			lpChange->set_type(Core::Msg::LpChange::CHANGE_DAMAGE);
			lpChange->set_player(wrapper->read<player_t>("player"));
			lpChange->set_amount(wrapper->read<uint32_t>("amount"));
			
			encoded = true;
		}
		break;
		case MSG_RECOVER:
		{
			auto lpChange = information->mutable_lp_change();
			lpChange->set_type(Core::Msg::LpChange::CHANGE_RECOVER);
			lpChange->set_player(wrapper->read<player_t>("player"));
			lpChange->set_amount(wrapper->read<uint32_t>("amount"));
			
			encoded = true;
		}
		break;
		case MSG_LPUPDATE:
		{
			auto lpChange = information->mutable_lp_change();
			lpChange->set_type(Core::Msg::LpChange::CHANGE_BECOME);
			lpChange->set_player(wrapper->read<player_t>("player"));
			lpChange->set_amount(wrapper->read<uint32_t>("amount"));
			
			encoded = true;
		}
		break;
		case MSG_PAY_LPCOST:
		{
			auto lpChange = information->mutable_lp_change();
			lpChange->set_type(Core::Msg::LpChange::CHANGE_PAY);
			lpChange->set_player(wrapper->read<player_t>("player"));
			lpChange->set_amount(wrapper->read<uint32_t>("amount"));
			
			encoded = true;
		}
		break;
		case MSG_CARD_TARGET:
		{
			auto selectedCards = information->mutable_selected_cards();
			selectedCards->set_type(Core::Msg::SelectedCards::SELECTION_CARD_TARGET);
			
			ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, 0, selectedCards->mutable_targeting_card());
			ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, 1, selectedCards->add_cards());
			
			encoded = true;
		}
		break;
		case MSG_CANCEL_TARGET:
		{
			auto selectedCards = information->mutable_selected_cards();
			selectedCards->set_type(Core::Msg::SelectedCards::SELECTION_CARD_DETARGET);
			
			ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, 0, selectedCards->mutable_targeting_card());
			ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, 1, selectedCards->add_cards());
			
			encoded = true;
		}
		break;
		case MSG_ADD_COUNTER:
		{
			auto counterChange = information->mutable_counter_change();
			counterChange->set_type(Core::Msg::CounterChange::CHANGE_ADD);
			
			auto counter = counterChange->mutable_counter();
			auto place = counterChange->mutable_place();
			counter->set_type(wrapper->read<uint16_t>("counter type"));
			place->set_controller(wrapper->read<player_t>("player"));
			place->set_location(wrapper->read<small_location_t>("location"));
			place->set_sequence(wrapper->read<small_sequence_t>("sequence"));
			counter->set_count(wrapper->read<uint16_t>("counter count"));
			
			encoded = true;
		}
		break;
		case MSG_REMOVE_COUNTER:
		{
			auto counterChange = information->mutable_counter_change();
			counterChange->set_type(Core::Msg::CounterChange::CHANGE_REMOVE);
			
			auto counter = counterChange->mutable_counter();
			auto place = counterChange->mutable_place();
			counter->set_type(wrapper->read<uint16_t>("counter type"));
			place->set_controller(wrapper->read<player_t>("player"));
			place->set_location(wrapper->read<small_location_t>("location"));
			place->set_sequence(wrapper->read<small_sequence_t>("sequence"));
			counter->set_count(wrapper->read<uint16_t>("counter count"));
			
			encoded = true;
		}
		break;
		case MSG_ATTACK:
		{
			auto onAttack = information->mutable_on_attack();
			onAttack->set_status(Core::Msg::OnAttack::STATUS_ATTACK_DECLARATION);
			
			ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, 0, onAttack->mutable_attacker());
			
			auto atkTarget = onAttack->mutable_attack_target();
			ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, 1, atkTarget);
			
			// clear card if location is empty
			if(atkTarget->location() == 0 && atkTarget->sequence() == 0)
				onAttack->clear_attack_target();
			
			encoded = true;
		}
		break;
		case MSG_BATTLE:
		{
			auto onAttack = information->mutable_on_attack();
			onAttack->set_status(Core::Msg::OnAttack::STATUS_ATTACK_DECLARATION);
			
			auto attacker = onAttack->mutable_attacker();
			auto atkTarget = onAttack->mutable_attack_target();
			
			ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, 0, attacker);
			attacker->set_atk(wrapper->read<uint32_t>("attacker atk"));
			attacker->set_def(wrapper->read<uint32_t>("attacker def"));
			wrapper->seek(1, Buffer::seek_dir::cur, "attacker battle destroyed");
			
			ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, 1, atkTarget);
			atkTarget->set_atk(wrapper->read<uint32_t>("attack_target atk"));
			atkTarget->set_def(wrapper->read<uint32_t>("attack_target def"));
			wrapper->seek(1, Buffer::seek_dir::cur, "attack_target battle destroyed");
			
			// clear card if location is empty
			if(atkTarget->location() == 0 && atkTarget->sequence() == 0)
				onAttack->clear_attack_target();
			
			encoded = true;
		}
		break;
		case MSG_ATTACK_DISABLED:
		{
			auto miscAction = information->mutable_misc_action();
			miscAction->set_type(Core::Msg::MiscAction::ACTION_ATTACK_NEGATED);
			
			encoded = true;
		}
		break;
		case MSG_DAMAGE_STEP_START:
		{
			auto miscAction = information->mutable_misc_action();
			miscAction->set_type(Core::Msg::MiscAction::ACTION_DAMAGE_STEP_START);
			
			encoded = true;
		}
		break;
		case MSG_DAMAGE_STEP_END:
		{
			auto miscAction = information->mutable_misc_action();
			miscAction->set_type(Core::Msg::MiscAction::ACTION_DAMAGE_STEP_END);
			
			encoded = true;
		}
		break;
		case MSG_TOSS_COIN:
		{
			auto result = information->mutable_result();
			result->set_type(Core::Msg::Result::RESULT_TOSS_COIN);
			
			result->set_player(wrapper->read<player_t>("player"));
			
			auto count = wrapper->read<uint8_t>("count");
			for(decltype(count) i = 0; i < count; i++)
				result->add_results(wrapper->read<uint8_t>("result ", (int)i));
			
			encoded = true;
		}
		break;
		case MSG_TOSS_DICE:
		{
			auto result = information->mutable_result();
			result->set_type(Core::Msg::Result::RESULT_TOSS_DICE);
			
			result->set_player(wrapper->read<player_t>("player"));
			
			auto count = wrapper->read<uint8_t>("count");
			for(decltype(count) i = 0; i < count; i++)
				result->add_results(wrapper->read<uint8_t>("result ", (int)i));
			
			encoded = true;
		}
		break;
		case MSG_HAND_RES:
		{
			auto result = information->mutable_result();
			result->set_type(Core::Msg::Result::RESULT_RPS);
			
			auto handResults = wrapper->read<uint8_t>("hand results");
			result->add_results((handResults & 0x3) - 1);
			result->add_results(((handResults >> 2) & 0x3) - 1);
			
			encoded = true;
		}
		break;
		case MSG_CARD_HINT:
		{
			auto cardHint = information->mutable_card_hint();
			
			auto card = cardHint->mutable_card();
			ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, 1, card);
			
			cardHint->set_type(wrapper->read<uint8_t>("hint type"));
			cardHint->set_data(wrapper->read<uint64_t>("hint data"));
			
			encoded = true;
		}
		break;
		case MSG_PLAYER_HINT:
		{
			auto playerHint = information->mutable_player_hint();
			
			playerHint->set_player(wrapper->read<player_t>("player"));
			playerHint->set_type(wrapper->read<uint8_t>("hint type"));
			playerHint->set_data(wrapper->read<uint64_t>("hint data"));
			
			encoded = true;
		}
		break;
		case MSG_TAG_SWAP:
		{
			auto swapPlayer = information->mutable_swap_player();
			
			swapPlayer->set_player(wrapper->read<player_t>("player"));
			swapPlayer->set_main_deck_count(wrapper->read<cardcount_t>("main deck count"));
			
			auto extraCount = wrapper->read<cardcount_t>("extra deck count");
			wrapper->seek(sizeof(cardcount_t), Buffer::seek_dir::cur, "extra_p_count (face-up pendulums)");
			auto handCount = wrapper->read<cardcount_t>("hand deck count");
			
			ToCardCode(wrapper->read<cardcode_t>("top of deck"), swapPlayer->mutable_top_of_deck());
			
			for(decltype(handCount) i = 0; i < handCount; i++)
				ToCardCode(wrapper->read<cardcode_t>("hand card ", (int)i), swapPlayer->add_hand_cards());
			
			for(decltype(extraCount) i = 0; i < extraCount; i++)
				ToCardCode(wrapper->read<cardcode_t>("extra card ", (int)i), swapPlayer->add_extra_cards());
			
			encoded = true;
		}
		break;
		case MSG_MATCH_KILL:
		{
			pimpl->isMatchKill = true;
			pimpl->matchKillCardId = wrapper->read<cardcode_t>("card code");
			
			encoded = false;
		}
		break;
	}
	
	return encoded;
}

Core::AnyMsg MsgEncoder::Encode(void* buffer, size_t length, bool& encoded)
{
	Core::AnyMsg msg{};

	pimpl->ib.open(buffer, length);

	pimpl->ib.log("Message Type: ", (int)msgType, '\n');
	const int msgType = (int)pimpl->ib.read<uint8_t>("Message type");
	switch(msgType)
	{
		// Specific Request messages
		case MSG_SELECT_BATTLECMD:
		case MSG_SELECT_IDLECMD:
		case MSG_SELECT_EFFECTYN:
		case MSG_SELECT_YESNO:
		case MSG_SELECT_OPTION:
		case MSG_SELECT_CARD:
		case MSG_SELECT_CHAIN:
		case MSG_SELECT_PLACE:
		// case MSG_SELECT_DISFIELD:
		case MSG_SELECT_POSITION:
		case MSG_SELECT_TRIBUTE:
		// case MSG_SORT_CARD:
		case MSG_SORT_CHAIN:
		case MSG_SELECT_COUNTER:
		case MSG_SELECT_SUM:
		case MSG_SELECT_UNSELECT_CARD:
		case MSG_ROCK_PAPER_SCISSORS:
		case MSG_ANNOUNCE_RACE:
		case MSG_ANNOUNCE_ATTRIB:
		case MSG_ANNOUNCE_CARD:
		case MSG_ANNOUNCE_NUMBER:
		case MSG_ANNOUNCE_CARD_FILTER:
		{
			encoded = SpecificRequestMsg(msg, msgType);
		}
		break;
		
		// Specific Information messages
		case MSG_HINT:
		case MSG_MISSED_EFFECT:
		{
			encoded = SpecificInformationMsg(msg, msgType);
		}
		break;
		
		// Information messages
		case MSG_WIN:
		case MSG_CONFIRM_DECKTOP:
		case MSG_CONFIRM_CARDS:
		case MSG_SHUFFLE_DECK:
		case MSG_SHUFFLE_HAND:
		case MSG_SWAP_GRAVE_DECK:
		case MSG_SHUFFLE_SET_CARD:
		case MSG_REVERSE_DECK:
		case MSG_DECK_TOP:
		case MSG_SHUFFLE_EXTRA:
		case MSG_NEW_TURN:
		case MSG_NEW_PHASE:
		case MSG_CONFIRM_EXTRATOP:
		case MSG_MOVE:
		case MSG_POS_CHANGE:
		case MSG_SET:
		case MSG_SWAP:
		case MSG_FIELD_DISABLED:
		case MSG_SUMMONING:
		case MSG_SUMMONED:
		case MSG_SPSUMMONING:
		case MSG_SPSUMMONED:
		case MSG_FLIPSUMMONING:
		case MSG_FLIPSUMMONED:
		case MSG_CHAINING:
		case MSG_CHAINED:
		case MSG_CHAIN_SOLVING:
		case MSG_CHAIN_SOLVED:
		case MSG_CHAIN_END:
		case MSG_CHAIN_NEGATED:
		case MSG_CHAIN_DISABLED:
		case MSG_RANDOM_SELECTED:
		case MSG_BECOME_TARGET:
		case MSG_DRAW:
		case MSG_DAMAGE:
		case MSG_RECOVER:
		case MSG_LPUPDATE:
		case MSG_PAY_LPCOST:
		case MSG_CARD_TARGET:
		case MSG_CANCEL_TARGET:
		case MSG_ADD_COUNTER:
		case MSG_REMOVE_COUNTER:
		case MSG_ATTACK:
		case MSG_BATTLE:
		case MSG_ATTACK_DISABLED:
		case MSG_DAMAGE_STEP_START:
		case MSG_DAMAGE_STEP_END:
		case MSG_TOSS_COIN:
		case MSG_TOSS_DICE:
		case MSG_HAND_RES:
		case MSG_CARD_HINT:
		case MSG_PLAYER_HINT:
		case MSG_TAG_SWAP:
		case MSG_MATCH_KILL:
		{
			encoded = InformationMsg(msg, msgType);
		}
		break;
		
		// Unhandled Messages
		default:
		{
			// UNHANDLED
			pimpl->ib.log("Warning: Unhandled Message\n");
			encoded = false;
		}
		break;
	}
	
	return (lastMsg = msg);
}


} // namespace YGOpen
