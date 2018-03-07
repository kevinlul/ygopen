#ifndef __PLAYER_LOCAL_HPP__
#define __PLAYER_LOCAL_HPP__
#include "player.hpp"
class PlayerLocal : public Player
{
	void Send(BufferManipulator* bm);
};
#endif // __PLAYER_LOCAL_HPP__
