#pragma once

#include <cstddef>
#include <cstdint>

namespace Saam {
	using byte = uint8_t;

	struct Pixel {
		byte v0 = 0, v1 = 0, v2 = 0, v3 = 0;
	};

	struct ColorRgb : Pixel {
		constexpr byte& r() { return v0; }
		constexpr byte& g() { return v1; }
		constexpr byte& b() { return v2; }
		constexpr const byte& r() const { return v0; }
		constexpr const byte& g() const { return v1; }
		constexpr const byte& b() const { return v2; }
	};

	struct ColorRgba : ColorRgb {
		constexpr byte& a() { return v3; }
		constexpr const byte& a() const { return v3; }
	};

	struct ColorHsl : Pixel {
		constexpr byte& h() { return v0; }
		constexpr byte& s() { return v1; }
		constexpr byte& l() { return v2; }
		constexpr const byte& h() const { return v0; }
		constexpr const byte& s() const { return v1; }
		constexpr const byte& l() const { return v2; }
	};

	class TextFramebuffer {
	private:
		void* impl = nullptr;
		TextFramebuffer(const TextFramebuffer&) = delete;

	public:
		TextFramebuffer();
		~TextFramebuffer();

		void ready();
		void flush(bool monochrome = false, bool wait = false);
		size_t getWidth();
		size_t getHeight();

		static size_t getViewportWidth();
		static size_t getViewportHeight();
		static bool isMonochrome();
		static void _putframe(const char*, size_t len);

		void put(const int x, const int y, const char,
			const ColorRgb fg, const ColorRgb bg);

		void put(const int x, const int y,
			const char* str, const size_t len,
			const ColorRgb fg, const ColorRgb bg);
	};

	char getCharByShape(const char(&parts)[4]);

	void projectImg(TextFramebuffer& framebuffer,
		ColorRgba* img, size_t w, size_t h,
		const bool colorize = true, const bool shading = true,
		const bool reverse = false, const int density = 50,
		const unsigned char shadingThreshold = 1, const int lumaCoefficient = 0);

	void projectImgMono(TextFramebuffer& framebuffer,
		ColorRgba* img, size_t w, size_t h,
		const bool reverse = false, const int density = 50,
		const int lumaCoefficient = 0);
}