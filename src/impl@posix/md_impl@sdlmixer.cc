#include "../media.hpp"

#include <SDL_mixer.h>

using namespace Saam;

namespace {
	struct ImplData {
		Mix_Music* music;
	};
}

#define impl_typed ((ImplData*)impl)

Media::Media(const char* file) : impl(new ImplData()) {
	impl_typed->music = Mix_LoadMUS("music.mp3");
}

Media::~Media() {
	delete (ImplData*)impl;
}

void Media::play() {
	if (Mix_PausedMusic())
		Mix_ResumeMusic();
	else
		Mix_PlayMusic(impl_typed->music, 0);
}

void Media::pause() {
	Mix_PauseMusic();
}

void Media::reset() {
	Mix_SetMusicPosition(0);
}