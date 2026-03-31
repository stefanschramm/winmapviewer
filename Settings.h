#pragma once

#include "Common.h"

struct Settings {
	int styleIdentifier;
	int zoomLevel;
	int centerX;
	int centerY;
	bool useTls;
	std::string customStyleUrlTemplate;
};

Settings loadSettingsFromRegistry();
void storeSettingsInRegistry(Settings settings);
