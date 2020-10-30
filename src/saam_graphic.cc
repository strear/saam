#include "saam_graphic.hpp"
#include "saam_data.hpp"
#include <algorithm>
#include <cstring>
#include <cmath>

using namespace Saam;

ColorRgb& Saam::hslToRgb(ColorHsl& var) {
#define _div_255_fast(x)    (((x) + 1 + (((x) + 1) >> 8)) >> 8)

	if (var.s() == 0) {
		var.h() = var.s() = var.l();
		return (ColorRgb&)var;
	}

	auto hue2rgb = [](int p, int q, int t) -> int {
		if (t < 0) t += 256;
		if (t > 255) t -= 256;

		if (t < 256 / 6) {
			return p + _div_255_fast((q - p) * 6 * t);
		}
		if (t < 256 / 2) {
			return q;
		}
		if (t < 256 * 2 / 3) {
			return p + _div_255_fast((q - p) * (255 * 6 * 2 / 3 - 6 * t));
		}
		return p;
	};

	int q = var.l() < 256 / 2
		? var.l() + _div_255_fast(var.l() * var.s())
		: var.l() + var.s() - _div_255_fast(var.l() * var.s());
	int p = 2 * var.l() - q;

	var.l() = hue2rgb(p, q, var.h() - 256 / 3);
	var.s() = hue2rgb(p, q, var.h());
	var.h() = hue2rgb(p, q, var.h() + 256 / 3);

#undef _div_255_fast
	return (ColorRgb&)var;
}

inline ColorHsl& Saam::rgbToHsl(ColorRgb& var) {
	int mx = var.r() > var.g() ? var.r() > var.b()
		? var.r() : var.b() : var.b() > var.g() ? var.b() : var.g();
	int mn = var.r() < var.g() ? var.r() < var.b()
		? var.r() : var.b() : var.b() < var.g() ? var.b() : var.g();

	if (mx == mn) {
		var.g() = 0;
		return (ColorHsl&)var;
	}

	int d = mx - mn;
	int h;
	int s = 255 * d / (255 - abs(mx + mn - 255));
	int l = (mx + mn) / 2;

	if (mx == var.r()) {
		h = 255 * (var.g() - var.b()) / d + 255 * (var.g() < var.b() ? 6 : 0);
	} else if (mx == var.g()) {
		h = 255 * (var.b() - var.r()) / d + 255 * 2;
	} else {
		h = 255 * (var.r() - var.g()) / d + 255 * 4;
	}

	var.r() = h / 6;
	var.g() = s;
	var.b() = l;

	return (ColorHsl&)var;
}

byte Saam::grayScale(const ColorRgb& data, const int coefficient) {
	switch (coefficient) {
	case 601:
		return (299 * data.r() + 587 * data.g() + 114 * data.b() + 500) / 1000;
	case 709:
		return (2126 * data.r() + 7152 * data.g() + 722 * data.b() + 5000) / 10000;
	case 2020:
		return (2627 * data.r() + 6780 * data.g() + 593 * data.b() + 5000) / 10000;
	default:
		return (data.r() + data.r() + data.r() + data.b()
			+ data.g() + data.g() + data.g() + data.g()) >> 3;
	}
}

void Saam::getBlockC(const ColorRgba* data,
	size_t i, size_t j, size_t w, ColorHsl(&block)[8]) {
	i *= 2, j *= 4;

	memcpy(block + 0, (ColorHsl*)data + (j + 0) * w + i, 2 * sizeof(*block));
	memcpy(block + 2, (ColorHsl*)data + (j + 1) * w + i, 2 * sizeof(*block));
	memcpy(block + 4, (ColorHsl*)data + (j + 2) * w + i, 2 * sizeof(*block));
	memcpy(block + 6, (ColorHsl*)data + (j + 3) * w + i, 2 * sizeof(*block));

	for (int n = 0; n < 8; n++)
		rgbToHsl((ColorRgb&)block[n]);
}

void Saam::getBlockG(const ColorRgba* data,
	size_t i, size_t j, size_t w, byte(&block)[8], int lumaCoefficient) {
	i *= 2, j *= 4;

	for (int n = 0; n < 8; n++)
		block[n] = grayScale(data[(j + n / 2) * w + i + n % 2], lumaCoefficient);
}

