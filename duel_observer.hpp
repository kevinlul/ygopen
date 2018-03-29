#ifndef __DUEL_OBSERVER_HPP__
#define __DUEL_OBSERVER_HPP__
#include "util/buffer_manipulator.hpp"

class DuelObserver
{
public:
	virtual void OnNotify(BufferManipulator bm) = 0;
};
#endif // __DUEL_OBSERVER_HPP__
