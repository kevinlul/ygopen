#include <sstream>
#include <SDL_image.h>
#include "graphic_board.hpp"
#include "card_texture_manager.hpp"

static const char* GAME_WINDOW_TITLE = "PERCYMERCURYEDOKOISHITDOANEINZEKTORPRO Es..?";

constexpr int FIELD_WIDTH = 512;
constexpr int FIELD_HEIGHT = 410;

constexpr int GAME_WIDTH = 512*2;
constexpr int GAME_HEIGHT = (410 + 38)*2;

DuelBoardSDL::DuelBoardSDL() : closing(false), error(0)
{
	// INIT SDL
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::printf("Could not initialize SDL: %s\n", SDL_GetError());
		error = 1;
		return;
	}
	
	window = SDL_CreateWindow(GAME_WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED,
	                          SDL_WINDOWPOS_UNDEFINED, GAME_WIDTH, GAME_HEIGHT,
	                          SDL_WINDOW_HIDDEN | SDL_WINDOW_ALLOW_HIGHDPI);
	
	if(window == nullptr)
	{
		std::printf("Could not create SDL Window: %s\n", SDL_GetError());
		error = 2;
		return;
	}
	
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED |
	                              SDL_RENDERER_PRESENTVSYNC);
	if(renderer == nullptr)
	{
		std::printf("Could not create SDL Renderer: %s\n", SDL_GetError());
		error = 3;
		return;
	}
	
	if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best"))
	    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	
	SDL_RenderSetLogicalSize(renderer, FIELD_WIDTH, FIELD_HEIGHT + 38); // TODO: check for errors?
	
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer); SDL_RenderPresent(renderer);
	
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	
	srand(SDL_GetTicks()); // TODO: use <random> instead
	
	// INIT TTF
	// TODO: shouldnt be here
	if(TTF_Init() != 0)
	{
		std::printf("Could not initialize TTF: %s\n", TTF_GetError());
		error = 4;
		return;
	}
	if(!(font = TTF_OpenFont("fonts/arialbd.ttf", 12)))
	{
		std::printf("Could not open font: %s\n", TTF_GetError());
		error = 5;
		return;
	}
	deltaLast = SDL_GetTicks();
	// End
	
	ctm = new CardTextureManager();
	ctm->SetRenderer(renderer);
	
	auto LoadImage = [this](SDL_Texture*& texture, const char* fn)
	{
		SDL_Surface* surface = IMG_Load(fn);
		texture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface);
	};
	
	LoadImage(field3, "field3.png");
	LoadImage(cover, "cover.jpg");
	LoadImage(unknown, "unknown.jpg");
	
	SDL_ShowWindow(window);
}

