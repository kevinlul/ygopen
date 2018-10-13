#ifndef __STATUS_HPP__
#define __STATUS_HPP__

enum class Status : unsigned int
{
	Disabled         = 0x1,
	ToEnable         = 0x2,
	ToDisable        = 0x4,
	ProcComplete     = 0x8,
	SetTurn          = 0x10,
	NoLevel          = 0x20,
	BattleResult     = 0x40,
	SpSummonStep     = 0x80,
	FormChanged      = 0x100,
	Summoning        = 0x200,
	EffectEnabled    = 0x400,
	SummonTurn       = 0x800,
	DestroyConfirmed = 0x1000,
	LeaveConfirmed   = 0x2000,
	BattleDestroyed  = 0x4000,
	CopyingEffect    = 0x8000,
	Chaining         = 0x10000,
	SummonDisabled   = 0x20000,
	ActivateDisabled = 0x40000,
	EffectReplaced   = 0x80000,
	FutureFusion     = 0x100000,
	AttackCanceled   = 0x200000,
	Initializing     = 0x400000,
	Activated        = 0x800000,
	JustPos          = 0x1000000,
	ContinousPos     = 0x2000000,
	Forbidden        = 0x4000000,
	ActFromHand      = 0x8000000,
	OppBattle        = 0x10000000,
	FlipSummonTurn   = 0x20000000,
	SpSummonTurn     = 0x40000000
};

#endif // __STATUS_HPP__
