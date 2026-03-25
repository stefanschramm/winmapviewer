#pragma once

#include "Common.h"

struct Settings {
	int styleIdentifier;
	int zoomLevel;
	bool useTls;
	LonLat lonLat;
};

Settings loadSettingsFromRegistry();
void storeSettingsInRegistry(Settings settings);
