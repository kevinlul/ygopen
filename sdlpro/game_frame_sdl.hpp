#ifndef __GAME_FRAME_HPP__
#define __GAME_FRAME_HPP__
#include <SDL.h>
#include <SDL_ttf.h>

class DatabaseManager;
class CardTextureManager;

class GameFrame
{
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Event e;

	double deltaLast; // TEMP

	double deltaNow, deltaThen;
	double deltaTime;

	bool closing;
	int error; // non-zero == error

	bool isReplayMode; // Replay mode is for recording purposes

	DatabaseManager* dbm;
	CardTextureManager* ctm;

	TTF_Font* font;

	void UpdateDelta();

	void OnInit();

	void OnEvent();
	void OnStep();
	void OnDraw();

	void OnExit();
public:
	GameFrame(int argc, char* argv[]);
	
	int Execute();
};

#endif // __GAME_FRAME_HPP__
