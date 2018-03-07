#include <cassert>
#include <cstdint>
#include <map>

#include "duel.hpp"

#include "core_interface.hpp"
#include "core_messages.hpp"

#include "duel_observer.hpp"

enum DuelMessage
{
	Continue = 0,
	NeedResponse = 1,
	EndOfDuel = 2,
};

// TODO: figure out where are those commented messages handled
static const std::map<int, int> msgResults =
{
	{CoreMessage::Retry, DuelMessage::NeedResponse},
	{CoreMessage::Hint, DuelMessage::Continue},
	//{CoreMessage::Waiting, }, 
	//{CoreMessage::Start, }, 
	{CoreMessage::Win, DuelMessage::EndOfDuel},
	//{CoreMessage::UpdateData, DuelMessage::Continue},
	//{CoreMessage::UpdateCard, DuelMessage::Continue},
	//{CoreMessage::RequestDeck, DuelMessage::Continue},
	{CoreMessage::SelectBattleCmd, DuelMessage::NeedResponse},
	{CoreMessage::SelectIdleCmd, DuelMessage::NeedResponse},
	{CoreMessage::SelectEffectYn, DuelMessage::NeedResponse},
	{CoreMessage::SelectYesNo, DuelMessage::NeedResponse},
	{CoreMessage::SelectOption, DuelMessage::NeedResponse},
	{CoreMessage::SelectCard, DuelMessage::NeedResponse},
	{CoreMessage::SelectChain, DuelMessage::NeedResponse},
	{CoreMessage::SelectPlace, DuelMessage::NeedResponse},
	{CoreMessage::SelectPosition, DuelMessage::NeedResponse},
	{CoreMessage::SelectTribute, DuelMessage::NeedResponse},
	{CoreMessage::SortChain, DuelMessage::NeedResponse},
	{CoreMessage::SelectCounter, DuelMessage::NeedResponse},
	{CoreMessage::SelectSum, DuelMessage::NeedResponse},
	{CoreMessage::SelectDisfield, DuelMessage::NeedResponse},
	{CoreMessage::SortCard, DuelMessage::NeedResponse},
	{CoreMessage::ConfirmDecktop, DuelMessage::Continue},
	{CoreMessage::ConfirmCards, DuelMessage::Continue},
	{CoreMessage::ShuffleDeck, DuelMessage::Continue},
	{CoreMessage::ShuffleHand, DuelMessage::Continue},
	{CoreMessage::RefreshDeck, DuelMessage::Continue},
	{CoreMessage::SwapGraveDeck, DuelMessage::Continue},
	{CoreMessage::ShuffleSetCard, DuelMessage::Continue},
	{CoreMessage::ReverseDeck, DuelMessage::Continue},
	{CoreMessage::DeckTop, DuelMessage::Continue},
	{CoreMessage::NewTurn, DuelMessage::Continue},
	{CoreMessage::NewPhase, DuelMessage::Continue},
	{CoreMessage::Move, DuelMessage::Continue},
	{CoreMessage::PosChange, DuelMessage::Continue},
	{CoreMessage::Set, DuelMessage::Continue},
	{CoreMessage::Swap, DuelMessage::Continue},
	{CoreMessage::FieldDisabled, DuelMessage::Continue},
	{CoreMessage::Summoning, DuelMessage::Continue},
	{CoreMessage::Summoned, DuelMessage::Continue},
	{CoreMessage::SpSummoning, DuelMessage::Continue},
	{CoreMessage::SpSummoned, DuelMessage::Continue},
	{CoreMessage::FlipSummoning, DuelMessage::Continue},
	{CoreMessage::FlipSummoned, DuelMessage::Continue},
	{CoreMessage::Chaining, DuelMessage::Continue},
	{CoreMessage::Chained, DuelMessage::Continue},
	{CoreMessage::ChainSolving, DuelMessage::Continue},
	{CoreMessage::ChainSolved, DuelMessage::Continue},
	{CoreMessage::ChainEnd, DuelMessage::Continue},
	{CoreMessage::ChainNegated, DuelMessage::Continue},
	{CoreMessage::ChainDisabled, DuelMessage::Continue},
	{CoreMessage::CardSelected, DuelMessage::Continue},
	{CoreMessage::RandomSelected, DuelMessage::Continue},
	{CoreMessage::BecomeTarget, DuelMessage::Continue},
	{CoreMessage::Draw, DuelMessage::Continue},
	{CoreMessage::Damage, DuelMessage::Continue},
	{CoreMessage::Recover, DuelMessage::Continue},
	{CoreMessage::Equip, DuelMessage::Continue},
	{CoreMessage::LpUpdate, DuelMessage::Continue},
	{CoreMessage::Unequip, DuelMessage::Continue},
	{CoreMessage::CardTarget, DuelMessage::Continue},
	{CoreMessage::CancelTarget, DuelMessage::Continue},
	{CoreMessage::PayLpCost, DuelMessage::Continue},
	{CoreMessage::AddCounter, DuelMessage::Continue},
	{CoreMessage::RemoveCounter, DuelMessage::Continue},
	{CoreMessage::Attack, DuelMessage::Continue},
	{CoreMessage::Battle, DuelMessage::Continue},
	{CoreMessage::AttackDisabled, DuelMessage::Continue},
	{CoreMessage::DamageStepStart, DuelMessage::Continue},
	{CoreMessage::DamageStepEnd, DuelMessage::Continue},
	{CoreMessage::MissedEffect, DuelMessage::Continue},
	{CoreMessage::BeChainTarget, DuelMessage::Continue},
	{CoreMessage::CreateRelation, DuelMessage::Continue},
	{CoreMessage::ReleaseRelation, DuelMessage::Continue},
	{CoreMessage::TossCoin, DuelMessage::Continue},
	{CoreMessage::TossDice, DuelMessage::Continue},
	{CoreMessage::RockPaperScissors, DuelMessage::NeedResponse},
	{CoreMessage::HandResult, DuelMessage::Continue},
	{CoreMessage::AnnounceRace, DuelMessage::NeedResponse},
	{CoreMessage::AnnounceAttrib, DuelMessage::NeedResponse},
	{CoreMessage::AnnounceCard, DuelMessage::NeedResponse},
	{CoreMessage::AnnounceNumber, DuelMessage::NeedResponse},
	{CoreMessage::AnnounceCardFilter, DuelMessage::NeedResponse},
	{CoreMessage::CardHint, DuelMessage::Continue},
	{CoreMessage::TagSwap, DuelMessage::Continue},
	{CoreMessage::ReloadField, DuelMessage::Continue},
	{CoreMessage::AiName, DuelMessage::Continue},
	{CoreMessage::ShowHint, DuelMessage::Continue},
	{CoreMessage::PlayerHint, DuelMessage::Continue},
	{CoreMessage::MatchKill, DuelMessage::Continue},
	//{CoreMessage::CustomMsg, DuelMessage::Continue},
	{CoreMessage::SelectUnselect, DuelMessage::NeedResponse}
};