DuelBoardSDL::~DuelBoardSDL()
{
	if(ctm)
		delete ctm;
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void DuelBoardSDL::OnEvent()
{
	switch(e.type)
	{
		case SDL_QUIT:
			closing = true;
			break;
		case SDL_KEYDOWN:
			if(e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				closing = true;
			else if(e.key.keysym.scancode == SDL_SCANCODE_LEFT)
			{
				m1.lock();
				printf("state = %i; realtime = %i\n", GetCurrentState(),
				       IsRealtime());
				Backward();
				m1.unlock();
			}
			else if(e.key.keysym.scancode == SDL_SCANCODE_RIGHT)
			{
				m1.lock();
				printf("state = %i; realtime = %i\n", GetCurrentState(),
				       IsRealtime());
				Forward();
				m1.unlock();
			}
				
			break;
	}
}

void DuelBoardSDL::UpdateDelta()
{
	deltaNow = SDL_GetTicks();
	deltaTime = (deltaNow - deltaThen) / 1000.0;
	deltaThen = deltaNow;
}

void DuelBoardSDL::OnStep()
{
	UpdateDelta();
	
	if(deltaNow > deltaLast + 250.0f)
	{
		char title[255];
		std::sprintf(title, "%s | FPS: %.2f", GAME_WINDOW_TITLE, 1.0f / deltaTime);
		deltaLast = deltaNow;
// 		SDL_SetWindowTitle(window, title);
	}
}

void DuelBoardSDL::OnDraw()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	
	static SDL_Rect fieldRect{0, 19, FIELD_WIDTH, FIELD_HEIGHT};
	SDL_RenderCopy(renderer, field3, NULL, &fieldRect);
	
	SDL_Rect cardRect{0,0,37,56};
	
	auto DrawText = [&](int x, int y, const char* text, bool centered = true)
	{
		SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text, {255, 255, 255, 255});
		SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
		SDL_Rect textRect{x, y, surf->w, surf->h};
		if(centered)
		{
			textRect.x = x - (surf->w / 2);
			textRect.y = y - (surf->h / 2);
		}
		SDL_RenderCopy(renderer, tex, NULL, &textRect);
		SDL_DestroyTexture(tex);
		SDL_FreeSurface(surf);
	};
	
	auto DrawCardText = [&](int x, int y, const char* text)
	{
		SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text, {0, 0, 128, 255});
		SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
		SDL_Rect textRect{x, y, 37, 12};
		SDL_RenderCopy(renderer, tex, NULL, &textRect);
		SDL_DestroyTexture(tex);
		SDL_FreeSurface(surf);
	};
	
	auto DrawVerticalPile = [&](uint32_t controller, uint32_t location, int x, int y)
	{
		auto& pile = GetPile(controller, location);
		auto pileSize = pile.size();
		
		if(pileSize)
		{
			auto& c = pile.back();
			auto code = c.code();
			auto pos = c.pos();
			SDL_RendererFlip flip = (SDL_RendererFlip)SDL_FLIP_NONE;
			if(controller)
				flip = (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
			cardRect.x = x;
			cardRect.y = y;
			if(pos & PositionFaceDown)
				SDL_RenderCopyEx(renderer, cover, NULL, &cardRect, 0.0, NULL, flip);
			else if(code != 0)
				SDL_RenderCopyEx(renderer, ctm->GetCardTextureByCode(code), NULL, &cardRect, 0.0, NULL, flip);
			else
				SDL_RenderCopyEx(renderer, unknown, NULL, &cardRect, 0.0, NULL, flip);
		}
		char someText[32];
		sprintf(someText, "%lu", pileSize);
		DrawText(x + 18, y + 28, someText);
	};
	
	auto DrawHand = [&](uint32_t controller, bool reversed, int x, int y)
	{
		cardRect.x = x;
		cardRect.y = y;
		SDL_RendererFlip flip = (SDL_RendererFlip)SDL_FLIP_NONE;
		if(controller)
			flip = (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
		for(auto& c : GetPile(controller, LocationHand))
		{
			auto code = c.code();
			if(code != 0)
				SDL_RenderCopyEx(renderer, ctm->GetCardTextureByCode(code), NULL, &cardRect, 0.0, NULL, flip);
			else
				SDL_RenderCopyEx(renderer, unknown, NULL, &cardRect, 0.0, NULL, flip);
			
			std::stringstream ss;
			std::string str;
			ss.str(""); ss << "code: " << c.code(); str = ss.str();
			DrawCardText(cardRect.x, cardRect.y, str.c_str());
			ss.str(""); ss << "pos: " << c.pos(); str = ss.str();
			DrawCardText(cardRect.x, cardRect.y + 12, str.c_str());
			
			if(reversed)
				cardRect.x -= cardRect.w;
			else
				cardRect.x += cardRect.w;
		}
	};
	
	auto DrawFieldSingle = [&](YGOpen::Place p, int x, int y)
	{
		auto cit = fieldZones.find(p);
		if(cit != fieldZones.end())
		{
			auto& c = cit->second;
			auto code = c.code();
			auto pos = c.pos();
			double angle = 0;
			SDL_RendererFlip flip = (SDL_RendererFlip)SDL_FLIP_NONE;
			if(!(std::get<1>(p) & LocationSpellZone))
				angle = (pos & PositionDefense) ? -90.0 : 0.0;
			if(std::get<0>(p))
				flip = (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
			cardRect.x = x;
			cardRect.y = y;
			if(pos & PositionFaceDown)
				SDL_RenderCopyEx(renderer, cover, NULL, &cardRect, angle, NULL, flip);
			else if(code != 0)
				SDL_RenderCopyEx(renderer, ctm->GetCardTextureByCode(code), NULL, &cardRect, angle, NULL, flip);
			else
				SDL_RenderCopyEx(renderer, unknown, NULL, &cardRect, angle, NULL, flip);
			
			std::stringstream ss;
			std::string str;
			ss.str(""); ss << "code: " << c.code(); str = ss.str();
			DrawCardText(x, y, str.c_str());
			ss.str(""); ss << "pos: " << c.pos(); str = ss.str();
			DrawCardText(x, y + 12, str.c_str());
			int tmpY = y + 24;
			for(auto it = c.counters.begin(); it != c.counters.end(); it++)
			{
				ss.str(""); ss << "(" << it->first << ", " << it->second() << ")"; str = ss.str();
				DrawCardText(x, tmpY, str.c_str());
				tmpY += 12;
			}
		}
	};
	
	DrawVerticalPile(1, LocationMainDeck, 64, 8 + 19);
	DrawVerticalPile(1, LocationGraveyard, 64, 75 + 19);
	DrawVerticalPile(1, LocationBanished, 64, 143 + 19);
	DrawVerticalPile(1, LocationExtraDeck, 406, 8 + 19);
	DrawHand(1, true, 360, 0);
	
	DrawVerticalPile(0, LocationMainDeck, 406, 346 + 19);
	DrawVerticalPile(0, LocationGraveyard, 406, 279 + 19);
	DrawVerticalPile(0, LocationBanished, 406, 211 + 19);
	DrawVerticalPile(0, LocationExtraDeck, 64, 346 + 19);
	DrawHand(0, false, 109, 373 + 19);
	
	// player 0 mzones
	DrawFieldSingle({0, LocationMonsterZone, 0, -1}, 115 + 8, 245 + 2 + 19);
	DrawFieldSingle({0, LocationMonsterZone, 1, -1}, 171 + 8, 245 + 2 + 19);
	DrawFieldSingle({0, LocationMonsterZone, 2, -1}, 227 + 8, 245 + 2 + 19);
	DrawFieldSingle({0, LocationMonsterZone, 3, -1}, 284 + 8, 245 + 2 + 19);
	DrawFieldSingle({0, LocationMonsterZone, 4, -1}, 340 + 8, 245 + 2 + 19);
	DrawFieldSingle({0, LocationMonsterZone, 5, -1}, 171 + 8, 176 + 2 + 19);
	DrawFieldSingle({0, LocationMonsterZone, 6, -1}, 285 + 8, 176 + 2 + 19);
	
	// player 0 szones
	DrawFieldSingle({0, LocationSpellZone, 0, -1}, 115 + 8, 307 + 2 + 19);
	DrawFieldSingle({0, LocationSpellZone, 1, -1}, 171 + 8, 307 + 2 + 19);
	DrawFieldSingle({0, LocationSpellZone, 2, -1}, 227 + 8, 307 + 2 + 19);
	DrawFieldSingle({0, LocationSpellZone, 3, -1}, 284 + 8, 307 + 2 + 19);
	DrawFieldSingle({0, LocationSpellZone, 4, -1}, 340 + 8, 307 + 2 + 19);
	DrawFieldSingle({0, LocationSpellZone, 5, -1}, 64, 279 + 19);
	
	// player 1 mzones
	DrawFieldSingle({1, LocationMonsterZone, 0, -1}, 340 + 8, 105 + 2 + 19);
	DrawFieldSingle({1, LocationMonsterZone, 1, -1}, 284 + 8, 105 + 2 + 19);
	DrawFieldSingle({1, LocationMonsterZone, 2, -1}, 227 + 8, 105 + 2 + 19);
	DrawFieldSingle({1, LocationMonsterZone, 3, -1}, 171 + 8, 105 + 2 + 19);
	DrawFieldSingle({1, LocationMonsterZone, 4, -1}, 115 + 8, 105 + 2 + 19);
	DrawFieldSingle({1, LocationMonsterZone, 5, -1}, 285 + 8, 176 + 2 + 19);
	DrawFieldSingle({1, LocationMonsterZone, 6, -1}, 171 + 8, 176 + 2 + 19);
	
	// player 1 szones
	DrawFieldSingle({1, LocationSpellZone, 0, -1}, 115 + 8, 43 + 2 + 19);
	DrawFieldSingle({1, LocationSpellZone, 1, -1}, 171 + 8, 43 + 2 + 19);
	DrawFieldSingle({1, LocationSpellZone, 2, -1}, 227 + 8, 43 + 2 + 19);
	DrawFieldSingle({1, LocationSpellZone, 3, -1}, 284 + 8, 43 + 2 + 19);
	DrawFieldSingle({1, LocationSpellZone, 4, -1}, 340 + 8, 43 + 2 + 19);
	DrawFieldSingle({1, LocationSpellZone, 5, -1}, 406, 75 + 19);
	
	std::stringstream ss;
	std::string str;
	ss.str(""); ss << "Turn: " << turn; str = ss.str();
	DrawText(0, (FIELD_HEIGHT/2) - 28, str.c_str(), false);
	ss.str(""); ss << "Turn Player: " << turnPlayer(); str = ss.str();
	DrawText(0, (FIELD_HEIGHT/2) - 14, str.c_str(), false);
	ss.str(""); ss << "Phase: " << phase(); str = ss.str();
	DrawText(0, (FIELD_HEIGHT/2), str.c_str(), false);
	ss.str(""); ss << "Player 0 LP: " << playerLP[0](); str = ss.str();
	DrawText(0, (FIELD_HEIGHT/2) + 14, str.c_str(), false);
	ss.str(""); ss << "Player 1 LP: " << playerLP[1](); str = ss.str();
	DrawText(0, (FIELD_HEIGHT/2) + 28, str.c_str(), false);
	
	SDL_RenderPresent(renderer);
}

void DuelBoardSDL::Run()
{
	while(!closing && error == 0)
	{
		while(SDL_PollEvent(&e))
			OnEvent();
		
		if(m1.try_lock())
		{
			OnStep();
			OnDraw();
			m1.unlock();
		}
	}
}
