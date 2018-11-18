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

void GetEDFromU64(Core::Data::EffectDesc* msg, effectdesc_t ed)
{
	msg->set_code(ed >> 4);
	msg->set_string_id(ed & 0xF);
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
			
			const auto summonableCardsCount = wrapper->read<small_cardcount_t>("summonable_cards.size()");
			for(int i = 0; i < summonableCardsCount; i++)
			{
				auto card = selectCmd->add_cards_summonable();
				card->set_code(wrapper->read<cardcode_t>("card code ", i));
				card->set_controller(wrapper->read<player_t>("controller ", i));
				card->set_location(wrapper->read<small_location_t>("location ", i));
				card->set_sequence(wrapper->read<sequence_t>("sequence ", i));
			}
			
			const auto spsummonableCardsCount = wrapper->read<small_cardcount_t>("spsummonable_cards.size()");
			for(int i = 0; i < spsummonableCardsCount; i++)
			{
				auto card = selectCmd->add_cards_spsummonable();
				card->set_code(wrapper->read<cardcode_t>("card code ", i));
				card->set_controller(wrapper->read<player_t>("controller ", i));
				card->set_location(wrapper->read<small_location_t>("location ", i));
				card->set_sequence(wrapper->read<sequence_t>("sequence ", i));
			}
			
			const auto repositionableCardsCount = wrapper->read<small_cardcount_t>("repositionable_cards.size()");
			for(int i = 0; i < repositionableCardsCount; i++)
			{
				auto card = selectCmd->add_cards_spsummonable();
				card->set_code(wrapper->read<cardcode_t>("card code ", i));
				card->set_controller(wrapper->read<player_t>("controller ", i));
				card->set_location(wrapper->read<small_location_t>("location ", i));
				card->set_sequence(wrapper->read<small_sequence_t>("sequence ", i)); // NOTE: different size
			}
			
			const auto msetableCardsCount = wrapper->read<small_cardcount_t>("msetable_cards.size()");
			for(int i = 0; i < msetableCardsCount; i++)
			{
				auto card = selectCmd->add_cards_msetable();
				card->set_code(wrapper->read<cardcode_t>("card code ", i));
				card->set_controller(wrapper->read<player_t>("controller ", i));
				card->set_location(wrapper->read<small_location_t>("location ", i));
				card->set_sequence(wrapper->read<sequence_t>("sequence ", i));
			}
			
			const auto ssetableCardsCount = wrapper->read<small_cardcount_t>("ssetable_cards.size()");
			for(int i = 0; i < ssetableCardsCount; i++)
			{
				auto card = selectCmd->add_cards_ssetable();
				card->set_code(wrapper->read<cardcode_t>("card code ", i));
				card->set_controller(wrapper->read<player_t>("controller ", i));
				card->set_location(wrapper->read<small_location_t>("location ", i));
				card->set_sequence(wrapper->read<sequence_t>("sequence ", i));
			}
			
			const auto cardsWEffectCardsCount = wrapper->read<small_cardcount_t>("select_chains.size()");
			for(int i = 0; i < cardsWEffectCardsCount; i++)
			{
				auto card = selectCmd->add_cards_w_effect();
				card->set_code(wrapper->read<cardcode_t>("card code ", i));
				card->set_controller(wrapper->read<player_t>("controller ", i));
				card->set_location(wrapper->read<small_location_t>("location ", i));
				card->set_sequence(wrapper->read<sequence_t>("sequence ", i));
				GetEDFromU64(card->mutable_effect_desc(), wrapper->read<effectdesc_t>("effectdesc ", i));
			}
			
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
