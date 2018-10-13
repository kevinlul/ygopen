#ifndef __MISC_HPP__
#define __MISC_HPP__

enum class Assume : unsigned int
{
	Code       = 1,
	Type       = 2,
	Level      = 3,
	Rank       = 4,
	Attribute  = 5,
	Race       = 6,
	Attack     = 7,
	Defense    = 8,
	Link       = 9,
	LinkMarker = 10
};

enum class LinkMarker : unsigned int
{
	BottomLeft  = 0x1,
	Bottom      = 0x2,
	BottomRight = 0x4,
	Left        = 0x8,
	Right       = 0x20,
	TopLeft     = 0x40,
	Top         = 0x80,
	TopRight    = 0x100
};

// Flip effect flags
#define NO_FLIP_EFFECT     0x10000
#define FLIP_SET_AVAILABLE 0x20000

// Counter
#define COUNTER_WITHOUT_PERMIT 0x1000
#define COUNTER_NEED_ENABLE    0x2000

#endif // __MISC_HPP__
