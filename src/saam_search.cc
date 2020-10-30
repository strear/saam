#include "saam.hpp"
#include "saam_data.hpp"
#include <algorithm>

using namespace Saam;

namespace {
	void search(const char(&parts)[4],
		const char nodeId, unsigned& minDistance, char& result) {

		const GlyphKdNode& node = glyph_shapes[(size_t)nodeId];
		const char way = parts[node.splitdim] - node.shape[node.splitdim];
		unsigned newDistance;

		if (way > 0) {
			if (node.rightId != -1) {
				search(parts, node.rightId, minDistance, result);
				if (minDistance == 0) return;
			}

			newDistance =
				(abs(parts[0] - node.shape[0]) + abs(parts[3] - node.shape[3]))
				+ (abs(parts[2] - node.shape[2]) + abs(parts[1] - node.shape[1]));

			if (newDistance < minDistance) {
				minDistance = newDistance;
				result = node.glyph;
				if (minDistance == 0) return;
			}

			if (minDistance > unsigned(way) && node.leftId != -1) {
				search(parts, node.leftId, minDistance, result);
			}
		} else {
			if (node.leftId != -1) {
				search(parts, node.leftId, minDistance, result);
				if (minDistance == 0) return;
			}

			newDistance =
				(abs(parts[0] - node.shape[0]) + abs(parts[3] - node.shape[3]))
				+ (abs(parts[2] - node.shape[2]) + abs(parts[1] - node.shape[1]));

			if (newDistance < minDistance) {
				minDistance = newDistance;
				result = node.glyph;
				if (minDistance == 0) return;
			}

			if (minDistance > unsigned(-way) && node.rightId != -1) {
				search(parts, node.rightId, minDistance, result);
			}
		}
	}

	char cache[1 << (4 * cache_bitwidth)] = { ' ' };
}

char Saam::getCharByShape(const char(&parts)[4]) {
	char& result = cache[parts[0]
		| (parts[1] << cache_bitwidth)
		| (parts[2] << 2 * cache_bitwidth)
		| (parts[3] << 3 * cache_bitwidth)];

	if (result) return result;

	unsigned minDistance = 4 * glyph_coverage;
	search(parts, 0, minDistance, result);

	return result;
}