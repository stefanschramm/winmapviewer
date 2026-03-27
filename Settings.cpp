#include <iostream>

#include "Settings.h"
#include "resource.h"

const char* SUB_KEY = "Software\\stefanschramm.net\\winmapviewer";
const char* VALUE_NAME_CENTER_X = "centerX";
const char* VALUE_NAME_CENTER_Y = "centerY";
const char* VALUE_NAME_STYLE_IDENTIFIER = "styleIdentifier";
const char* VALUE_NAME_USE_TLS = "useTls";
const char* VALUE_NAME_ZOOMLEVEL = "zoomLevel";

Settings getDefaultSettings() {
	Settings settings;
	settings.styleIdentifier = IDM_STYLE_OSM_STANDARD;
	settings.useTls = false;
	// Put some eurocentrism in here
	settings.zoomLevel = 4;
	settings.centerX = 2211;
	settings.centerY = 1353;

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

bool loadInt(HKEY& hKey, const char* valueName, int* value) {
	DWORD pwdType;
	DWORD dwSize = sizeof(*value);
	LRESULT lResult = RegGetValue(
		hKey,
		"",
		valueName,
		RRF_RT_REG_DWORD,
		&pwdType,
		reinterpret_cast<BYTE*>(value),
		&dwSize
	);

	if (lResult != ERROR_SUCCESS) {
		return false;
	}

	return true;
}

Settings loadSettingsFromRegistry() {
	Settings settings = getDefaultSettings();

	HKEY hKey = openRegistryKey();
	if (hKey == NULL) {
		// Probably initial start
		return settings;
	}

	// Errors on loading are ignored - causes usage of defaults

	loadInt(hKey, VALUE_NAME_STYLE_IDENTIFIER, &(settings.styleIdentifier));
	loadInt(hKey, VALUE_NAME_CENTER_X, &(settings.centerX));
	loadInt(hKey, VALUE_NAME_CENTER_Y, &(settings.centerY));
	loadInt(hKey, VALUE_NAME_ZOOMLEVEL, &(settings.zoomLevel));

	int iUseTls = settings.useTls ? 1 : 0;
	loadInt(hKey, VALUE_NAME_USE_TLS, &iUseTls);
	settings.useTls = iUseTls != 0;

	closeRegistryKey(hKey);

	return settings;
}

void storeInt(HKEY& hKey, const char* valueName, int value) {
	LRESULT lResult = RegSetValueEx(
		hKey,
		valueName,
		0,
		REG_DWORD,
		reinterpret_cast<BYTE*>(&(value)),
		sizeof(value)
	);

	if (lResult != ERROR_SUCCESS) {
		throw "Unable to store integer value in registry.";
	}
}

void storeSettingsInRegistry(Settings settings) {
	HKEY hKey = createRegistryKey();

	storeInt(hKey, VALUE_NAME_STYLE_IDENTIFIER, settings.styleIdentifier);
	storeInt(hKey, VALUE_NAME_CENTER_X, settings.centerX);
	storeInt(hKey, VALUE_NAME_CENTER_Y, settings.centerY);
	storeInt(hKey, VALUE_NAME_ZOOMLEVEL, settings.zoomLevel);
	storeInt(hKey, VALUE_NAME_USE_TLS, settings.useTls ? 1 : 0);

	closeRegistryKey(hKey);
}
