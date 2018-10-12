#ifndef __REASON_HPP__
#define __REASON_HPP__

enum Reason
{
	Destroy    = 0x1,
	Release    = 0x2,
	Temporary  = 0x4,
	Material   = 0x8,
	Summon     = 0x10,
	Battle     = 0x20,
	Effect     = 0x40,
	Cost       = 0x80,
	Adjust     = 0x100,
	LostTarget = 0x200,
	Rule       = 0x400,
	SpSummon   = 0x800,
	DisSummon  = 0x1000,
	Flip       = 0x2000,
	Discard    = 0x4000,
	RDamage    = 0x8000,
	RRecover   = 0x10000,
	Return     = 0x20000,
	Fusion     = 0x40000,
	Synchro    = 0x80000,
	Ritual     = 0x100000,
	Xyz        = 0x200000,
	Replace    = 0x1000000,
	Draw       = 0x2000000,
	Redirect   = 0x4000000,
	// Reveal  = 0x8000000,
	Link       = 0x10000000
};

#endif // __REASAON_HPP__
