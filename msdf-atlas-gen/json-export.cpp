
#include "json-export.h"

#include "GlyphGeometry.h"
#include <fmt/os.h>
#include <fmt/ostream.h>
#include <fstream>
#include <iostream>
#include <string>

namespace msdf_atlas {

static std::string escapeJsonString(const char *str) {
  char uval[7] = "\\u0000";
  std::string outStr;
  while (*str) {
    switch (*str) {
    case '\\':
      outStr += "\\\\";
      break;
    case '"':
      outStr += "\\\"";
      break;
    case '\n':
      outStr += "\\n";
      break;
    case '\r':
      outStr += "\\r";
      break;
    case '\t':
      outStr += "\\t";
      break;
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08: /* \\t */ /* \\n */
    case 0x0b:
    case 0x0c: /* \\r */
    case 0x0e:
    case 0x0f:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x19:
    case 0x1a:
    case 0x1b:
    case 0x1c:
    case 0x1d:
    case 0x1e:
    case 0x1f:
      uval[4] = '0' + (*str >= 0x10);
      uval[5] = "0123456789abcdef"[*str & 0x0f];
      outStr += uval;
      break;
    default:
      outStr.push_back(*str);
    }
    ++str;
  }
  return outStr;
}

static const char *imageTypeString(ImageType type) {
  switch (type) {
  case ImageType::HARD_MASK:
    return "hardmask";
  case ImageType::SOFT_MASK:
    return "softmask";
  case ImageType::SDF:
    return "sdf";
  case ImageType::PSDF:
    return "psdf";
  case ImageType::MSDF:
    return "msdf";
  case ImageType::MTSDF:
    return "mtsdf";
  }
  return nullptr;
}

void exportJSONTo(const FontGeometry *fonts, int fontCount, double fontSize,
                  double pxRange, int atlasWidth, int atlasHeight,
                  ImageType imageType, YDirection yDirection,
                  std::ostream &output, bool kerning) {
  fmt::print(output, "{{");

  // Atlas properties
  fmt::print(output, "\"atlas\": {{");
  {
    fmt::print(output, "\"type\":\"{}\",", imageTypeString(imageType));
    if (imageType == ImageType::SDF || imageType == ImageType::PSDF ||
        imageType == ImageType::MSDF || imageType == ImageType::MTSDF)
      fmt::print(output, "\"distanceRange\":{:.17g},", pxRange);

    fmt::print(output, "\"size\":{:.17g},", fontSize);
    fmt::print(output, "\"width\":{:d},", atlasWidth);
    fmt::print(output, "\"height\":{:d},", atlasHeight);
    fmt::print(output, "\"yOrigin\":\"{}\"",
               yDirection == YDirection::TOP_DOWN ? "top" : "bottom");
  }
  fmt::print(output, "}},");

  if (fontCount > 1)
    fmt::print(output, "\"variants\':[");
  for (int i = 0; i < fontCount; ++i) {
    const FontGeometry &font = fonts[i];
    if (fontCount > 1)
      fmt::print(output, i == 0 ? "{{" : ",{{");

    // Font name
    const char *name = font.getName();
    if (name)
      fmt::print(output, "\"name\":\"{}\",", escapeJsonString(name));

    // Font metrics
    fmt::print(output, "\"metrics\":{{");
    {
      double yFactor = yDirection == YDirection::TOP_DOWN ? -1 : 1;
      const msdfgen::FontMetrics &metrics = font.getMetrics();
      fmt::print(output, "\"emSize\":{:.17g},", metrics.emSize);
      fmt::print(output, "\"lineHeight\":{:.17g},", metrics.lineHeight);
      fmt::print(output, "\"ascender\":{:.17g},", yFactor * metrics.ascenderY);
      fmt::print(output, "\"descender\":{:.17g},",
                 yFactor * metrics.descenderY);
      fmt::print(output, "\"underlineY\":{:.17g},",
                 yFactor * metrics.underlineY);
      fmt::print(output, "\"underlineThickness\":{:.17g}",
                 metrics.underlineThickness);
    }
    fmt::print(output, "}},");

    // Glyph mapping
    fmt::print(output, "\"glyphs\":[");
    bool firstGlyph = true;
    for (const GlyphGeometry &glyph : font.getGlyphs()) {
      fmt::print(output, firstGlyph ? "{{" : ",{{");
      switch (font.getPreferredIdentifierType()) {
      case GlyphIdentifierType::GLYPH_INDEX:
        fmt::print(output, "\"index\":{:d},", glyph.getIndex());
        break;
      case GlyphIdentifierType::UNICODE_CODEPOINT:
        fmt::print(output, "\"unicode\":{},", glyph.getCodepoint());
        break;
      }
      fmt::print(output, "\"advance\":{:.17g}", glyph.getAdvance());
      double l, b, r, t;
      glyph.getQuadPlaneBounds(l, b, r, t);
      if (l || b || r || t) {
        switch (yDirection) {
        case YDirection::BOTTOM_UP:
          fmt::print(output,
                     ",\"planeBounds\":{{\"left\":{:.17g},\"bottom\":{:.17g},"
                     "\"right\":{:.17g},\"top\":{:.17g}}}",
                     l, b, r, t);
          break;
        case YDirection::TOP_DOWN:
          fmt::print(output,
                     ",\"planeBounds\":{{\"left\":{:.17g},\"top\":{:.17g},"
                     "\"right\":{:.17g},\"bottom\":{:.17g}}}",
                     l, -t, r, -b);
          break;
        }
      }
      glyph.getQuadAtlasBounds(l, b, r, t);
      if (l || b || r || t) {
        switch (yDirection) {
        case YDirection::BOTTOM_UP:
          fmt::print(output,
                     ",\"atlasBounds\":{{\"left\":{:.17g},\"bottom\":{:.17g},"
                     "\"right\":{:.17g},\"top\":{:.17g}}}",
                     l, b, r, t);
          break;
        case YDirection::TOP_DOWN:
          fmt::print(output,
                     ",\"atlasBounds\":{{\"left\":{:.17g},\"bottom\":{:.17g},"
                     "\"right\":{:.17g},\"top\":{:.17g}}}",
                     l, atlasHeight - t, r, atlasHeight - b);
          break;
        }
      }
      fmt::print(output, "}}");
      firstGlyph = false;
    }
    fmt::print(output, "]");

    // Kerning pairs
    if (kerning) {
      fmt::print(output, ",\"kerning\":[");
      bool firstPair = true;
      switch (font.getPreferredIdentifierType()) {
      case GlyphIdentifierType::GLYPH_INDEX:
        for (const std::pair<std::pair<int, int>, double> &kernPair :
             font.getKerning()) {
          fmt::print(output, firstPair ? "{{" : ",{{");
          fmt::print(output, "\"index1\":{:d},", kernPair.first.first);
          fmt::print(output, "\"index2\":{:d},", kernPair.first.second);
          fmt::print(output, "\"advance\":{:.17g}", kernPair.second);
          fmt::print(output, "}}");
          firstPair = false;
        }
        break;
      case GlyphIdentifierType::UNICODE_CODEPOINT:
        for (const std::pair<std::pair<int, int>, double> &kernPair :
             font.getKerning()) {
          const GlyphGeometry *glyph1 =
              font.getGlyph(msdfgen::GlyphIndex(kernPair.first.first));
          const GlyphGeometry *glyph2 =
              font.getGlyph(msdfgen::GlyphIndex(kernPair.first.second));
          if (glyph1 && glyph2 && glyph1->getCodepoint() &&
              glyph2->getCodepoint()) {
            fmt::print(output, firstPair ? "{{" : ",{{");
            fmt::print(output, "\"unicode1\":{},", glyph1->getCodepoint());
            fmt::print(output, "\"unicode2\":{},", glyph2->getCodepoint());
            fmt::print(output, "\"advance\":{:.17g}", kernPair.second);
            fmt::print(output, "}}");
            firstPair = false;
          }
        }
        break;
      }
      fmt::print(output, "]");
    }

    if (fontCount > 1)
      fmt::print(output, "}}");
  }
  if (fontCount > 1)
    fmt::print(output, "]");

  fmt::print(output, "}}\n");
}

bool exportJSON(const FontGeometry *fonts, int fontCount, double fontSize,
                double pxRange, int atlasWidth, int atlasHeight,
                ImageType imageType, YDirection yDirection,
                const char *filename, bool kerning) {

  auto file = std::ofstream(filename);
  exportJSONTo(fonts, fontCount, fontSize, pxRange, atlasWidth, atlasHeight,
               imageType, yDirection, file, kerning);
  file.close();

  return true;
}

bool exportJSON(const FontGeometry *fonts, int fontCount, double fontSize,
                double pxRange, int atlasWidth, int atlasHeight,
                ImageType imageType, YDirection yDirection,
                std::ostringstream &output, bool kerning) {

  exportJSONTo(fonts, fontCount, fontSize, pxRange, atlasWidth, atlasHeight,
               imageType, yDirection, output, kerning);

  return true;
}

} // namespace msdf_atlas
