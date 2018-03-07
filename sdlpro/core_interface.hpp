#ifndef __CORE_INTERFACE__
#define __CORE_INTERFACE__
#include <string>

#include "card.hpp"

typedef unsigned char* (*script_reader)(const char*, int*);
typedef unsigned int (*card_reader)(unsigned int, CardData*);
typedef unsigned int (*message_handler)(void*, unsigned int);

class CoreInterface
{
	std::string activeCorePath;
	void* handle;

	template<typename T>
	T LoadFunction(void* handle, T* func, const char* name, bool unload);
public:
	// Core loading
	CoreInterface(bool loadLibrary);
	bool LoadLibrary(const char* path);
	bool LoadLibrary();
	bool ReloadLibrary();

	bool IsLibraryLoaded();

	// Core functions
	void (*set_script_reader)(script_reader);
	void (*set_card_reader)(card_reader);
	void (*set_message_handler)(message_handler);

	long (*create_duel)(unsigned int);
	void (*start_duel)(long, int);
	void (*end_duel)(long);
	void (*set_player_info)(long, int, int, int, int);
	void (*get_log_message)(long, unsigned char*);
	int (*get_message)(long, unsigned char*);
	int (*process)(long);
	void (*new_card)(long, unsigned int, unsigned char, unsigned char, unsigned char, unsigned char, unsigned char);
	//void new_tag_card(long, unsigned int, unsigned char, unsigned char);
	//void new_relay_card(long, unsigned int, unsigned char, unsigned char, unsigned char);
	int (*query_card)(long, unsigned char, unsigned char, unsigned char, int, unsigned char*, int);
	int (*query_field_count)(long, unsigned char, unsigned char);
	int (*query_field_card)(long, unsigned char, unsigned char, int, unsigned char*, int);
	int (*query_field_info)(long, unsigned char*);
	void (*set_responsei)(long, int);
	void (*set_responseb)(long, unsigned char*);
	int (*preload_script)(long, char*, int);

	// Core extension functions
	//void (*query_version)(int*, int*, int*);
	//const char* (*query_version_string)(void);

	// Core unloading
	~CoreInterface();
	void UnloadLibrary(); 
};

#endif // __CORE_INTERFACE__