void Saam::mix(const ColorHsl(&block)[8], ColorHsl& target) {
	int hSum = 0;

	for (int i = 0; i < 8; i++) {
		hSum += block[i].h();
		if (block[i].h() > block[0].h() + 128) {
			hSum -= 256;
		} else if (block[0].h() > block[i].h() + 128) {
			hSum += 256;
		}
	}
	hSum /= 8;
	if (hSum < 0) hSum += 256;

	target.h() = byte(hSum);

	target.s() = byte((int)(
		block[0].s() + block[1].s() + block[2].s() + block[3].s()
		+ block[4].s() + block[5].s() + block[6].s() + block[7].s()) / 8);

	target.l() = byte((int)(
		block[0].l() + block[1].l() + block[2].l() + block[3].l()
		+ block[4].l() + block[5].l() + block[6].l() + block[7].l()) / 8);
}

void Saam::mix(const byte(&block)[8], byte& target) {
	target = byte((int)(
		block[0] + block[1] + block[2] + block[3]
		+ block[4] + block[5] + block[6] + block[7]) / 8);
}

void Saam::valueC(const ColorHsl(&block)[8],
	char(&vars)[4], bool reverse, int fadeup) {

	if (reverse) {
		memset(vars, density_baseline + fadeup, 4);
		for (int i = 0; i < 8; i++)
			vars[i / 4 * 2 + i % 2] -= block[i].l() / (255 / density_baseline) / 2;
	} else {
		memset(vars, fadeup, 4);
		for (int i = 0; i < 8; i++)
			vars[i / 4 * 2 + i % 2] += block[i].l() / (255 / density_baseline) / 2;
	}

	for (int i = 0; i < 4; i++)
		vars[i] = vars[i] < 0 ? 0 : vars[i] > density_baseline ? density_baseline : vars[i];
}

void Saam::valueG(const byte(&block)[8],
	char(&vars)[4], bool reverse, int fadeup) {

	if (reverse) {
		memset(vars, density_baseline + fadeup, 4);
		for (int i = 0; i < 8; i++)
			vars[i / 4 * 2 + i % 2] -= block[i] / (255 / density_baseline) / 2;
	} else {
		memset(vars, fadeup, 4);
		for (int i = 0; i < 8; i++)
			vars[i / 4 * 2 + i % 2] += block[i] / (255 / density_baseline) / 2;
	}

	for (int i = 0; i < 4; i++)
		vars[i] = vars[i] < 0 ? 0 : vars[i] > density_baseline ? density_baseline : vars[i];
}

void Saam::valueC(const ColorHsl(&block)[8], char(&vars)[4], const bool(&marks)[8]) {
	memset(vars, 0, 4);
	for (int i = 0; i < 8; i++)
		if (marks[i]) vars[i / 4 * 2 + i % 2] += block[i].l() / (255 / density_baseline) / 2;

	for (int i = 0; i < 4; i++)
		vars[i] = vars[i] > density_baseline ? density_baseline : vars[i];
}

void Saam::valueG(const byte(&block)[8], char(&vars)[4], const bool(&marks)[8]) {
	memset(vars, 0, 4);
	for (int i = 0; i < 8; i++)
		if (marks[i]) vars[i / 4 * 2 + i % 2] += block[i] / (255 / density_baseline) / 2;

	for (int i = 0; i < 4; i++)
		vars[i] = vars[i] > density_baseline ? density_baseline : vars[i];
}

