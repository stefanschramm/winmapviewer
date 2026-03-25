#include <iostream>

#include "Settings.h"
#include "resource.h"

const char* SUB_KEY = "Software\\stefanschramm.net\\winmapviewer";
const char* VALUE_NAME_STYLE_IDENTIFIER = "styleIdentifier";

Settings getDefaultSettings() {
	Settings settings;
	settings.lonLat = {52.0, 13.0};
	settings.styleIdentifier = IDM_STYLE_OSM_STANDARD;
	settings.useTls = false;
	settings.zoomLevel = 13;

	return settings;
}

HKEY openRegistryKey() {
	HKEY hKey;
	DWORD dwDisposition;
	LONG lResult = RegOpenKeyEx(
		HKEY_CURRENT_USER,
		SUB_KEY,
		0,
		KEY_ALL_ACCESS,
		&hKey
	);

	if (lResult != ERROR_SUCCESS) {
		return NULL;
	}

	return hKey;
}

HKEY createRegistryKey() {
	HKEY hKey;
	DWORD dwDisposition;
	LONG lResult = RegCreateKeyEx(
		HKEY_CURRENT_USER,
		SUB_KEY,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hKey,
		&dwDisposition
	);

	if (lResult != ERROR_SUCCESS) {
		throw "Unable to open registry key for writing settings.";
	}

	return hKey;
}

void closeRegistryKey(HKEY hKey) {
	LSTATUS closeResult = RegCloseKey(hKey);
	if (closeResult != ERROR_SUCCESS) {
		throw "Unable to close registry key for writing settings.";
	}
}

Settings loadSettingsFromRegistry() {
	Settings settings = getDefaultSettings();

	HKEY hKey = openRegistryKey();
	if (hKey == NULL) {
		// Probably initial start
		return settings;
	}

	DWORD pwdType;
	DWORD dwSize = sizeof(settings.styleIdentifier);
	LRESULT lResult = RegGetValue(
		hKey,
		"",
		VALUE_NAME_STYLE_IDENTIFIER,
		RRF_RT_REG_DWORD,
		&pwdType,
		reinterpret_cast<BYTE*>(&(settings.styleIdentifier)),
		&dwSize
	);

	if (lResult != ERROR_SUCCESS) {
		throw "Unable to read style identifier from registry.";
	}

	// TODO: load other values into struct

	closeRegistryKey(hKey);

	return settings;
}

void storeSettingsInRegistry(Settings settings) {
	HKEY hKey = createRegistryKey();

	LRESULT lResult = RegSetValueEx(
		hKey,
		VALUE_NAME_STYLE_IDENTIFIER,
		0,
		REG_DWORD,
		reinterpret_cast<BYTE*>(&(settings.styleIdentifier)),
		sizeof(settings.styleIdentifier)
	);

	if (lResult != ERROR_SUCCESS) {
		throw "Unable to store style identifier in registry.";
	}

	// TODO: store other values from struct
	// TODO: store complete struct instead of individual values?

	closeRegistryKey(hKey);
}
