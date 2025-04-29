class TileKey {
	int zoomLevel;
	int x;
	int y;

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
