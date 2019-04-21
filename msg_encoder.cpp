#include <functional>
#include <bitset>

#include <google/protobuf/text_format.h>

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
using small_cardcount_t = uint8_t;
using small_location_t = uint8_t;
using small_sequence_t = uint8_t;
using small_position_t = uint8_t;

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
		ToCardCode(wrapper->read<cardcode_t>("card code ", i), card);

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
			ReadCardVector<small_cardcount_t, small_location_t, sequence_t>(wrapper, add_w_effect, nullptr, ReadEffectDesc);
			
			CardSpawner add_can_attack = BindFromPointer(selectCmd, add_cards_can_attack);
			InlineCardRead ReadAtkDirectly = [](Buffer::ibufferw& wrapper, const int count, Core::Data::CardInfo* card)
			{
				card->set_can_attack_directly(wrapper->read<uint8_t>("can_attack_directly ", count));
			};
			ReadCardVector<small_cardcount_t, small_location_t, small_sequence_t>(wrapper, add_can_attack, nullptr, ReadAtkDirectly);
			
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
			ReadCardVector<small_cardcount_t, small_location_t, sequence_t>(wrapper, add_summonable);

			CardSpawner add_spsummonable = BindFromPointer(selectCmd, add_cards_spsummonable);
			ReadCardVector<small_cardcount_t, small_location_t, sequence_t>(wrapper, add_spsummonable);

			CardSpawner add_repositionable = BindFromPointer(selectCmd, add_cards_repositionable);
			ReadCardVector<small_cardcount_t, small_location_t, small_sequence_t>(wrapper, add_repositionable);

			CardSpawner add_msetable = BindFromPointer(selectCmd, add_cards_msetable);
			ReadCardVector<small_cardcount_t, small_location_t, sequence_t>(wrapper, add_msetable);

			CardSpawner add_ssetable = BindFromPointer(selectCmd, add_cards_ssetable);
			ReadCardVector<small_cardcount_t, small_location_t, sequence_t>(wrapper, add_ssetable);

			CardSpawner add_w_effect = BindFromPointer(selectCmd, add_cards_w_effect);
			InlineCardRead ReadEffectDesc = [](Buffer::ibufferw& wrapper, const int count, Core::Data::CardInfo* card)
			{
				ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc ", count), card->mutable_effect_desc());
			};
			ReadCardVector<small_cardcount_t, small_location_t, sequence_t>(wrapper, add_w_effect, nullptr, ReadEffectDesc);

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

			selectCards->set_min(wrapper->read<uint8_t>("min"));
			selectCards->set_max(wrapper->read<uint8_t>("max"));

			CardSpawner add_selectable = BindFromPointer(selectCards, add_cards_selectable);
			ReadCardVector<cardcount_t, small_location_t, sequence_t, position_t>(wrapper, add_selectable);
			
			encoded = true;
		}
		break;
		case MSG_SELECT_CHAIN:
		{
			auto selectToChain = specific->mutable_request()->mutable_select_to_chain();

			const auto count = wrapper->read<uint8_t>("number of chains");

			wrapper->seek(1, Buffer::seek_dir::cur, "spe_count");

			selectToChain->set_forced(wrapper->read<uint8_t>("forced"));

			wrapper->seek(8, Buffer::seek_dir::cur, "hint_timing x 2");

			// I use a normal loop to collect info for this one
			// NOTE: can probably be improved upon but i cannot think
			// of a better way of doing it right now
			for(int i = 0; i < count; i++)
			{
				auto chainCard = selectToChain->add_cards_w_effect();

				chainCard->set_effect_desc_type(wrapper->read<uint8_t>("EDESC ", i));
				auto card = chainCard->mutable_card();

				ToCardCode(wrapper->read<cardcode_t>("card code ", i), card);
				ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, i, card);
				ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc ", i), card->mutable_effect_desc());
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

			auto extractPlacesForPlayer = [&selectPlaces, &flag](const int player, const int indexStart)
			{
				int i = indexStart;

				auto addPlace = [&selectPlaces, player](int location, int sequence)
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

			extractPlacesForPlayer(0, 0);
			extractPlacesForPlayer(1, 16);
			
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

			selectCards->set_min(wrapper->read<uint8_t>("min"));
			selectCards->set_max(wrapper->read<uint8_t>("max"));

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
			ReadCardVector<small_cardcount_t, small_location_t, small_sequence_t, do_not_read_t>(wrapper, add_cards_selectable, nullptr, ReadCounterCount);
			
			// TODO: Set right counter type to each card?
			
			encoded = true;
		}
		break;
		case MSG_SELECT_SUM:
		{
			auto selectCards = specific->mutable_request()->mutable_select_cards();
			
			selectCards->set_type(Core::Msg::SelectCards::SELECTION_SUM);
			
			selectCards->set_sum(wrapper->read<uint16_t>("acc (sum)"));
			
			selectCards->set_min(wrapper->read<uint8_t>("min"));
			selectCards->set_max(wrapper->read<uint8_t>("max"));
			
			InlineCardRead ReadSumParam = [](Buffer::ibufferw& wrapper, const int count, Core::Data::CardInfo* card)
			{
				card->set_sum_param(wrapper->read<uint32_t>("sum param ", count));
			};
			
			CardSpawner add_cards_must_select = BindFromPointer(selectCards, add_cards_must_select);
			ReadCardVector<small_cardcount_t, small_location_t, sequence_t, do_not_read_t>(wrapper, add_cards_must_select, nullptr, ReadSumParam);
			
			CardSpawner add_cards_selectable = BindFromPointer(selectCards, add_cards_selectable);
			ReadCardVector<small_cardcount_t, small_location_t, sequence_t, do_not_read_t>(wrapper, add_cards_selectable, nullptr, ReadSumParam);
			
			encoded = true;
		}
		break;
		case MSG_SELECT_UNSELECT_CARD:
		{
			auto selectCards = specific->mutable_request()->mutable_select_cards();
			
			selectCards->set_can_accept(wrapper->read<uint8_t>("buttonok (can_accept)"));
			selectCards->set_can_cancel(wrapper->read<uint8_t>("cancelable"));
			selectCards->set_min(wrapper->read<uint8_t>("min"));
			selectCards->set_max(wrapper->read<uint8_t>("max"));
			
			CardSpawner add_selectable = BindFromPointer(selectCards, add_cards_selectable);
			ReadCardVector<cardcount_t, small_location_t, sequence_t, position_t>(wrapper, add_selectable);
			
			CardSpawner add_unselectable = BindFromPointer(selectCards, add_cards_unselectable);
			ReadCardVector<cardcount_t, small_location_t, sequence_t, position_t>(wrapper, add_unselectable);
			
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
		case MSG_CONFIRM_DECKTOP:
		{
			auto confirmCards = specific->mutable_information()->mutable_confirm_cards();
			
			confirmCards->set_type(Core::Msg::ConfirmCards::CONFIRM_DECKTOP);
			
			specific->set_player(wrapper->read<player_t>("player"));
			
			CardSpawner add_cards = BindFromPointer(confirmCards, add_cards);
			ReadCardVector<small_cardcount_t, small_location_t, small_sequence_t>(wrapper, add_cards);
			
			encoded = true;
		}
		break;
		case MSG_CONFIRM_CARDS:
		{
			auto confirmCards = specific->mutable_information()->mutable_confirm_cards();
			
			confirmCards->set_type(Core::Msg::ConfirmCards::CONFIRM_CARDS);
			
			specific->set_player(wrapper->read<player_t>("player"));
			
			CardSpawner add_cards = BindFromPointer(confirmCards, add_cards);
			ReadCardVector<small_cardcount_t, small_location_t, sequence_t>(wrapper, add_cards);
			
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
		case MSG_MATCH_KILL:
		{
			pimpl->isMatchKill = true;
			pimpl->matchKillCardId = wrapper->read<cardcode_t>("match killer");
			
			encoded = false;
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
			
			auto count = wrapper->read<small_cardcount_t>(".size()");
			CardSpawner add_shuffled_cards = BindFromPointer(shuffleLocation, add_shuffled_cards);
			for(decltype(count) i = 0; i < count; i++)
			{
				Core::Data::CardInfo* card = add_shuffled_cards();
				ToCardCode(wrapper->read<cardcode_t>("card code ", i), card);
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
	}
	
	return encoded;
}

Core::AnyMsg MsgEncoder::Encode(void* buffer, size_t length, bool& encoded)
{
	bool unhandled = false;
	Core::AnyMsg msg{};

	pimpl->ib.open(buffer, length);

	const auto msgType = pimpl->ib.read<uint8_t>("Encode: message type");
	pimpl->ib.log("Message Type: ", (int)msgType, '\n');
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
		{
			encoded = SpecificRequestMsg(msg, msgType);
		}
		break;
		
		// Specific Information messages
		case MSG_HINT:
		case MSG_CONFIRM_DECKTOP:
		case MSG_CONFIRM_CARDS:
		{
			encoded = SpecificInformationMsg(msg, msgType);
		}
		break;
		
		// Information messages
		case MSG_WIN:
		case MSG_MATCH_KILL:
		case MSG_SHUFFLE_DECK:
		case MSG_SHUFFLE_HAND:
		case MSG_SWAP_GRAVE_DECK:
		case MSG_SHUFFLE_SET_CARD:
		case MSG_REVERSE_DECK:
		{
			encoded = InformationMsg(msg, msgType);
		}
		break;
		
		// Unhandled Messages
		default:
		{
			// UNHANDLED
			pimpl->ib.log("Warning: Unhandled Message\n");
			unhandled = true;
			encoded = false;
		}
		break;
	}
	
	if(!unhandled)
	{
		std::string str;
		google::protobuf::TextFormat::PrintToString(msg, &str);
		std::cout << str << std::endl;
	}
	
	return (lastMsg = msg);
}


} // namespace YGOpen
