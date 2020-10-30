#include "saam.hpp"
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <future>
#include <functional>

using namespace Saam;

namespace {
	struct ImplData {
		char* dataGrid = nullptr;
		ColorRgb* frontColorGrid = nullptr;
		ColorRgb* backColorGrid = nullptr;
		char* outputSequence = nullptr;

		size_t sequenceLength = 0;
		size_t width = 0, height = 0;
		size_t gridCapacity = 0;

		~ImplData();
		void updateBound(size_t width, size_t height);
		void flushC();
		void flushM();

		std::future<void> worker;
	};

	void ImplData::updateBound(size_t w, size_t h) {
		if (this->width == w && this->height == h) return;

		const size_t area = w * h;
		this->width = w;
		this->height = h;
		sequenceLength = area * 128;

		if (dataGrid == nullptr || gridCapacity < area) {
			dataGrid = (char*)malloc(area * sizeof(char));
			frontColorGrid = (ColorRgb*)malloc(area * sizeof(ColorRgb));
			backColorGrid = (ColorRgb*)malloc(area * sizeof(ColorRgb));
			outputSequence = (char*)malloc(sequenceLength * sizeof(char));
		}
		else if (gridCapacity < area) {
			dataGrid = (char*)realloc(dataGrid, area * sizeof(char));
			frontColorGrid = (ColorRgb*)realloc(frontColorGrid, area * sizeof(ColorRgb));
			backColorGrid = (ColorRgb*)realloc(backColorGrid, area * sizeof(ColorRgb));
			outputSequence = (char*)realloc(outputSequence, sequenceLength * sizeof(char));
		}

		gridCapacity = area;
	}

	ImplData::~ImplData() {
		free(dataGrid);
		free(frontColorGrid);
		free(backColorGrid);
		free(outputSequence);
	}
}

#define impl_typed ((ImplData*)impl)

TextFramebuffer::TextFramebuffer()
	: impl(new ImplData()) {}

TextFramebuffer::~TextFramebuffer() {
	delete (ImplData*)impl;
}

void TextFramebuffer::prepare() {
	const size_t area = impl_typed->width * impl_typed->height;

	memset(impl_typed->dataGrid, ' ', area);
	memset(impl_typed->frontColorGrid, 0, area * 3);
	memset(impl_typed->backColorGrid, 0, area * 3);

	impl_typed->updateBound(getWidth(), getHeight());
}

void TextFramebuffer::flush(bool monochrome, bool wait) {
	if (impl_typed->worker.valid()
		&& impl_typed->worker.wait_for(std::chrono::seconds::zero())
		!= std::future_status::ready) return;

	impl_typed->worker = std::async(std::launch::async,
		monochrome ? std::bind(ImplData::flushM, impl_typed)
		: std::bind(ImplData::flushC, impl_typed));

	if (wait) impl_typed->worker.wait();
}

void TextFramebuffer::put(
	const int x, const int y, const char ch,
	const ColorRgb fg, const ColorRgb bg) {

	impl_typed->dataGrid[y * impl_typed->width + x] = ch;
	impl_typed->frontColorGrid[y * impl_typed->width + x] = fg;
	impl_typed->backColorGrid[y * impl_typed->width + x] = bg;
}

void TextFramebuffer::put(
	const int x, const int y,
	const char* str, const size_t len,
	const ColorRgb fg, const ColorRgb bg) {

	memcpy(impl_typed->dataGrid + (y * impl_typed->width + x), str, len);
	for (size_t i = x; i < len + x; i++) {
		const size_t colorIndex = (y * impl_typed->width + i);
		impl_typed->frontColorGrid[colorIndex] = fg;
		impl_typed->backColorGrid[colorIndex] = bg;
	}
}

namespace {
	void ImplData::flushC() {
		sequenceLength = 0;
		ColorRgb oldFg, oldBg;
		memset(&oldFg, -1, sizeof(ColorRgb));
		memset(&oldBg, -1, sizeof(ColorRgb));

		for (size_t i = 0; i < height; i++) {
			char posSequence[16] = "\x1b[001;001H";
			posSequence[2 + 2] = '0' + (i + 1) % 10;
			posSequence[2 + 1] = '0' + (i + 1) / 10 % 10;
			posSequence[2] = '0' + (char)(i + 1) / 100;

			memcpy(outputSequence + sequenceLength, posSequence, 10);
			sequenceLength += 10;
			for (size_t j = 0; j < width; j++) {
				const size_t dataIndex = i * width + j;

				if (dataGrid[dataIndex] == '\0') {
					dataGrid[dataIndex] = ' ';
				}

				int cmp1 = memcmp(&oldFg, frontColorGrid + dataIndex, sizeof(ColorRgb));
				int cmp2 = memcmp(&oldBg, backColorGrid + dataIndex, sizeof(ColorRgb));
				if (cmp1 != 0) {
					char sequence[] = "\x1b[38;2;000;000;000m";
					sequence[9] = '0' + frontColorGrid[dataIndex].r() % 10;
					sequence[8] = '0' + frontColorGrid[dataIndex].r() / 10 % 10;
					sequence[7] = '0' + frontColorGrid[dataIndex].r() / 100;

					sequence[13] = '0' + frontColorGrid[dataIndex].g() % 10;
					sequence[12] = '0' + frontColorGrid[dataIndex].g() / 10 % 10;
					sequence[11] = '0' + frontColorGrid[dataIndex].g() / 100;

					sequence[17] = '0' + frontColorGrid[dataIndex].b() % 10;
					sequence[16] = '0' + frontColorGrid[dataIndex].b() / 10 % 10;
					sequence[15] = '0' + frontColorGrid[dataIndex].b() / 100;

					memcpy(outputSequence + sequenceLength, sequence, 19);
					sequenceLength += 19;

					memcpy(&oldFg, frontColorGrid + dataIndex, sizeof(ColorRgb));
				}
				if (cmp2 != 0) {
					char sequence[] = "\x1b[48;2;000;000;000m";
					sequence[9] = '0' + backColorGrid[dataIndex].r() % 10;
					sequence[8] = '0' + backColorGrid[dataIndex].r() / 10 % 10;
					sequence[7] = '0' + backColorGrid[dataIndex].r() / 100;

					sequence[13] = '0' + backColorGrid[dataIndex].g() % 10;
					sequence[12] = '0' + backColorGrid[dataIndex].g() / 10 % 10;
					sequence[11] = '0' + backColorGrid[dataIndex].g() / 100;

					sequence[17] = '0' + backColorGrid[dataIndex].b() % 10;
					sequence[16] = '0' + backColorGrid[dataIndex].b() / 10 % 10;
					sequence[15] = '0' + backColorGrid[dataIndex].b() / 100;

					memcpy(outputSequence + sequenceLength, sequence, 19);
					sequenceLength += 19;

					memcpy(&oldBg, backColorGrid + dataIndex, sizeof(ColorRgb));
				}

				outputSequence[sequenceLength] = dataGrid[dataIndex];
				sequenceLength++;
			}
		}
		outputSequence[sequenceLength] = '\0';

		TextFramebuffer::_putstr(outputSequence, sequenceLength);
	}

	void ImplData::flushM() {
		TextFramebuffer::_putstr(dataGrid, width * height);
	}
}