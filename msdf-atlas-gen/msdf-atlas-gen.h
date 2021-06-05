
#pragma once

/*
 * MULTI-CHANNEL SIGNED DISTANCE FIELD ATLAS GENERATOR v1.2 (2021-05-29)
 * ---------------------------------------------------------------------
 * A utility by Viktor Chlumsky, (c) 2020 - 2021
 *
 * Generates compact bitmap font atlases using MSDFGEN.
 *
 */

#include <msdfgen-ext.h>
#include <msdfgen.h>

#include "AtlasGenerator.h"
#include "AtlasStorage.h"
#include "BitmapAtlasStorage.h"
#include "DynamicAtlas.h"
#include "FontGeometry.h"
#include "GlyphBox.h"
#include "GlyphGeometry.h"
#include "ImmediateAtlasGenerator.h"
#include "Rectangle.h"
#include "RectanglePacker.h"
#include "TightAtlasPacker.h"
#include "Workload.h"
#include "bitmap-blit.h"
#include "glyph-generators.h"
#include "image-encode.h"
#include "image-save.h"
#include "json-export.h"
#include "rectangle-packing.h"
#include "size-selectors.h"
#include "types.h"
#include "utf8.h"

#define MSDF_ATLAS_VERSION "1.2"
