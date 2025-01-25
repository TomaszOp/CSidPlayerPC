#ifndef SDL_HELPER_H
#define SDL_HELPER_H

#include <SDL_config.h>
#include <SDL.h>
#include <SDL_audio.h>


class SDL_Helper
{
public:
	SDL_Helper();

	SDL_AudioSpec soundspec;

	int SDL__Init(Uint32 flags);
	int SDL__OpenAudio(SDL_AudioSpec* desired, SDL_AudioSpec* obtained);
	void SDL__PauseAudio(int pause_on);
	void SDL__CloseAudio(void);
	const char* SDL__GetError(void);


private:


};

#endif