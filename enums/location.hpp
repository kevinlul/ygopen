#ifndef __LOCATION_HPP__
#define __LOCATION_HPP__

enum class Location : unsigned int
{
	MainDeck     = 0x1,
	Hand         = 0x2,
	MonsterZone  = 0x4,
	SpellZone    = 0x8,
	Graveyard    = 0x10,
	Banished     = 0x20,
	ExtraDeck    = 0x40,
	Overlay      = 0x80,
	OnField      = 0x0C,
	FieldZone    = 0x100,
	PendulumZone = 0x200
};

#endif // __LOCATION_HPP__
