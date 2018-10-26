#ifndef __DECK_HPP__
#define __DECK_HPP__
#include <vector>

namespace YGOpen
{

class DatabaseManager;
class Banlist;

class Deck
{
	bool verified; // The deck doesn't have any missing cards
	bool usable; // The deck doesn't have any banned/over limit card
public:
	Deck();

	std::vector<unsigned int> main;
	std::vector<unsigned int> extra;
	std::vector<unsigned int> side;

	unsigned int Verify(DatabaseManager& dbm);
	bool IsVerified() const;

	unsigned int CheckUsability(Banlist& bl, const int minmd = 40, const int maxmd = 60, const int mined = 0, const int maxed = 15, const int minsd = 0, const int maxsd = 15);
	bool CanBeUsed() const;
};

} // namespace YGOpen

#endif // __DECK_HPP__
