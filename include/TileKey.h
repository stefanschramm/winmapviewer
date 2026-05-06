#pragma once

// Disable long identifiers warning
#pragma warning(disable : 4786)

class TileKey {
  public:
	std::string styleUrlTemplate;
	int zoomLevel;
	int x;
	int y;

	TileKey(std::string styleUrlTemplate, int zoomLevel, int x, int y) : styleUrlTemplate(styleUrlTemplate), zoomLevel(zoomLevel), x(x), y(y) {};

	bool operator<(const TileKey& other) const {
		if (x != other.x) {
			return x < other.x;
		}
		if (y != other.y) {
			return y < other.y;
		}
		if (zoomLevel != other.zoomLevel) {
			return zoomLevel < other.zoomLevel;
		}
		return styleUrlTemplate < other.styleUrlTemplate;
	}

	bool operator==(const TileKey& other) const {
		return x == other.x && y == other.y && zoomLevel == other.zoomLevel && styleUrlTemplate == other.styleUrlTemplate;
	}
};