// Fixed-length Core Messages
static const std::map<int, int> msgLengths =
{ 
	{CoreMessage::Hint           , 1 + 1 + 4},
	{CoreMessage::ShuffleDeck    , 1},
	{CoreMessage::RefreshDeck    , 1},
	{CoreMessage::SwapGraveDeck  , 1},
	{CoreMessage::ReverseDeck    , 0},
	{CoreMessage::DeckTop        , 6},
	{CoreMessage::NewTurn        , 1},
	{CoreMessage::NewPhase       , 2},
	{CoreMessage::PosChange      , 9},
	{CoreMessage::Set            , 4 + 4},
	{CoreMessage::Swap           , 16},
	{CoreMessage::FieldDisabled  , 4},
	{CoreMessage::Summoning      , 8},
	{CoreMessage::Summoned       , 0},
	{CoreMessage::SpSummoning    , 8},
	{CoreMessage::SpSummoned     , 0},
	{CoreMessage::FlipSummoning  , 8},
	{CoreMessage::FlipSummoned   , 0},
	{CoreMessage::Chaining       , 20},
	{CoreMessage::Chained        , 1},
	{CoreMessage::ChainSolving   , 1},
	{CoreMessage::ChainSolved    , 1},
	{CoreMessage::ChainEnd       , 0},
	{CoreMessage::ChainNegated   , 1},
	{CoreMessage::ChainDisabled  , 1},
	{CoreMessage::Damage         , 5},
	{CoreMessage::Recover        , 5},
	{CoreMessage::Equip          , 8},
	{CoreMessage::LpUpdate       , 5},
	{CoreMessage::Unequip        , 4},
	{CoreMessage::CardTarget     , 8},
	{CoreMessage::CancelTarget   , 8},
	{CoreMessage::PayLpCost      , 5},
	{CoreMessage::AddCounter     , 7},
	{CoreMessage::RemoveCounter  , 7},
	{CoreMessage::Attack         , 8},
	{CoreMessage::Battle         , 26},
	{CoreMessage::AttackDisabled , 0},
	{CoreMessage::DamageStepStart, 0},
	{CoreMessage::DamageStepEnd  , 0},
	{CoreMessage::MissedEffect   , 8},
	{CoreMessage::HandResult     , 1},
	{CoreMessage::CardHint       , 13},
	{CoreMessage::PlayerHint     , 10},
	{CoreMessage::MatchKill      , 1}
};

