#pragma once

#include "Common.h"

struct Settings {
	int styleIdentifier;
	int zoomLevel;
	int centerX;
	int centerY;
	bool useTls;
};

Settings loadSettingsFromRegistry();
void storeSettingsInRegistry(Settings settings);
