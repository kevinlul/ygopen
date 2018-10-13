#ifndef __ATTRIBUTE_HPP__
#define __ATTRIBUTE_HPP__

enum class Attribute : unsigned int
{
	Earth  = 0x1,
	Water  = 0x2,
	Fire   = 0x4,
	Wind   = 0x8,
	Light  = 0x10,
	Dark   = 0x20,
	Divine = 0x40
};

#endif // __ATTRIBUTE_HPP__
