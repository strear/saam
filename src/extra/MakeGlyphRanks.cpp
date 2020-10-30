#undef UNICODE
#include "..\..\..\..\1953610_Z_2\Malarga\include\easyx.h"
#include <cstdio>
#include <cstring>

namespace {
	const char* fontface = "Terminal";

	unsigned char density_baseline = 0;
	unsigned char glyph_shapes[127 - 32][4] = {};

	void getRanks() {
		settextstyle(20, 10, fontface, 0, 0, FW_NORMAL, 0, 0, 0);

		for (int i = 0; i < 127 - 32; i++) {
			outtextxy(i % 8 * 10, i / 8 * 20, i + 32);

			for (int ia = 0; ia < 4; ia++) {
				int x0 = ia % 2 * 5 + i % 8 * 10;
				int y0 = ia / 2 * 10 + i / 8 * 20;

				for (int k = 0; k < 50; k++) {
					if (getpixel(x0 + k % 5, y0 + k / 5) != 0)
						glyph_shapes[i][ia]++;
				}

				setfillcolor(HSLtoRGB(102.0f, 0.6f, glyph_shapes[i][ia] / 50.0f));
				solidrectangle(200 + x0, y0, 200 + x0 + 4, y0 + 9);

				if (glyph_shapes[i][ia] > density_baseline)
					density_baseline = glyph_shapes[i][ia];
			}
		}
	}

	struct GlyphList {
		byte count;
		char* glyphs = nullptr;

		GlyphList();
		GlyphList(GlyphList& src, byte begin, byte end);
		~GlyphList();

		int variance(byte dim);
		int pivot();
		void sort(byte dim, int l, int r);
	};

	byte currentNodeId = 0;

	struct GlyphKdNode {
		GlyphKdNode* left = nullptr, * right = nullptr;
		byte id;
		char glyph;
		byte splitdim = 0;

		GlyphKdNode(GlyphList& glyphs);
		~GlyphKdNode() {
			if (left != nullptr) delete left;
			if (right != nullptr) delete right;
		}
	};

	GlyphList::GlyphList() : count(127 - 32) {
		glyphs = new char[127 - 32];
		for (int i = 0; i < 127 - 32; i++) glyphs[i] = i + 32;
	}

	GlyphList::GlyphList(GlyphList& src, byte begin, byte end) : count(end - begin) {
		glyphs = new char[end - begin];
		memcpy(glyphs, src.glyphs + begin, end - begin);
	}

	GlyphList::~GlyphList() {
		if (glyphs != nullptr) delete[] glyphs;
	}

	int GlyphList::variance(byte dim) {
		if (count == 0) throw;

		int average = 0;
		for (int i = 0; i < count; i++) {
			average += glyph_shapes[(size_t)glyphs[i]][dim];
		}
		average /= count;

		int variance = 0;
		for (int i = 0; i < count; i++) {
			variance += (glyph_shapes[(size_t)glyphs[i]][dim] - average)
				* (glyph_shapes[(size_t)glyphs[i]][dim] - average);
		}
		return variance / count;
	}

	int GlyphList::pivot() {
		int maxVariance = variance(0);
		int compareVariance[3] = {
			variance(1), variance(2), variance(3)
		};
		int ans = 0;

		for (int i = 0; i < 3; i++) {
			if (compareVariance[i] > maxVariance) {
				maxVariance = compareVariance[i];
				ans = i + 1;
			}
		}

		return ans;
	}

	void GlyphList::sort(byte dim, int l, int r) {
		if (l < r) {
			int i = l, j = r;
			char x = glyphs[l];
			while (i < j) {
				while (i < j && glyph_shapes[glyphs[j] - 32][dim] >= glyph_shapes[x - 32][dim])
					j--;
				if (i < j)
					glyphs[i++] = glyphs[j];

				while (i < j && glyph_shapes[glyphs[i] - 32][dim] < glyph_shapes[x - 32][dim])
					i++;
				if (i < j)
					glyphs[j--] = glyphs[i];
			}
			glyphs[i] = x;
			sort(dim, l, i - 1);
			sort(dim, i + 1, r);
		}
	}

	GlyphKdNode::GlyphKdNode(GlyphList& glyphs) : id(currentNodeId++) {
		if (glyphs.count == 1) {
			glyph = glyphs.glyphs[0];
		} else {
			glyphs.sort(splitdim = glyphs.pivot(), 0, glyphs.count - 1);
			glyph = glyphs.glyphs[glyphs.count / 2];

			if (glyphs.count / 2 > 0) {
				GlyphList leftGlyphs(glyphs, 0, glyphs.count / 2);
				left = new GlyphKdNode(leftGlyphs);
			}

			if (glyphs.count / 2 + 1 < glyphs.count) {
				GlyphList rightGlyphs(glyphs, glyphs.count / 2 + 1, glyphs.count);
				right = new GlyphKdNode(rightGlyphs);
			}
		}
	}

	GlyphKdNode* tree;

	void printTreeNode(GlyphKdNode* node) {
		printf("		{ %2d, %2d, %1d, %3d, { %2d, %2d, %2d, %2d } },\n",
			node->left == nullptr ? -1 : node->left->id,
			node->right == nullptr ? -1 : node->right->id,
			node->splitdim, node->glyph,
			glyph_shapes[node->glyph - 32][0], glyph_shapes[node->glyph - 32][1],
			glyph_shapes[node->glyph - 32][2], glyph_shapes[node->glyph - 32][3]);

		if (node->left != nullptr) printTreeNode(node->left);
		if (node->right != nullptr) printTreeNode(node->right);
	}

	int log2(int x) {
		float fx;
		unsigned long ix, exp;

		fx = (float)x;
		ix = *(unsigned long*)&fx;
		exp = (ix >> 23) & 0xFF;

		return exp - 127;
	}

	void output() {
		printf("#pragma once\n");
		printf("\n");
		printf("// Generated by MakeGlyphRanks.cpp\n");
		printf("// The font used is \"%s\".\n", fontface);
		printf("\n");
		printf("namespace Saam {\n");
		printf("	struct GlyphKdNode {\n");
		printf("		char leftId, rightId;\n");
		printf("		unsigned char splitdim;\n");
		printf("		char glyph;\n");
		printf("		unsigned char shape[4];\n");
		printf("	};\n");
		printf("\n");
		printf("	const unsigned char glyph_coverage = 50;\n");
		printf("	const unsigned char density_baseline = %d;\n", (density_baseline + 5) / 10 * 10);
		printf("	const unsigned char cache_bitwidth = %d;\n", log2(density_baseline) + 1);
		printf("\n");
		printf("	const GlyphKdNode glyph_shapes[127 - 32] = {\n");

		printTreeNode(tree);

		printf("	};\n");
		printf("}");
	}
}

int main() {
	initgraph(400, 300, EW_SHOWCONSOLE);
	getRanks();

	GlyphList all;
	tree = new GlyphKdNode(all);
	output();
	delete tree;

	getchar();
	closegraph();
	return 0;
}