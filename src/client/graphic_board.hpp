#pragma once
#include "board.hpp"
#include <SDL.h>
#include <SDL_ttf.h>
#include <atomic>
#include <mutex>
#include <condition_variable>

class CardTextureManager;

class DuelBoardSDL : public YGOpen::DuelBoard<YGOpen::Card>
{
	bool closing;
	double deltaLast; // TEMP
	
	double deltaNow, deltaThen;
	double deltaTime;

	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Event e;
	
	TTF_Font* font;
	
	CardTextureManager* ctm;
	
	SDL_Texture* field3;
	SDL_Texture* cover;
	SDL_Texture* unknown;
	
	void UpdateDelta();
	
	void OnEvent();
	void OnStep();
	void OnDraw();
public:
	std::mutex m1;
	
	int error; // non-zero == error
	DuelBoardSDL();
	~DuelBoardSDL();
	
	void Run();
};
