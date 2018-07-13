#ifndef __DUEL_HPP__
#define __DUEL_HPP__
#include <vector>

#include "util/buffer_manipulator.hpp"

#define DUEL_BUFFER_SIZE 4096 // size took from core/duel.cpp

class CoreInterface;
class DuelObserver;

class Duel
{
	CoreInterface* core;
	unsigned char buffer[DUEL_BUFFER_SIZE];
	long pduel;

	//int lastMsgPlayer;
	std::vector<DuelObserver*> observers;

	int Analyze(unsigned int bufferLen);
	int HandleCoreMessage(int msgType, BufferManipulator* bm);

	void Message(BufferManipulator bm);

	void MsgRetry(BufferManipulator* bm);
	void MsgHint(BufferManipulator* bm);

public:
	Duel(CoreInterface* core);
	~Duel();

	void AddObserver(DuelObserver* duelObs);

	void SetPlayersInfo(int startLP, int startHand, int drawCount);

	//void SetDeck(int player, std::vector<unsigned int>& deck);
	//void SetExtraDeck(int player, std::vector<unsigned int>& deck);
	
	void Start(int options);

	void Process();
};
#endif // __DUEL_HPP__
