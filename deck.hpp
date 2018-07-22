#ifndef __DECK_HPP__
#define __DECK_HPP__
#include <vector>

class DatabaseManager;
class Banlist;

class Deck
{
	bool verified; // The deck doesn't have any missing cards
	bool usable; // The deck doesn't have any banned/over limit card
	std::vector<unsigned int> main;
	std::vector<unsigned int> extra;
	std::vector<unsigned int> side;
public:
	Deck();
	
	void SetMainDeck(const std::vector<unsigned int>& deck);
	void SetExtraDeck(const std::vector<unsigned int>& deck);
	void SetSideDeck(const std::vector<unsigned int>& deck);

	unsigned int Verify(DatabaseManager* dbm);
	bool IsVerified() const;

	unsigned int CheckUsability(Banlist* bl, const int minmd = 40, const int maxmd = 60, const int mined = 0, const int maxed = 15, const int minsd = 0, const int maxsd = 15);
	bool CanBeUsed() const;
};
#endif // __DECK_HPP__
