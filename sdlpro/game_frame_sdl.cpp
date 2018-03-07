#include "game_frame_sdl.hpp"

#include <cstdio>

#include "database_manager.hpp"
#include "card_texture_manager.hpp"

static const char* GAME_WINDOW_TITLE        = "JADGE! Is This MERCURYEDOKOISHIDEVSALVATIONPROEs Percy TDOANE?";
static const int GAME_SCREEN_STARTING_WIDTH  = 1024;
static const int GAME_SCREEN_STARTING_HEIGHT = 640;

void GameFrame::UpdateDelta()
{
	deltaNow = SDL_GetTicks();
	deltaTime = (deltaNow - deltaThen) / 1000.0;
	deltaThen = deltaNow;
}

void GameFrame::OnInit()
{
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::printf("Could not initialize SDL: %s\n", SDL_GetError());
		error = 1;
		return;
	}

	window = SDL_CreateWindow(GAME_WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                          GAME_SCREEN_STARTING_WIDTH, GAME_SCREEN_STARTING_HEIGHT,
	                          SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	if(window == nullptr)
	{
		std::printf("Could not create SDL Window: %s\n", SDL_GetError());
		error = 2;
		return;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if(renderer == nullptr)
	{
		std::printf("Could not create SDL Renderer: %s\n", SDL_GetError());
		error = 3;
		return;
	}

	if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best"))
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	SDL_RenderSetLogicalSize(renderer, GAME_SCREEN_STARTING_WIDTH,
	                         GAME_SCREEN_STARTING_HEIGHT); // TODO: check for errors?

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer); SDL_RenderPresent(renderer);

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	srand(SDL_GetTicks()); // TODO: use <random> instead

	// TODO: This shouldn't be here
	if(TTF_Init() != 0)
	{
		std::printf("Could not initialize TTF: %s\n", TTF_GetError());
		error = 4;
		return;
	}
	if(!(font = TTF_OpenFont("fonts/arialbd.ttf", 16)))
	{
		std::printf("Could not open font: %s\n", TTF_GetError());
		error = 5;
		return;
	}
	deltaLast = SDL_GetTicks();
	// End
	
	dbm = new DatabaseManager();
	dbm->LoadDatabase("cards.cdb");

	ctm = new CardTextureManager();
	ctm->SetRenderer(renderer);

	SDL_ShowWindow(window);
}

void GameFrame::OnEvent()
{
	switch(e.type)
	{
		case SDL_QUIT:
			closing = true;
		break;
		case SDL_KEYDOWN:
			if(e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) // Exit the game by pressing ESC key
				closing = true;
			if(e.key.keysym.scancode == SDL_SCANCODE_SPACE)
				std::puts(dbm->GetCardStringsByCode(76794549)->desc.c_str());
		break;
	}
}

void GameFrame::OnStep()
{
	UpdateDelta();
	
	if(deltaNow > deltaLast + 250.0f)
	{
		char title[128];
		std::sprintf(title, "%s | FPS: %f", GAME_WINDOW_TITLE, 1.0f / deltaTime);
		deltaLast = deltaNow;
		SDL_SetWindowTitle(window, title);
	}
}

void GameFrame::OnDraw()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);


	SDL_RenderPresent(renderer);
}

void GameFrame::OnExit()
{
	if(dbm)
		delete dbm;
	if(ctm)
		delete ctm;
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

GameFrame::GameFrame(int argc, char* argv[]) : window(nullptr), renderer(nullptr),
                                               closing(false), error(0), isReplayMode(false),
                                               dbm(nullptr), ctm(nullptr)
{
	// TODO: turn on replay mode based on arguments
}

int GameFrame::Execute()
{
	OnInit();

	while(!closing && error == 0)
	{
		while(SDL_PollEvent(&e))
			OnEvent();
		OnStep();
		OnDraw();
	}

	OnExit();
	
	return error;
}
