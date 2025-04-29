#pragma once

class TileKey {
  public:
	int zoomLevel;
	int x;
	int y;

	TileKey(int zoomLevel, int x, int y) : zoomLevel(zoomLevel), x(x), y(y){};

	bool operator<(const TileKey& other) const {
		if (x != other.x) {
			return x < other.x;
		}
		if (y != other.y) {
			return y < other.y;
		}
		return zoomLevel < other.zoomLevel;
	}
};
