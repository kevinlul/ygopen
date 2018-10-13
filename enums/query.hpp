#ifndef __QUERY_HPP__
#define __QUERY_HPP__

enum class Query : unsigned int
{
	Code        = 0x1,
	Position    = 0x2,
	Alias       = 0x4,
	Type        = 0x8,
	Level       = 0x10,
	Rank        = 0x20,
	Attribute   = 0x40,
	Race        = 0x80,
	Attack      = 0x100,
	Defense     = 0x200,
	BaseAttack  = 0x400,
	BaseDefense = 0x800,
	Reason      = 0x1000,
	ReasonCard  = 0x2000,
	EquipCard   = 0x4000,
	TargetCard  = 0x8000,
	OverlayCard = 0x10000,
	Counters    = 0x20000,
	Owner       = 0x40000,
	IsDisabled  = 0x80000,
	IsPublic    = 0x100000,
	LScale      = 0x200000,
	RSCale      = 0x400000,
	Link        = 0x800000
};

#endif // __QUERY_HPP__
