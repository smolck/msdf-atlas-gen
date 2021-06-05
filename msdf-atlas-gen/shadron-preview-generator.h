
#pragma once

#include "FontGeometry.h"
#include "types.h"
#include <msdfgen-ext.h>
#include <msdfgen.h>

namespace msdf_atlas {

/// Generates a Shadron script that displays a string using the generated atlas
bool generateShadronPreview(const FontGeometry *fonts, int fontCount,
                            ImageType atlasType, int atlasWidth,
                            int atlasHeight, double pxRange,
                            const unicode_t *text, const char *imageFilename,
                            bool fullRange, const char *outputFilename);

} // namespace msdf_atlas
