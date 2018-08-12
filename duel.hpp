#ifndef __DUEL_HPP__
#define __DUEL_HPP__
#include <vector>
#include <utility>

#include "util/buffer_manipulator.hpp"

#define DUEL_BUFFER_SIZE 4096 // size took from core/duel.cpp

class CoreInterface;
class DuelObserver;

class Duel
{
	CoreInterface& core;
	unsigned char buffer[DUEL_BUFFER_SIZE];
	long pduel;

	std::vector<DuelObserver*> observers;

	int Analyze(unsigned int bufferLen);
	int HandleCoreMessage(int msgType, BufferManipulator* bm);

	void Message(void* buff, size_t length);
public:
	Duel(CoreInterface& core, unsigned int seed);
	~Duel();

	void AddObserver(DuelObserver* duelObs);

	void Start(int options);

	//TODO: create individual version
	void SetPlayersInfo(int startLP, int startHand, int drawCount);

	void Process();

	void NewCard(int code, int owner, int playerID, int location, int sequence, int position);
	void NewTagCard(int code, int owner, int location);
	void NewRelayCard(int code, int owner, int location, int playerNumber);

	// Any call to the following functions invalidates the result of the last function called
	std::pair<void*, size_t> QueryCard(int playerID, int location, int sequence, int queryFlag, bool useCache = false);
	int QueryFieldCount(int playerID, int location);
	std::pair<void*, size_t> QueryFieldCard(int playerID, int location, int queryFlag, bool useCache = false);
	std::pair<void*, size_t> QueryFieldInfo();
	
	void SetResponseInteger(int val);
	void SetResponseBuffer(void* buff, size_t length);

	//void SetDeck(int player, std::vector<unsigned int>& deck);
	//void SetExtraDeck(int player, std::vector<unsigned int>& deck);
};
#endif // __DUEL_HPP__
