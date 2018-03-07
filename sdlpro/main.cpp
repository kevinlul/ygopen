#include "core_interface.hpp"
#include "core_auxiliary.hpp"

#include "database_manager.hpp"

#include "duel.hpp"

int main(int argc, char* argv[])
{
	DatabaseManager dbm;
	dbm.LoadDatabase("cards.cdb");

	CoreAuxiliary::SetDatabaseManager(&dbm);

	CoreInterface ci(false);
	if(ci.LoadLibrary())
	{
		std::puts("Core loaded successfully");
		auto f = [](const char*, int*) -> unsigned char*
		{
			return (unsigned char*)"";
		};
		ci.set_script_reader(f);
		ci.set_card_reader(&CoreAuxiliary::CoreCardReader);
		ci.set_message_handler(&CoreAuxiliary::CoreMessageHandler);
		
		std::puts("Creating duel");
		Duel duel(&ci);
		std::puts("Starting duel");
		duel.SetPlayersInfo(8000, 0, 0);
		duel.Start(0x2810); // DUEL_PZONE + DUEL_EMZONE + NoShuffleDeck
		duel.Process();

		ci.UnloadLibrary();
	}

	return 0;
}
