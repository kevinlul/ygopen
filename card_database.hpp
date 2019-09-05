#ifndef CARD_DATABASE_HPP
#define CARD_DATABASE_HPP
#include <string_view>
#include <unordered_map>

#include "pod_card.hpp"

struct sqlite3;
struct sqlite3_stmt;

namespace YGOpen
{

class CardDatabase
{
public:
	// Creates a in-memory database
	CardDatabase();
	
	// Opens or creates a disk database
	CardDatabase(std::string_view absFilePath);
	
	~CardDatabase();
	
	// Add a new database to the amalgation
	bool Merge(std::string_view absFilePath);
	
	// Retrieve card ready to be used by the core API
	CardData CardDataFromCode(unsigned int code, bool& found);
protected:
	sqlite3* db{nullptr};
private:
	sqlite3_stmt* aStmt{nullptr};
	sqlite3_stmt* sStmt{nullptr};
};

} // namespace YGOpen

#endif // CARD_DATABASE_HPP