bool Saam::differentiate(const ColorHsl(&block)[8],
	ColorHsl& bg, ColorHsl& fg, const bool(&marks)[8]) {

	int fgCount = 0;

	for (int i = 0; i < 8; i++)
		if (marks[i]) fgCount++;

	if (!fgCount || !(8 - fgCount)) return false;

	int bgH = 0, fgH = 0, bgHref = -1, fgHref = -1;
	int bgS = 0, bgL = 0, fgS = 0, fgL = 0;

	for (int i = 0; i < 8; i++)
		if (marks[i]) {
			fgH += block[i].h();
			if (fgHref == -1) {
				fgHref = block[i].h();
			} else if (block[i].h() > fgHref + 128) {
				fgH -= 256;
			} else if (fgHref > block[i].h() + 128) {
				fgH += 256;
			}

			fgS += block[i].s();
			fgL += block[i].l();
		} else {
			bgH += block[i].h();
			if (bgHref == -1) {
				bgHref = block[i].h();
			} else if (block[i].h() > bgHref + 128) {
				bgH -= 256;
			} else if (bgHref > block[i].h() + 128) {
				bgH += 256;
			}

			bgS += block[i].s();
			bgL += block[i].l();
		}

	if (bgH < 0) bgH += 256;
	if (fgH < 0) fgH += 256;

	bg.h() = byte(bgH / (8 - fgCount));
	bg.s() = byte(bgS / (8 - fgCount));
	bg.l() = byte(bgL / (8 - fgCount));

	fg.h() = byte(fgH / fgCount);
	fg.s() = byte(fgS / fgCount);
	fg.l() = byte(fgL / fgCount);

	return true;
}

bool Saam::differentiate(const byte(&block)[8], byte& bg, byte& fg, const bool(&marks)[8]) {
	int bgSum = 0, fgSum = 0, fgCount = 0;

	for (int i = 0; i < 8; i++)
		if (marks[i]) fgCount++;

	if (!fgCount || !(8 - fgCount)) return false;

	for (int i = 0; i < 8; i++)
		(marks[i] ? fgSum : bgSum) += block[i];

	bg = bgSum / (8 - fgCount);
	fg = fgSum / fgCount;

	return true;
}

void Saam::seperateCharOut(const ColorHsl(&block)[8], ColorRgb& bg, ColorRgb& fg,
	char& singleChar, const unsigned char& shadingThreshold) {

	ColorHsl& bgHsl = (ColorHsl&)bg;
	ColorHsl& fgHsl = (ColorHsl&)fg;

	mix(block, bgHsl);

	bool fgMarks[8] = {};
	for (int i = 0; i < 8; i++) {
		if (abs(block[i].h() - bgHsl.h()) >= shadingThreshold)
			fgMarks[i] = true;
	}

	bool diff = differentiate(block, bgHsl, fgHsl, fgMarks);

	hslToRgb(bgHsl);

	if (diff) {
		char vars[4];
		valueC(block, vars, fgMarks);
		singleChar = getCharByShape(vars);
		hslToRgb(fgHsl);
	} else {
		singleChar = ' ';
	}
}

void Saam::seperateCharOut(const byte(&block)[8], ColorRgb& bg, ColorRgb& fg,
	char& singleChar, const unsigned char& shadingThreshold) {

	byte& bgLum = bg.r();
	byte& fgLum = fg.r();

	mix(block, bgLum);

	bool fgMarks[8] = {};
	for (int i = 0; i < 8; i++) {
		if (abs(block[i] - bgLum) >= shadingThreshold)
			fgMarks[i] = true;
	}

	bool diff = differentiate(block, bgLum, fgLum, fgMarks);

	bg.b() = bg.g() = bgLum;

	if (diff) {
		char vars[4];
		valueG(block, vars, fgMarks);
		singleChar = getCharByShape(vars);
		fg.b() = fg.g() = fgLum;
	} else {
		singleChar = ' ';
	}
}

void Saam::projectCharIn(const ColorHsl(&block)[8],
	bool reverse, int fadeup, ColorRgb& fg, char& singleChar) {

	ColorHsl& fgHsl = (ColorHsl&)fg;
	mix(block, fgHsl);

	if (fgHsl.l() != (reverse ? 255 : 0)) {
		char vars[4];
		valueC(block, vars, reverse, fadeup);
		singleChar = getCharByShape(vars);
		fgHsl.l() = byte(sqrt(fgHsl.l()) * 16);
	} else {
		singleChar = ' ';
	}

	hslToRgb(fgHsl);
}

void Saam::projectCharIn(const byte(&block)[8],
	bool reverse, int fadeup, ColorRgb& fg, char& singleChar) {

	byte& fgLum = fg.r();
	mix(block, fgLum);

	if (fgLum != (reverse ? 255 : 0)) {
		char vars[4];
		valueG(block, vars, reverse, fadeup);
		singleChar = getCharByShape(vars);
		fgLum = byte(sqrt(fgLum) * 16);
	} else {
		singleChar = ' ';
	}

	fg.b() = fg.g() = fgLum;
}

