#ifndef __CARD_TEXTURE_MANAGER__
#define __CARD_TEXTURE_MANAGER__
#include <unordered_set>
#include <unordered_map>
#include <SDL.h>

class CardTextureManager
{
	std::unordered_set<unsigned int> error_cards;
	std::unordered_map<unsigned int, SDL_Texture*> textures;
	SDL_Texture* errorTexture;

	SDL_Renderer* renderer;

	SDL_Texture* GetTextureFromCode(unsigned int);
public:
	CardTextureManager();
	~CardTextureManager();
	void SetRenderer(SDL_Renderer*);

	SDL_Texture* GetCardTextureByCode(unsigned int);
};

#endif //__CARD_TEXTURE_MANAGER__
