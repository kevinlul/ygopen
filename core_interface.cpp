#include "core_interface.hpp"

#include <cstdio>

namespace YGOpen
{

//TODO: Use a DEFINE macro on premake instead, and fallback to these values
#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
static const char* DEFAULT_CORE_NAME = "ygopen-core.dll";
#else
static const char* DEFAULT_CORE_NAME = "./libygopen-core.so";
#endif


// "Native" functions below modified from implementations found in SDL2 library
// Author: Sam Lantinga <slouken@libsdl.org>
// Date: 26-10-2018
// Code version: Lastest version at the date (from hg repo)
// Availability:
//	https://hg.libsdl.org/SDL/raw-file/691c32a30fb9/src/loadso/windows/SDL_sysloadso.c
//	https://hg.libsdl.org/SDL/raw-file/691c32a30fb9/src/loadso/dlopen/SDL_sysloadso.c

// Prototypes
void* NativeLoadObject(const char* file);
void* NativeLoadFunction(void* handle, const char* name);
void  NativeUnloadObject(void* handle);

#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#include <windows.h>

// NOTE: Might need to handle unicode

void* NativeLoadObject(const char* file)
{
// NOTE: We might need a way to identify if the platform is WINRT
#ifdef __WINRT__
	/* WinRT only publically supports LoadPackagedLibrary() for loading .dll
		files.  LoadLibrary() is a private API, and not available for apps
		(that can be published to MS' Windows Store.)
	*/
	void* handle = (void*) LoadPackagedLibrary(tstr, 0);
#else
	void* handle = (void*) LoadLibrary(tstr);
#endif

	/* Generate an error message if all loads failed */
	if (handle == nullptr)
	{
		std::string errStr = std::string("Failed loading ") + file;
		printf(errStr.c_str());
	}
	return handle;
}

void* NativeLoadFunction(void* handle, const char* name)
{
	void* symbol = (void*) GetProcAddress((HMODULE) handle, name);
	if (symbol == nullptr)
	{
		std::string errStr = std::string("Failed loading ") + name;
		printf(errStr.c_str());
	}
	return symbol;
}

void  NativeUnloadObject(void* handle)
{
	if (handle != nullptr)
		FreeLibrary((HMODULE) handle);
}

#else
#include <dlfcn.h>

void* NativeLoadObject(const char* file)
{
	void* handle;
	const char* loaderror;

	/* possible IOS support in far future?
	#if SDL_VIDEO_DRIVER_UIKIT
	if (!UIKit_IsSystemVersionAtLeast(8.0)) {
		SDL_SetError("SDL_LoadObject requires iOS 8+");
		return NULL;
	}
	#endif
	*/

	handle = dlopen(file, RTLD_NOW|RTLD_LOCAL);
	loaderror = (char*)dlerror();
	if (handle == nullptr)
		printf("Failed loading %s: %s", file, loaderror);

	return (handle);
}

void* NativeLoadFunction(void* handle, const char* name)
{
	void* symbol = dlsym(handle, name);
	if (symbol == nullptr)
	{
		/* append an underscore for platforms that need that. */
		std::string _name = std::string("_") + name;
		symbol = dlsym(handle, _name.c_str());
		if (symbol == nullptr)
			printf("Failed loading %s: %s", name, (const char*)dlerror());
	}
	return (symbol);
}

void NativeUnloadObject(void* handle)
{
	if (handle != nullptr)
		dlclose(handle);
}
#endif

template<typename T>
T CoreInterface::LoadFunction(void* handle, T* func, const char* name, bool unload)
{
	*func = (T)NativeLoadFunction(handle, name);
	if(*func == nullptr && unload)
		UnloadLibrary();

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

	handle = NativeLoadObject(usedPath.c_str());
	/* TODO: get current working directory without SDL
	if(handle == nullptr)
	{
		usedPath = SDL_GetBasePath();
		usedPath += path;

		handle = NativeLoadObject(usedPath.c_str());
	}
	*/

	if(handle == nullptr)
		return false;

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
		NativeUnloadObject(handle);
	handle = nullptr;
	activeCorePath = "";
}

} // namespace YGOpen
