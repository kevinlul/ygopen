#ifndef __DUEL_OBSERVER_DISPATCHER_HPP__
#define __DUEL_OBSERVER_DISPATCHER_HPP__
#include "duel_observer.hpp"

class DuelObserverDispatcher
{
	std::vector<DuelObserver*> observers;
	std::queue<std::pair<char*, int>> messages;
public:
	void AddMessage(BufferManipulator bm);
	void Notify();
	void Attach(DuelObserver* obs);
};

#endif // __DUEL_OBSERVER_DISPATCHER_HPP__
