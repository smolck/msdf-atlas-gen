
#pragma once

#include "FontGeometry.h"
#include "types.h"
#include <msdfgen-ext.h>
#include <msdfgen.h>
#include <sstream>

namespace msdf_atlas {

/// Writes the font and glyph metrics and atlas layout data into a comprehensive
/// JSON file
bool exportJSON(const FontGeometry *fonts, int fontCount, double fontSize,
                double pxRange, int atlasWidth, int atlasHeight,
                ImageType imageType, YDirection yDirection,
                const char *filename, bool kerning);
/// Writes the font and glyph metrics and atlas layout data as JSON into a
/// std::ostringstream.
bool exportJSON(const FontGeometry *fonts, int fontCount, double fontSize,
                double pxRange, int atlasWidth, int atlasHeight,
                ImageType imageType, YDirection yDirection,
                std::ostringstream &output, bool kerning);

} // namespace msdf_atlas
