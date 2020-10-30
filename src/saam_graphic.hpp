#pragma once

#include "saam.hpp"

namespace Saam {
	inline ColorRgb& hslToRgb(ColorHsl&);
	inline ColorHsl& rgbToHsl(ColorRgb&);

	byte grayScale(const ColorRgb& data, const int coefficient = 0);

	void getBlockC(const ColorRgba* data,
		size_t i, size_t j, size_t w, ColorHsl(&block)[8]);
	void getBlockG(const ColorRgba* data,
		size_t i, size_t j, size_t w, byte(&block)[8], int lumaCoefficient);

	void mix(const ColorHsl(&block)[8], ColorHsl& target);
	void mix(const byte(&block)[8], byte& target);
	void valueC(const ColorHsl(&block)[8], char(&vars)[4], const bool(&marks)[8]);
	void valueG(const byte(&block)[8], char(&vars)[4], const bool(&marks)[8]);
	void valueC(const ColorHsl(&block)[8], char(&vars)[4],
		const bool reverse, const int fadeup);
	void valueG(const byte(&block)[8], char(&vars)[4],
		const bool reverse, const int fadeup);

	bool differentiate(const ColorHsl(&block)[8],
		ColorHsl& bg, ColorHsl& fg, const bool(&marks)[8]);
	bool differentiate(const byte(&block)[8],
		byte& bg, byte& fg, const bool(&marks)[8]);

	void seperateCharOut(const ColorHsl(&block)[8],
		ColorRgb& bg, ColorRgb& fg, char& singleChar,
		const unsigned char& shadingThreshold);
	void seperateCharOut(const byte(&block)[8],
		ColorRgb& bg, ColorRgb& fg, char& singleChar,
		const unsigned char& shadingThreshold);

	void projectCharIn(const ColorHsl(&block)[8],
		const bool reverse, const int fadeup, ColorRgb& fg, char& singleChar);
	void projectCharIn(const byte(&block)[8],
		const bool reverse, const int fadeup, ColorRgb& fg, char& singleChar);
}