#pragma once

#include <vector>

#include "Common.h"

class GpxLoader {
  public:
	std::vector<std::vector<LonLat> > load(const char* path) const;
};
