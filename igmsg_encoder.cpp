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
typedef uint64_t effectdesc_t;

// Types that should be changed (increased) in far future
typedef uint8_t small_cardcount_t;
typedef uint8_t small_sequence_t;
typedef uint8_t small_location_t;

// reads a effectdesc_t value and places the right content on the given EffectDesc
void ToEffectDesc(effectdesc_t ed, Core::Data::EffectDesc* msg)
{
	msg->set_code(ed >> 4);
	msg->set_string_id(ed & 0xF);
}

// Function that returns a new CardInfo.
// The returned card should be inside of a array,
// managed by a GMsg; normally the [gmsg]->add_* functions
typedef std::function<YGOpen::Core::Data::CardInfo*()> CardSpawner;

// Binds a function from a object pointer, without needing to write the full object path.
// NOTE: Can be improved but this works right now.
#define BindFromPointer(object, member) std::bind(&std::remove_pointer<decltype(object)>::type::member, object)

// Function that reads data from a given wrapper and places the right content on the given card.
typedef std::function<void(Buffer::ibufferw& wrapper, int& count, YGOpen::Core::Data::CardInfo* card)> InlineCardRead;

// Used to tell ReadCardVector to not read a specific value
struct do_not_read_t {};

// Reads a card vector from the given wrapper.
// a CardSpawner must be given, which should give a new card each time it is called
// aditionally, two functions before and after the default card read can be given, in case aditional info should be read
template<typename Count, typename Location, typename Sequence>
void ReadCardVector(Buffer::ibufferw& wrapper, CardSpawner cs, InlineCardRead bcr = nullptr, InlineCardRead acr = nullptr)
{
	const Count count = wrapper->read<Count>(".size()");
	for(int i = 0; i < (int)count; i++)
	{
		YGOpen::Core::Data::CardInfo* card = cs();
		
		// Before Card Read
		if(bcr) bcr(wrapper, i, card);

		// Card Code & Dirty Bit
		const auto code = wrapper->read<cardcode_t>("card code ", i);
		card->set_code(code & 0x7FFFFFFF); // Do not include last bit
		card->set_bit(code & 0x80000000);

		// Controller
		card->set_controller(wrapper->read<player_t>("controller ", i));
		
		// Location
		if constexpr(!std::is_same<do_not_read_t, Location>())
			card->set_location(wrapper->read<Location>("location ", i));
		
		// Sequence
		if constexpr(!std::is_same<do_not_read_t, Sequence>())
			card->set_sequence(wrapper->read<Sequence>("sequence ", i));
		
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
	
	switch(msgType)
	{
		case SelectIdleCmd:
		{
			auto specific = gmsg.mutable_specific();
			specific->set_player(wrapper->read<player_t>("player"));
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
			InlineCardRead ReadEffectDesc = [](Buffer::ibufferw& wrapper, int& count, YGOpen::Core::Data::CardInfo* card)
			{
				ToEffectDesc(wrapper->read<effectdesc_t>("effectdesc ", count), card->mutable_effect_desc());
			};
			ReadCardVector<small_cardcount_t, small_location_t, sequence_t>(wrapper, add_w_effect, nullptr, ReadEffectDesc);
			
			selectCmd->set_able_to_bp(wrapper->read<uint8_t>("to_bp"));
			selectCmd->set_able_to_ep(wrapper->read<uint8_t>("to_ep"));
			selectCmd->set_can_shuffle(wrapper->read<uint8_t>("can_shuffle"));
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
