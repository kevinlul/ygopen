#include "core_interface.hpp"

#include <cstdio>
#include <SDL.h>

//TODO: Use a DEFINE macro on premake instead, and fallback to these values
#if defined(__LINUX__) || defined(__MACOSX__)
static const char* DEFAULT_CORE_NAME = "libygopen-core.so";
#else
static const char* DEFAULT_CORE_NAME = "ygopen-core.dll";
#endif

template<typename T>
T CoreInterface::LoadFunction(void* handle, T* func, const char* name, bool unload)
{
	*func = (T)SDL_LoadFunction(handle, name);
	if(*func == nullptr)
	{
		std::puts(SDL_GetError());
		if(unload)
			UnloadLibrary();
	}
	
	return *func;
}

CoreInterface::CoreInterface(bool loadLibrary) :
	activeCorePath(""),
	handle(nullptr)
{
	if(loadLibrary)
		LoadLibrary();
}

bool CoreInterface::LoadLibrary(const char* path)
{
	UnloadLibrary();

	std::string usedPath = path;

	handle = SDL_LoadObject(usedPath.c_str());
	if(handle == nullptr)
	{
		usedPath = SDL_GetBasePath();
		usedPath += path;

		handle = SDL_LoadObject(usedPath.c_str());
	}

	if(handle == nullptr)
	{
		std::puts(SDL_GetError());
		return false;
	}

#define LF(x) if(LoadFunction(handle, &x, #x, true) == nullptr) \
              	return false;

	LF(set_script_reader)
	LF(set_card_reader)
	LF(set_message_handler)

	LF(create_duel)
	LF(start_duel)
	LF(end_duel)
	LF(set_player_info)
	LF(get_log_message)
	LF(get_message)
	LF(process)
	LF(new_card)
	LF(new_tag_card)
	LF(new_relay_card)
	LF(query_card)
	LF(query_field_count)
	LF(query_field_card)
	LF(query_field_info)
	LF(set_responsei)
	LF(set_responseb)
	LF(preload_script)

#undef LF

	activeCorePath = usedPath; 
	return true;
}

bool CoreInterface::LoadLibrary()
{
	return LoadLibrary(DEFAULT_CORE_NAME);
}

bool CoreInterface::ReloadLibrary()
{
	if(handle)
	{
		std::string corePath = activeCorePath;
		UnloadLibrary();
		return LoadLibrary(corePath.c_str());
	}

	puts("Core was not initially loaded. Reload is not possible");

	return false;
}

bool CoreInterface::IsLibraryLoaded()
{
	return (bool)handle;
}

CoreInterface::~CoreInterface()
{
	UnloadLibrary();
}

void CoreInterface::UnloadLibrary()
{
	if(handle)
		SDL_UnloadObject(handle);
	handle = nullptr;
	activeCorePath = "";
}
