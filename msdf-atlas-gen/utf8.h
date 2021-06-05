
#pragma once

#include "types.h"
#include <vector>

namespace msdf_atlas {

/// Decodes the UTF-8 string into an array of Unicode codepoints
void utf8Decode(std::vector<unicode_t> &codepoints, const char *utf8String);

} // namespace msdf_atlas