Duel::Duel(CoreInterface* core) :
	core(core),
	pduel(0)
{
	assert(!!core);

	pduel = core->create_duel(0); // TODO: randomize this value
}

Duel::~Duel()
{
	core->end_duel(pduel);
}

void Duel::SetPlayersInfo(int startLP, int startHand, int drawCount)
{
	core->set_player_info(pduel, 0, startLP, startHand, drawCount);
	core->set_player_info(pduel, 1, startLP, startHand, drawCount);
}

void Duel::Start(int options)
{
	core->start_duel(pduel, options);
}

void Duel::Process()
{
	uint16_t engineFlag = 0;
	uint32_t bufferLength = 0;
	int lastMessage = DuelMessage::Continue;
	while (true) 
	{
		if (engineFlag == 2) // Dont know what 2 means in this context
			break;

		int result = core->process(pduel);
		bufferLength = result & 0xFFFF;
		engineFlag = result >> 16;

		if (bufferLength > 0)
		{
			printf("buffer length: %d\n", bufferLength);
			core->get_message(pduel, (unsigned char*)&buffer);
			lastMessage = Analyze(bufferLength);
		}

		if(lastMessage == DuelMessage::Continue)
			continue;
		if(lastMessage == DuelMessage::NeedResponse ||
		   lastMessage == DuelMessage::EndOfDuel)
		{
			puts("Response Needed or End of Duel reached.");
			std::abort();
		}
	}
}

int Duel::Analyze(unsigned int bufferLen)
{
	BufferManipulator bm(buffer, bufferLen);
	while(bm.CanAdvance())
	{
		// Notify all observers about a new message
		Message(bm); 
		int msgType = bm.Read<uint8_t>();

		// Depending on the message we continue to read or stop
		if(HandleCoreMessage(msgType, &bm) == DuelMessage::NeedResponse)
			return DuelMessage::NeedResponse;
	}
	return DuelMessage::Continue;
}

int Duel::HandleCoreMessage(int msgType, BufferManipulator* bm)
{
	printf("\tMessage: %d\n", msgType);
	
	// Check the message response, and return if we need a user input
	auto search = msgResults.find(msgType);
	if(search != msgResults.end())
	{
		if(search->second == DuelMessage::NeedResponse ||
		   search->second == DuelMessage::EndOfDuel)
		   return search->second;
	}
	else
	{
		puts("\tMessage not handled");
		std::abort();
	}
	
	// Its a Continue message,
	// Forward BufferManipulator to the next message, if any.
	auto search2 = msgLengths.find(msgType);
	if(search2 != msgLengths.end())
	{
		// Fixed size messages
		bm->Forward(search2->second);
	}
	else
	{
		// Non-fixed size messages
		switch(msgType)
		{
			case CoreMessage::ConfirmDecktop:
			case CoreMessage::ConfirmExtratop:
			case CoreMessage::ConfirmCards:
				bm->Forward(1);
				bm->Forward(bm->Read<uint8_t>() * 7);
			break;
			case CoreMessage::ShuffleHand:
			case CoreMessage::ShuffleExtra:
			case CoreMessage::CardSelected:
			case CoreMessage::RandomSelected:
			case CoreMessage::BecomeTarget:
			case CoreMessage::Draw:
				bm->Forward(1);
				bm->Forward(bm->Read<uint8_t>() * 4);
			break;
			case CoreMessage::ShuffleSetCard:
				bm->Forward(bm->Read<uint8_t>() * 8);
			break;
			case CoreMessage::Move:
				bm->Forward(16);
			break;
			case CoreMessage::TossCoin:
			case CoreMessage::TossDice:
				bm->Forward(1);
				bm->Forward(bm->Read<uint8_t>());
			break;
			default:
				puts("\tNon-fixed forwarding not handled");
				std::abort();
			break;
		}
	}

	return DuelMessage::Continue;
}

// Messages functions
void Duel::Message(BufferManipulator bm)
{
	for(auto obs : observers)
		obs->OnNotify(bm);
}

