#pragma once

#include "TileKey.h"
#include <iostream>

/**
 * Represents a range (rectangle) of tiles for a specific zoom level
 *
 * Left will be larger then right when international date line is visible.
 * Left and right being equal means complete span (from East to West) is visible.
 */
class TileRange {
  public:
	int zoomLevel;
	int left;
	int right;
	int top;
	int bottom;

	TileRange(int zoomLevel, int xMin, int xMax, int yMin, int yMax)
		: zoomLevel(zoomLevel), left(xMin), right(xMax), top(yMin), bottom(yMax) {

		maxExtend = 1 << zoomLevel;

		if (xMax < 0) {
			throw "Invalid rectangle (xMax was negative).";
		}

		if (yMax < 0 || yMax > maxExtend) {
			throw "Invalid rectangle (yMax) for this zoom level.";
		}

		// Normalize complete x range spans
		if (xMin == xMax || xMax - xMin >= maxExtend) {
			left = 0;
			right = 0;
		} else {
			left = left % maxExtend;
			right = right % maxExtend;
		}
	};

	bool contains(const TileKey& tile) const {
		if (tile.zoomLevel != zoomLevel) {
			return false;
		}
		if (tile.x < 0 || tile.y < 0 || tile.x >= maxExtend || tile.y >= maxExtend) {
			throw "Invalid TileKey encountered.";
		}
		if (tile.y < top || tile.y > bottom) {
			return false;
		}
		if (left <= right) {
			if (left == right) {
				// complete extend
				return true;
			}
			return tile.x >= left && tile.x < right;
		} else {
			return (tile.x >= left && tile.x < maxExtend) || (tile.x < right);
		}
	}

	int count() const {
		int height = bottom - top;

		int width;
		if (left == right) {
			width = maxExtend;
		} else {
			width = left <= right ? right - left : (maxExtend - left + right);
		}

		return width * height;
	}

  private:
	int maxExtend;
};
