#include <functional>

#include "io_gmsg_stream.hpp"
#include "buffer.hpp"
#include "core_message.hpp"

namespace YGOpen
{
// Useful typedefs
typedef uint8_t  player_t; // used for controller too
typedef uint32_t cardcount_t;
typedef uint32_t cardcode_t;
typedef uint32_t location_t;
typedef uint32_t sequence_t;
typedef uint32_t position_t;
typedef uint64_t effectdesc_t;

// Types that should be changed (increased) in far future
typedef uint8_t small_cardcount_t;
typedef uint8_t small_sequence_t;
typedef uint8_t small_location_t;

// reads a effectdesc_t value and places the right content on the given EffectDesc
void ToEffectDesc(const effectdesc_t ed, Core::Data::EffectDesc* msg)
{
	msg->set_code(ed >> 4);
	msg->set_string_id(ed & 0xF);
}

// reads a card code and sets the right values of a Data.CardInfo message
void ToCardCode(const cardcode_t code, YGOpen::Core::Data::CardInfo* card)
{
	card->set_code(code & 0x7FFFFFFF); // Do not include last bit
	card->set_bit(code & 0x80000000);
}

// Function prototype that returns a CardInfo (newly created most likely).
// The returned card should be inside of a array,
// managed by a GMsg; normally the [gmsg]->add_* functions
typedef std::function<YGOpen::Core::Data::CardInfo*()> CardSpawner;

// Binds a function from a object pointer, without needing to write the full object path.
// NOTE: Can be improved but this works right now.
#define BindFromPointer(object, member) std::bind(&std::remove_pointer<decltype(object)>::type::member, object)

// Function prototype that reads data from a given wrapper and places the right content on the given card.
typedef std::function<void(Buffer::ibufferw& wrapper, const int count, YGOpen::Core::Data::CardInfo* card)> InlineCardRead;

// Used to tell ReadCardLocInfo to not read a specific value, at compile time
struct do_not_read_t {};

// Reads a single card location info from the given wrapper
template<typename Controller, typename Location, typename Sequence, typename Position>
void ReadCardLocInfo(Buffer::ibufferw& wrapper, const int count, YGOpen::Core::Data::CardInfo* card)
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
	for(int i = 0; i < (int)count; i++)
	{
		YGOpen::Core::Data::CardInfo* card = cs();
		
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

struct IGMsgEncoder::impl
{
	Buffer::ibuffer ib{};
};

IGMsgEncoder::IGMsgEncoder() : pimpl(new impl())
{}

IGMsgEncoder::~IGMsgEncoder() = default;

inline void IGMsgEncoder::SpecificMsg(Core::GMsg& gmsg, const int msgType)
{
	Buffer::ibufferw wrapper(&pimpl->ib);
	
	auto specific = gmsg.mutable_specific();
	specific->set_player(wrapper->read<player_t>("player"));
	
	switch(msgType)
	{
		case SelectBattleCmd:
		{
			auto selectCmd = specific->mutable_request()->mutable_select_cmd();
			
			selectCmd->set_type(Core::SelectCmd::COMMAND_BATTLE);
			
			CardSpawner add_w_effect = BindFromPointer(selectCmd, add_cards_w_effect);
			InlineCardRead ReadEffectDesc = [](Buffer::ibufferw& wrapper, const int count, YGOpen::Core::Data::CardInfo* card)
			{
				ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc ", count), card->mutable_effect_desc());
			};
			ReadCardVector<small_cardcount_t, small_location_t, sequence_t>(wrapper, add_w_effect, nullptr, ReadEffectDesc);
			
			CardSpawner add_can_attack = BindFromPointer(selectCmd, add_cards_can_attack);
			InlineCardRead ReadAtkDirectly = [](Buffer::ibufferw& wrapper, const int count, YGOpen::Core::Data::CardInfo* card)
			{
				card->set_can_attack_directly(wrapper->read<uint8_t>("can_attack_directly ", count));
			};
			ReadCardVector<small_cardcount_t, small_location_t, small_sequence_t>(wrapper, add_can_attack, nullptr, ReadAtkDirectly);
			
			selectCmd->set_able_to_mp2(wrapper->read<uint8_t>("to_mp2"));
			selectCmd->set_able_to_ep(wrapper->read<uint8_t>("to_ep"));
		}
		break;
		case SelectIdleCmd:
		{
			auto selectCmd = specific->mutable_request()->mutable_select_cmd();
			
			selectCmd->set_type(Core::SelectCmd::COMMAND_IDLE);
			
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
			InlineCardRead ReadEffectDesc = [](Buffer::ibufferw& wrapper, const int count, YGOpen::Core::Data::CardInfo* card)
			{
				ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc ", count), card->mutable_effect_desc());
			};
			ReadCardVector<small_cardcount_t, small_location_t, sequence_t>(wrapper, add_w_effect, nullptr, ReadEffectDesc);
			
			selectCmd->set_able_to_bp(wrapper->read<uint8_t>("to_bp"));
			selectCmd->set_able_to_ep(wrapper->read<uint8_t>("to_ep"));
			selectCmd->set_can_shuffle(wrapper->read<uint8_t>("can_shuffle"));
		}
		break;
		case SelectEffectYn:
		{
			auto selectYesNo = specific->mutable_request()->mutable_select_yesno();
			
			auto card = selectYesNo->mutable_card();
			ToCardCode(wrapper->read<cardcode_t>("card code"), card);
			ReadCardLocInfo<player_t, small_location_t, sequence_t, position_t>(wrapper, 0, card);
			ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc"), card->mutable_effect_desc());
		}
		break;
		case SelectYesNo:
		{
			auto selectYesNo = specific->mutable_request()->mutable_select_yesno();
			
			ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc"), selectYesNo->mutable_effect());
		}
		break;
		case SelectOption:
		{
			auto selectOption = specific->mutable_request()->mutable_select_option();
			
			const auto count = wrapper->read<uint8_t>("number of effects");
			for(int i = 0; i < count; i++)
				ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc ", (int)count), selectOption->add_effects());
		}
		break;
		case SelectCard:
		{
			auto selectCard = specific->mutable_request()->mutable_select_cards();
			
			selectCard->set_type(Core::SelectCards::SELECTION_EFFECT);
			
			selectCard->set_can_cancel(wrapper->read<uint8_t>("can_cancel"));
			
			selectCard->set_min(wrapper->read<uint8_t>("min"));
			selectCard->set_max(wrapper->read<uint8_t>("max"));
			
			CardSpawner add_selectable = BindFromPointer(selectCard, add_cards_selectable);
			ReadCardVector<cardcount_t, small_location_t, sequence_t, position_t>(wrapper, add_selectable);
		}
		break;
	}
}

inline void IGMsgEncoder::InformationMsg(Core::GMsg& gmsg, const int msgType)
{
	Buffer::ibufferw wrapper(&pimpl->ib);
}

Core::GMsg IGMsgEncoder::Encode(void* buffer, size_t length)
{
	Core::GMsg gmsg{};

	pimpl->ib.open(buffer, length);

	const auto msgType = pimpl->ib.read<uint8_t>("Encode: message type");
	switch(msgType)
	{
		// Specific messages
		case SelectBattleCmd:
		case SelectIdleCmd:
		case SelectEffectYn:
		case SelectYesNo:
		case SelectOption:
		case SelectCard:
		{
			SpecificMsg(gmsg, msgType);
		}
		break;
		// Information messages
		case Win:
		case MatchKill:
		{
			InformationMsg(gmsg, msgType);
		}
		break;
		default:
		{
			// UNHANDLED
		}
		break;
	}

	return (lastGMsg = gmsg);
}


} // namespace YGOpen
