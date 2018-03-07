#ifndef __PLAYER_HPP__
#define __PLAYER_HPP__
#include <string>
#include <vector>
#include "util/buffer_manipulator.hpp"
class Player
{
public:
	std::string name;
	std::vector<unsigned int> deck; // Main Deck
	std::vector<unsigned int> edeck; // Extra Deck
	std::vector<unsigned int> sdeck; // Side Deck

	virtual void Send(BufferManipulator* bm) = 0;
};
#endif // __PLAYER_HPP__
