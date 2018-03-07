#include "card_texture_manager.hpp"
#include <SDL_image.h>

static const char* card_folders[ ] =
{
	"pics/%d.png",
	"pics/%d.jpg",
	"expansions/pics/%d.png",
	"expansions/pics/%d.jpg",
};

SDL_Texture* CardTextureManager::GetTextureFromCode(unsigned int code) const
{
	if(renderer == nullptr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "CardTextureManager: Tried to load texture when renderer is null.");
		return nullptr;
	}

	char file[256];
	SDL_Surface* surface;
	for(unsigned int i = 0; i < sizeof(card_folders) / sizeof(const char*); ++i)
	{
		sprintf(file, card_folders[i], code);
		surface = IMG_Load(file);
		if(surface != nullptr)
			break;
		else
			puts(IMG_GetError());
	}

	if(surface == nullptr)
		return nullptr;

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	if(texture == nullptr)
		puts(SDL_GetError());

	SDL_FreeSurface(surface);
	return texture;
}

CardTextureManager::CardTextureManager() : errorTexture(nullptr), renderer(nullptr)
{}

CardTextureManager::~CardTextureManager()
{
	for(auto iter : textures)
		SDL_DestroyTexture(iter.second);
}

void CardTextureManager::SetRenderer(SDL_Renderer* r)
{
	renderer = r;
}

SDL_Texture* CardTextureManager::GetCardTextureByCode(unsigned int code) const
{
	auto search = textures.find(code);
	// Texture is already loaded
	if(search != textures.end())
		return search->second;
	
	// Texture was attempted to load but failed before
	if(error_cards.find(code) != error_cards.end())
		return errorTexture;

	// Attempt to load file and create texture
	SDL_Texture* tex = GetTextureFromCode(code);
	if(tex != nullptr)
	{
		textures.insert(std::make_pair(code, tex));
		return tex;
	}

	// Otherwise add to the failed list
	error_cards.insert(code);
	return errorTexture;
}
