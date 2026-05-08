#include "StyleDatabase.h"

const Style* StyleDatabase::get(int styleIdentifier) const {
	// Reverse proxy server urls are used to be able to centrally disable/change tile usage if required.
	static const Style styles[5] = {
		// 400 IDM_STYLE_OSM_STANDARD
		// https://tile.openstreetmap.org/{z}/{x}/{y}.png
		{
			"https://osm.kesto.de/tile/osm/{z}/{x}/{y}.png",
			"http://osm.kesto.de/tile/osm/{z}/{x}/{y}.png",
			"Map (C) OpenStreetMap",
			"http://openstreetmap.org/copyright",
			19
		},
		// 401 IDM_STYLE_OSM_GERMAN
		// https://tile.openstreetmap.de/{z}/{x}/{y}.png
		{
			"https://osm.kesto.de/tile/german/{z}/{x}/{y}.png",
			"http://osm.kesto.de/tile/german/{z}/{x}/{y}.png",
			"Map (C) OpenStreetMap",
			"http://openstreetmap.org/copyright",
			19
		},
		// 402 IDM_STYLE_OEPNV
		// https://tile.memomaps.de/tilegen/{z}/{x}/{y}.png
		{
			"https://osm.kesto.de/tile/oepnv/{z}/{x}/{y}.png",
			"http://osm.kesto.de/tile/oepnv/{z}/{x}/{y}.png",
			"Map (C) OpenStreetMap / Tiles: OEPNVkarte.de",
			"https://xn--pnvkarte-m4a.de",
			18
		},
		// 403 IDM_STYLE_OPENTOPO
		// https://a.tile.opentopomap.org/{z}/{x}/{y}.png
		{
			"https://osm.kesto.de/tile/opentopo/{z}/{x}/{y}.png",
			"http://osm.kesto.de/tile/opentopo/{z}/{x}/{y}.png",
			"Map (C) OpenStreetMap / Tiles: opentopomap.org",
			"https://opentopomap.org/",
			17
		},
		// 404 IDM_STYLE_SWISS
		// https://a.tile.opentopomap.org/{z}/{x}/{y}.png
		{
			"https://tile.osm.ch/switzerland/{z}/{x}/{y}.png",
			"http://tile.osm.ch/switzerland/{z}/{x}/{y}.png",
			"Map (C) OpenStreetMap contributors, Elevation: ASTER GDEM, EarthEnv-DEM90, CDEM contains information under OGL Canada",
			"https://sosm.ch/projects/tile-service/",
			20
		}
	};

	const int styleIndex = styleIdentifier - m_identifierOffset;

	if (styleIndex < 0 || styleIndex > sizeof(styles) / sizeof(Style) - 1) {
		throw "Invalid style.";
	}

	return &styles[styleIndex];
}
