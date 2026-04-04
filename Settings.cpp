#include <iostream>

#include "Settings.h"
#include "resource.h"

const char* SUB_KEY = "Software\\stefanschramm.net\\winmapviewer";
const char* VALUE_NAME_CENTER_X = "centerX";
const char* VALUE_NAME_CENTER_Y = "centerY";
const char* VALUE_NAME_STYLE_IDENTIFIER = "styleIdentifier";
const char* VALUE_NAME_USE_TLS = "useTls";
const char* VALUE_NAME_ZOOMLEVEL = "zoomLevel";
const char* VALUE_NAME_CUSTOM_STYLE_URL_TEMPLATE = "customStyleUrlTemplate";

Settings getDefaultSettings() {
	Settings settings;
	settings.styleIdentifier = IDM_STYLE_OSM_STANDARD;
	settings.useTls = false;
	// Put some eurocentrism in here
	settings.zoomLevel = 4;
	settings.centerX = 2211;
	settings.centerY = 1353;
	settings.customStyleUrlTemplate = "";

	return settings;
}

HKEY openRegistryKey() {
	HKEY hKey;
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
	LONG closeResult = RegCloseKey(hKey);
	if (closeResult != ERROR_SUCCESS) {
		throw "Unable to close registry key.";
	}
}

bool loadInt(HKEY hKey, const char* valueName, int* value) {
	DWORD dwType;
	DWORD dwSize = sizeof(*value);
	// Can not use RegGetValue here for VC++6 compatibility
	LONG lResult = RegQueryValueEx(
		hKey,
		valueName,
		NULL,
		&dwType,
		reinterpret_cast<BYTE*>(value),
		&dwSize
	);

	if (lResult != ERROR_SUCCESS || dwType != REG_DWORD) {
		return false;
	}

	return true;
}

bool loadString(HKEY hKey, const char* valueName, std::string* value) {
	DWORD dwType;
	DWORD dwSize = 0;
	LONG lResult;

	lResult = RegQueryValueEx(
		hKey,
		valueName,
		NULL,
		&dwType,
		NULL,
		&dwSize
	);

	if (lResult != ERROR_SUCCESS || dwType != REG_SZ || dwSize < 1) {
		return false;
	}

	value->resize(dwSize);

	lResult = RegQueryValueEx(
		hKey,
		valueName,
		NULL,
		&dwType,
		reinterpret_cast<BYTE*>(&(*value)[0]),
		&dwSize
	);

	if (lResult != ERROR_SUCCESS || dwType != REG_SZ) {
		return false;
	}

	// TODO: prevent strlen overflow / reimplement strnlen for VC++6?
	size_t actualLength = strlen(value->c_str());
	value->resize(actualLength);

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
	loadString(hKey, VALUE_NAME_CUSTOM_STYLE_URL_TEMPLATE, &(settings.customStyleUrlTemplate));

	int iUseTls = settings.useTls ? 1 : 0;
	loadInt(hKey, VALUE_NAME_USE_TLS, &iUseTls);
	settings.useTls = iUseTls != 0;

	closeRegistryKey(hKey);

	return settings;
}

void storeInt(HKEY hKey, const char* valueName, int value) {
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

void storeString(HKEY hKey, const char* valueName, std::string& value) {
	LRESULT lResult = RegSetValueEx(
		hKey,
		valueName,
		0,
		REG_SZ,
		reinterpret_cast<const BYTE*>(value.c_str()),
		value.size() + 1
	);

	if (lResult != ERROR_SUCCESS) {
		throw "Unable to store string value in registry.";
	}
}

void storeSettingsInRegistry(Settings settings) {
	HKEY hKey = createRegistryKey();

	storeInt(hKey, VALUE_NAME_STYLE_IDENTIFIER, settings.styleIdentifier);
	storeInt(hKey, VALUE_NAME_CENTER_X, settings.centerX);
	storeInt(hKey, VALUE_NAME_CENTER_Y, settings.centerY);
	storeInt(hKey, VALUE_NAME_ZOOMLEVEL, settings.zoomLevel);
	storeInt(hKey, VALUE_NAME_USE_TLS, settings.useTls ? 1 : 0);
	storeString(hKey, VALUE_NAME_CUSTOM_STYLE_URL_TEMPLATE, settings.customStyleUrlTemplate);

	closeRegistryKey(hKey);
}
