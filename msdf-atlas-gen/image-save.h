
#pragma once

#include "types.h"
#include <msdfgen.h>

namespace msdf_atlas {

/// Saves the bitmap as an image file with the specified format
template <typename T, int N>
bool saveImage(const msdfgen::BitmapConstRef<T, N> &bitmap, ImageFormat format,
               const char *filename,
               YDirection outputYDirection = YDirection::BOTTOM_UP);

} // namespace msdf_atlas

#include "image-save.hpp"
