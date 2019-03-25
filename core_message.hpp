#ifndef __CORE_MESSAGE_HPP__
#define __CORE_MESSAGE_HPP__

enum CoreMessage
{
	Retry              = 1,
	Hint               = 2,
	Waiting            = 3,
	Start              = 4,
	Win                = 5,
	UpdateData         = 6,
	UpdateCard         = 7,
	RequestDeck        = 8,
	SelectBattleCmd    = 10,
	SelectIdleCmd      = 11,
	SelectEffectYn     = 12,
	SelectYesNo        = 13,
	SelectOption       = 14,
	SelectCard         = 15,
	SelectChain        = 16,
	SelectPlace        = 18,
	SelectPosition     = 19,
	SelectTribute      = 20,
	SortChain          = 21,
	SelectCounter      = 22,
	SelectSum          = 23,
	SelectDisfield     = 24,
	SortCard           = 25,
	ConfirmDecktop     = 30,
	ConfirmCards       = 31,
	ShuffleDeck        = 32,
	ShuffleHand        = 33,
	RefreshDeck        = 34,
	SwapGraveDeck      = 35,
	ShuffleSetCard     = 36,
	ReverseDeck        = 37,
	DeckTop            = 38,
	NewTurn            = 40,
	NewPhase           = 41,
	Move               = 50,
	PosChange          = 53,
	Set                = 54,
	Swap               = 55,
	FieldDisabled      = 56,
	Summoning          = 60,
	Summoned           = 61,
	SpSummoning        = 62,
	SpSummoned         = 63,
	FlipSummoning      = 64,
	FlipSummoned       = 65,
	Chaining           = 70,
	Chained            = 71,
	ChainSolving       = 72,
	ChainSolved        = 73,
	ChainEnd           = 74,
	ChainNegated       = 75,
	ChainDisabled      = 76,
	CardSelected       = 80,
	RandomSelected     = 81,
	BecomeTarget       = 83,
	Draw               = 90,
	Damage             = 91,
	Recover            = 92,
	Equip              = 93,
	LpUpdate           = 94,
	Unequip            = 95,
	CardTarget         = 96,
	CancelTarget       = 97,
	PayLpCost          = 100,
	AddCounter         = 101,
	RemoveCounter      = 102,
	Attack             = 110,
	Battle             = 111,
	AttackDisabled     = 112,
	DamageStepStart    = 113,
	DamageStepEnd      = 114,
	MissedEffect       = 120,
	BeChainTarget      = 121,
	CreateRelation     = 122,
	ReleaseRelation    = 123,
	TossCoin           = 130,
	TossDice           = 131,
	RockPaperScissors  = 132,
	HandResult         = 133,
	AnnounceRace       = 140,
	AnnounceAttrib     = 141,
	AnnounceCard       = 142,
	AnnounceNumber     = 143,
	AnnounceCardFilter = 144,
	CardHint           = 160,
	TagSwap            = 161,
	ReloadField        = 162,
	AiName             = 163,
	ShowHint           = 164,
	PlayerHint         = 165,
	MatchKill          = 170,
	CustomMsg          = 180,

	// Extensions
	SelectUnselect     = 26,
	ShuffleExtra       = 39,
	ConfirmExtratop    = 42
};

