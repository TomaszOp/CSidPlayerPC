#include "SDL_Helper.h"

SDL_Helper::SDL_Helper(void)
{

}

int SDL_Helper::SDL__Init(Uint32 flags)
{
	return SDL_Init(flags);
}

int SDL_Helper::SDL__OpenAudio(SDL_AudioSpec* desired, SDL_AudioSpec* obtained)
{
	return SDL_OpenAudio(desired, obtained);
}

void SDL_Helper::SDL__PauseAudio(int pause_on)
{
	SDL_PauseAudio(pause_on);
}

void SDL_Helper::SDL__CloseAudio(void)
{
	SDL_CloseAudio();
}

const char* SDL_Helper::SDL__GetError(void)
{
	return SDL_GetError();
}