#include "GpxLoader.h"
#include "lib/xml/xml.h"

std::vector<LonLat> GpxLoader::load(const char* path) const {
	XMLNode* root = xml_parse_file(path);
	if (root == NULL) {
		throw "Unable to parse XML.";
	}

	const static std::string trkTag = "trk";
	const static std::string trksegTag = "trkseg";
	const static std::string trkptTag = "trkpt";

	std::vector<LonLat> points;

	// To keep data structures simple, we just retrieve all points in the GPX
	// and don't care about individual tracks or track segments.
	XMLNode* entries = xml_node_child_at(root, 0);
	for (size_t i = 0; i < entries->children->len; i++) {
		XMLNode* trk = xml_node_child_at(entries, i);
		if (trkTag != trk->tag) {
			continue;
		}
		for (size_t j = 0; j < trk->children->len; j++) {
			XMLNode* trkseg = xml_node_child_at(trk, j);
			if (trksegTag != trkseg->tag) {
				continue;
			}
			for (size_t k = 0; k < trkseg->children->len; k++) {
				XMLNode* trkpt = xml_node_child_at(trkseg, k);
				if (trkptTag != trkpt->tag) {
					continue; // actually unexpected
				}
				const char* lon = xml_node_attr(trkpt, "lon");
				const char* lat = xml_node_attr(trkpt, "lat");

				if (lon == NULL || lat == NULL) {
					// throw "Missing attribute on trkpt node.";
					continue;
				}

				LonLat lonLat = {strtod(lon, NULL), strtod(lat, NULL)};
				points.push_back(lonLat);
			}
		}
	}

	xml_node_free(root);

	return points;
}