void Saam::projectImg(TextFramebuffer& framebuffer,
	ColorRgba* img, size_t w, size_t h,
	bool colorize, bool shading, bool reverse, int density,
	unsigned char shadingThreshold, int lumaCoefficient) {

	if (w == 0 || h == 0) return;

	short edgeX = framebuffer.getWidth();
	short edgeY = framebuffer.getHeight();
	short originX = (edgeX - short(w) / 2) / 2;
	short originY = (edgeY - short(h) / 4) / 2;
	if (originX < 0) originX = 0;
	if (originY < 0) originY = 0;
	edgeX -= originX;
	edgeY -= originY;

	char singleChar;
	ColorRgb bg, fg;

	bg.r() = bg.g() = bg.b() = reverse ? 255 : 0;

	if (colorize) {
		ColorHsl block[8];

		if (shading) {
			for (unsigned short j = 0; j < h / 4; j++) {
				if (j >= edgeY) break;
				for (unsigned short i = 0; i < w / 2; i++) {
					if (i >= edgeX) break;
					getBlockC(img, i, j, w, block);
					seperateCharOut(block, bg, fg, singleChar, shadingThreshold);
					framebuffer.put(originX + i, originY + j, singleChar, fg, bg);
				}
			}
		} else {
			const int fadeup = (density - 50) * (glyph_coverage - density_baseline) / 100;

			for (unsigned short j = 0; j < h / 4; j++) {
				if (j >= edgeY) break;
				for (unsigned short i = 0; i < w / 2; i++) {
					if (i >= edgeX) break;
					getBlockC(img, i, j, w, block);
					projectCharIn(block, reverse, fadeup, fg, singleChar);
					framebuffer.put(originX + i, originY + j, singleChar, fg, bg);
				}
			}
		}
	} else {
		byte block[8];

		if (shading) {
			for (unsigned short j = 0; j < h / 4; j++) {
				if (j >= edgeY) break;
				for (unsigned short i = 0; i < w / 2; i++) {
					if (i >= edgeX) break;
					getBlockG(img, i, j, w, block, lumaCoefficient);
					seperateCharOut(block, bg, fg, singleChar, shadingThreshold);
					framebuffer.put(originX + i, originY + j, singleChar, fg, bg);
				}
			}
		} else {
			const int fadeup = (density - 50) * (glyph_coverage - density_baseline) / 100;

			for (unsigned short j = 0; j < h / 4; j++) {
				if (j >= edgeY) break;
				for (unsigned short i = 0; i < w / 2; i++) {
					if (i >= edgeX) break;
					getBlockG(img, i, j, w, block, lumaCoefficient);
					projectCharIn(block, reverse, fadeup, fg, singleChar);
					framebuffer.put(originX + i, originY + j, singleChar, fg, bg);
				}
			}
		}
	}
}

void Saam::projectImgMono(TextFramebuffer& framebuffer,
	ColorRgba* img, size_t w, size_t h,
	bool reverse, int density, int lumaCoefficient) {

	if (w == 0 || h == 0) return;

	short edgeX = framebuffer.getWidth();
	short edgeY = framebuffer.getHeight();
	short originX = (edgeX - short(w) / 2) / 2;
	short originY = (edgeY - short(h) / 4) / 2;
	if (originX < 0) originX = 0;
	if (originY < 0) originY = 0;
	edgeX -= originX;
	edgeY -= originY;

	ColorRgb bg, fg;
	if (reverse) {
		fg = {};
		bg.r() = bg.g() = bg.b() = 255;
	} else {
		bg = {};
		fg.r() = fg.g() = fg.b() = 255;
	}

	const int fadeup = (density - 50) * (glyph_coverage - density_baseline) / 100;

	char singleChar;
	byte block[8];
	char vars[4];

	for (unsigned short j = 0; j < h / 4; j++) {
		if (j >= edgeY) break;
		for (unsigned short i = 0; i < w / 2; i++) {
			if (i >= edgeX) break;
			getBlockG(img, i, j, w, block, lumaCoefficient);
			valueG(block, vars, reverse, fadeup);
			singleChar = getCharByShape(vars);

			framebuffer.put(originX + i, originY + j, singleChar, fg, bg);
		}
	}
}