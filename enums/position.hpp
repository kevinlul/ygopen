#ifndef __POSITION_HPP__
#define __POSITION_HPP__

enum Position
{
	FaceUpAttack    = 0x1,
	FaceDownAttack  = 0x2,
	FaceUpDefense   = 0x4,
	FaceDownDefense = 0x8,
	FaceUp          = 0x5,
	FaceDown        = 0xA,
	Attack          = 0x3,
	Defense         = 0xC
};

#endif // __POSITION_HPP__