// copied directly from common.h
// TODO: convert into enum (or probably not?)
// TODO: comment unused values
#define MSG_RETRY				1
#define MSG_HINT				2
#define MSG_WAITING				3
#define MSG_START				4
#define MSG_WIN					5
#define MSG_UPDATE_DATA			6
#define MSG_UPDATE_CARD			7
#define MSG_REQUEST_DECK		8
#define MSG_SELECT_BATTLECMD	10 // DONE
#define MSG_SELECT_IDLECMD		11 // DONE
#define MSG_SELECT_EFFECTYN		12 // DONE
#define MSG_SELECT_YESNO		13 // DONE
#define MSG_SELECT_OPTION		14 // DONE
#define MSG_SELECT_CARD			15 // DONE
#define MSG_SELECT_CHAIN		16 // DONE
#define MSG_SELECT_PLACE		18 // DONE
#define MSG_SELECT_POSITION		19 // DONE
#define MSG_SELECT_TRIBUTE		20 // DONE
#define MSG_SORT_CHAIN			21 // DONE
#define MSG_SELECT_COUNTER		22 // DONE
#define MSG_SELECT_SUM			23 // DONE
#define MSG_SELECT_DISFIELD		24 // TODO
#define MSG_SORT_CARD			25 // TODO
#define MSG_SELECT_UNSELECT_CARD	26
#define MSG_CONFIRM_DECKTOP		30
#define MSG_CONFIRM_CARDS		31
#define MSG_SHUFFLE_DECK		32
#define MSG_SHUFFLE_HAND		33
#define MSG_REFRESH_DECK		34
#define MSG_SWAP_GRAVE_DECK		35
#define MSG_SHUFFLE_SET_CARD	36
#define MSG_REVERSE_DECK		37
#define MSG_DECK_TOP			38
#define MSG_SHUFFLE_EXTRA		39
#define MSG_NEW_TURN			40
#define MSG_NEW_PHASE			41
#define MSG_CONFIRM_EXTRATOP	42
#define MSG_MOVE				50
#define MSG_POS_CHANGE			53
#define MSG_SET					54
#define MSG_SWAP				55
#define MSG_FIELD_DISABLED		56
#define MSG_SUMMONING			60
#define MSG_SUMMONED			61
#define MSG_SPSUMMONING			62
#define MSG_SPSUMMONED			63
#define MSG_FLIPSUMMONING		64
#define MSG_FLIPSUMMONED		65
#define MSG_CHAINING			70
#define MSG_CHAINED				71
#define MSG_CHAIN_SOLVING		72
#define MSG_CHAIN_SOLVED		73
#define MSG_CHAIN_END			74
#define MSG_CHAIN_NEGATED		75
#define MSG_CHAIN_DISABLED		76
#define MSG_CARD_SELECTED		80
#define MSG_RANDOM_SELECTED		81
#define MSG_BECOME_TARGET		83
#define MSG_DRAW				90
#define MSG_DAMAGE				91
#define MSG_RECOVER				92
#define MSG_EQUIP				93
#define MSG_LPUPDATE			94
#define MSG_UNEQUIP				95
#define MSG_CARD_TARGET			96
#define MSG_CANCEL_TARGET		97
#define MSG_PAY_LPCOST			100
#define MSG_ADD_COUNTER			101
#define MSG_REMOVE_COUNTER		102
#define MSG_ATTACK				110
#define MSG_BATTLE				111
#define MSG_ATTACK_DISABLED		112
#define MSG_DAMAGE_STEP_START	113
#define MSG_DAMAGE_STEP_END		114
#define MSG_MISSED_EFFECT		120
#define MSG_BE_CHAIN_TARGET		121
#define MSG_CREATE_RELATION		122
#define MSG_RELEASE_RELATION	123
#define MSG_TOSS_COIN			130
#define MSG_TOSS_DICE			131
#define MSG_ROCK_PAPER_SCISSORS	132
#define MSG_HAND_RES			133
#define MSG_ANNOUNCE_RACE		140
#define MSG_ANNOUNCE_ATTRIB		141
#define MSG_ANNOUNCE_CARD		142
#define MSG_ANNOUNCE_NUMBER		143
#define MSG_ANNOUNCE_CARD_FILTER	144
#define MSG_CARD_HINT			160
#define MSG_TAG_SWAP			161
#define MSG_RELOAD_FIELD		162	// Debug.ReloadFieldEnd()
#define MSG_AI_NAME				163
#define MSG_SHOW_HINT			164
#define MSG_PLAYER_HINT			165
#define MSG_MATCH_KILL			170
#define MSG_CUSTOM_MSG			180


#endif // __CORE_MESSAGE_HPP__
