#ifndef __RACE_HPP__
#define __RACE_HPP__

enum class Race : unsigned int
{
	Warrior      = 0x1,
	Spellcaster  = 0x2,
	Fairy        = 0x4,
	Fiend        = 0x8,
	Zombie       = 0x10,
	Machine      = 0x20,
	Aqua         = 0x40,
	Pyro         = 0x80,
	Rock         = 0x100,
	WingedBeast  = 0x200,
	Plant        = 0x400,
	Insect       = 0x800,
	Thunder      = 0x1000,
	Dragon       = 0x2000,
	Beast        = 0x4000,
	BeastWarrior = 0x8000,
	Dinosaur     = 0x10000,
	Fish         = 0x20000,
	SeaSerpent   = 0x40000,
	Reptile      = 0x80000,
	Psychic      = 0x100000,
	Divine       = 0x200000,
	CreatorGod   = 0x400000,
	Wyrm         = 0x800000,
	Cyberse      = 0x1000000
};


#endif // __RACE_HPP__
