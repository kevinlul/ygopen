#include "card_database.hpp"

#include <stdexcept> // std::runtime_error
#include <sqlite3.h>

#include "enums/type.hpp"

namespace YGOpen
{

static const char* DB_SCHEMAS =
R"(
CREATE TABLE "datas" (
	"id"        INTEGER,
	"ot"        INTEGER,
	"alias"     INTEGER,
	"setcode"   INTEGER,
	"type"      INTEGER,
	"atk"       INTEGER,
	"def"       INTEGER,
	"level"     INTEGER,
	"race"      INTEGER,
	"attribute" INTEGER,
	"category"  INTEGER,
	PRIMARY KEY("id")
);

CREATE TABLE "texts" (
	"id"    INTEGER,
	"name"  TEXT,
	"desc"  TEXT,
	"str1"  TEXT,
	"str2"  TEXT,
	"str3"  TEXT,
	"str4"  TEXT,
	"str5"  TEXT,
	"str6"  TEXT,
	"str7"  TEXT,
	"str8"  TEXT,
	"str9"  TEXT,
	"str10" TEXT,
	"str11" TEXT,
	"str12" TEXT,
	"str13" TEXT,
	"str14" TEXT,
	"str15" TEXT,
	"str16" TEXT,
	PRIMARY KEY("id")
);
)";

static const char* ATTACH_STMT =
R"(
ATTACH ? AS toMerge;
)";

static const char* MERGE_DATAS_STMT =
R"(
INSERT OR REPLACE INTO datas SELECT * FROM toMerge.datas;
)";

static const char* MERGE_TEXTS_STMT =
R"(
INSERT OR REPLACE INTO texts SELECT * FROM toMerge.texts;
)";

static const char* DETACH_STMT =
R"(
DETACH toMerge;
)";

static const char* SEARCH_STMT =
R"(
SELECT * FROM datas WHERE datas.id = ?;
)";

CardDatabase::CardDatabase() : CardDatabase(":memory:")
{}

CardDatabase::CardDatabase(std::string_view absFilePath)
{
	// Create database
	if(sqlite3_open(absFilePath.data(), &db) != SQLITE_OK)
		throw std::runtime_error(sqlite3_errmsg(db));
	// Prepare database
	char* err = nullptr;
	if(sqlite3_exec(db, DB_SCHEMAS, nullptr, nullptr, &err) == SQLITE_ABORT)
	{
		std::string errStr(err);
		sqlite3_free(err);
		sqlite3_close(db);
		throw std::runtime_error(errStr);
	}
	// Prepare attach statement
	if(sqlite3_prepare_v2(db, ATTACH_STMT, -1, &aStmt, nullptr) != SQLITE_OK)
	{
		std::string errStr(sqlite3_errmsg(db));
		sqlite3_close(db);
		throw std::runtime_error(errStr);
	}
	// Prepare card data search by id statement
	if(sqlite3_prepare_v2(db, SEARCH_STMT, -1, &sStmt, nullptr) != SQLITE_OK)
	{
		std::string errStr(sqlite3_errmsg(db));
		sqlite3_finalize(aStmt);
		sqlite3_close(db);
		throw std::runtime_error(errStr);
	}
}

CardDatabase::~CardDatabase()
{
	sqlite3_finalize(sStmt);
	sqlite3_finalize(aStmt);
	sqlite3_close(db);
}

bool CardDatabase::Merge(std::string_view absFilePath)
{
	sqlite3_reset(aStmt);
	sqlite3_bind_text(aStmt, 1, absFilePath.data(), -1, SQLITE_TRANSIENT);
	if(sqlite3_step(aStmt) != SQLITE_DONE)
	{
		// NOTE: maybe print error?
		return false;
	}
	sqlite3_exec(db, MERGE_DATAS_STMT, nullptr, nullptr, nullptr);
	sqlite3_exec(db, MERGE_TEXTS_STMT, nullptr, nullptr, nullptr);
	sqlite3_exec(db, DETACH_STMT, nullptr, nullptr, nullptr);
	return true;
}

CardData CardDatabase::CardDataFromCode(unsigned int code, bool& found)
{
	CardData cd;
	sqlite3_reset(sStmt);
	sqlite3_bind_int(sStmt, 1, code);
	if(sqlite3_step(sStmt) == SQLITE_ROW)
	{
		cd.code = sqlite3_column_int(sStmt, 0);
		// Core doesnt use 1 (`ot`)
		cd.alias = sqlite3_column_int(sStmt, 2);
		cd.setcode = sqlite3_column_int64(sStmt, 3);
		cd.type = sqlite3_column_int(sStmt, 4);
		cd.attack = sqlite3_column_int(sStmt, 5);
		cd.defense = sqlite3_column_int(sStmt, 6);
		if(cd.type & TYPE_LINK)
		{
			cd.link_marker = cd.defense;
			cd.defense = 0;
		}
		else
		{
			cd.link_marker = 0;
		}
		unsigned int level = sqlite3_column_int(sStmt, 7);
		if((level & 0x80000000) != 0) // Negative levels are handled too
		{
			level = -level;
			cd.level = -(level & 0xFF);
		}
		else
		{
			cd.level = level & 0xFF;
		}
		cd.lscale = (level >> 24) & 0xFF;
		cd.rscale = (level >> 16) & 0xFF;
		cd.race = sqlite3_column_int(sStmt, 8);
		cd.attribute = sqlite3_column_int(sStmt, 9);
		found = true;
	}
	else
	{
		found = false;
	}
	return cd;
}

} // namespace YGOpen
