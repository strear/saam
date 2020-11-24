#include "../imageArray.hpp"

#include <SDL_image.h>

using namespace Saam;

namespace {
	struct ReadImpl {
		SDL_Surface* image = nullptr;
		SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
	};

	void quitWithError(const char* msg, const char* detail = nullptr) {
		fprintf(stderr, "[X] %s", msg);
		if (detail != nullptr) fprintf(stderr, " - %s", detail);
		fprintf(stderr, "\n");
		quick_exit(0xffffffff);
	}
}

#define impl_typed ((ReadImpl*)impl)

PicLoader::PicLoader()
	: impl(new ReadImpl()) {}

PicLoader::~PicLoader() {
	if (impl_typed->image != nullptr)
		SDL_FreeSurface(impl_typed->image);
	SDL_FreeFormat(impl_typed->format);

	delete impl_typed;
}

void PicLoader::loadPic(const char* file) {
	if (impl_typed->image != nullptr)
		SDL_FreeSurface(impl_typed->image);

	impl_typed->image = IMG_Load(file);
	if (!impl_typed->image) quitWithError("Load picture failed", IMG_GetError());

	auto converted_image = SDL_ConvertSurface(impl_typed->image, impl_typed->format, 0);
	SDL_FreeSurface(impl_typed->image);
	impl_typed->image = converted_image;
}

void PicLoader::getPixels(Array<uint8_t>& receiver) {
	uint8_t* pixels = (uint8_t*)impl_typed->image->pixels;
	size_t w = impl_typed->image->w, h = impl_typed->image->h;

	receiver = Array<uint8_t>(pixels, w * h * 4);
	receiver.reshape(h, w, 4);
}