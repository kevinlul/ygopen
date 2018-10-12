#ifndef __REASON_TYPE_HPP__
#define __REASON_TYPE_HPP__

enum SummonType
{
	Normal   = 0x10000000,
	Advance  = 0x11000000, // Tribute Summon
	Dual     = 0x12000000, // Gemini Summon (Normal Summon again)
	Flip     = 0x20000000,
	Special  = 0x40000000,
	Fusion   = 0x43000000,
	Ritual   = 0x45000000,
	Synchro  = 0x46000000,
	Xyz      = 0x49000000,
	Pendulum = 0x4A000000,
	Link     = 0x4C000000
};


#endif // __REASON_TYPE_HPP__
