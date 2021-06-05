
#include "json-export.h"

#include "GlyphGeometry.h"
#include <string>
#include <fmt/os.h>

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

bool exportJSON(const FontGeometry *fonts, int fontCount, double fontSize,
                double pxRange, int atlasWidth, int atlasHeight,
                ImageType imageType, YDirection yDirection,
                const char *filename, bool kerning) {
  auto file = fmt::output_file(filename);
  file.print("{{");

  // Atlas properties
  file.print("\"atlas\": {{");
  {
    file.print("\"type\":\"{}\",", imageTypeString(imageType));
    if (imageType == ImageType::SDF || imageType == ImageType::PSDF ||
        imageType == ImageType::MSDF || imageType == ImageType::MTSDF)
      file.print("\"distanceRange\":{:.17g},", pxRange);
    
    file.print("\"size\":{:.17g},", fontSize);
    file.print("\"width\":{:d},", atlasWidth);
    file.print("\"height\":{:d},", atlasHeight);
    file.print("\"yOrigin\":\"{}\"", yDirection == YDirection::TOP_DOWN ? "top" : "bottom");
  }
  file.print("}},");

  if (fontCount > 1)
    file.print("\"variants\':[");
  for (int i = 0; i < fontCount; ++i) {
    const FontGeometry &font = fonts[i];
    if (fontCount > 1) file.print(i == 0 ? "{{" : ",{{");

    // Font name
    const char *name = font.getName();
    if (name)
      file.print("\"name\":\"{}\",", escapeJsonString(name));

    // Font metrics
    file.print("\"metrics\":{{");
    {
      double yFactor = yDirection == YDirection::TOP_DOWN ? -1 : 1;
      const msdfgen::FontMetrics &metrics = font.getMetrics();
      file.print("\"emSize\":{:.17g},", metrics.emSize);
      file.print("\"lineHeight\":{:.17g},", metrics.lineHeight);
      file.print("\"ascender\":{:.17g},", yFactor * metrics.ascenderY);
      file.print("\"descender\":{:.17g},", yFactor * metrics.descenderY);
      file.print("\"underlineY\":{:.17g},", yFactor * metrics.underlineY);
      file.print("\"underlineThickness\":{:.17g}", metrics.underlineThickness);
    }
    file.print("}},");

    // Glyph mapping
    file.print("\"glyphs\":[");
    bool firstGlyph = true;
    for (const GlyphGeometry &glyph : font.getGlyphs()) {
      file.print(firstGlyph ? "{{" : ",{{");
      switch (font.getPreferredIdentifierType()) {
      case GlyphIdentifierType::GLYPH_INDEX:
        file.print("\"index\":{:d},", glyph.getIndex());
        break;
      case GlyphIdentifierType::UNICODE_CODEPOINT:
        file.print("\"unicode\":{},", glyph.getCodepoint());
        break;
      }
      file.print("\"advance\":{:.17g}", glyph.getAdvance());
      double l, b, r, t;
      glyph.getQuadPlaneBounds(l, b, r, t);
      if (l || b || r || t) {
        switch (yDirection) {
        case YDirection::BOTTOM_UP:
          file.print(",\"planeBounds\":{{\"left\":{:.17g},\"bottom\":{:.17g},\"right\":{:.17g},\"top\":{:.17g}}}", l, b, r, t);
          break;
        case YDirection::TOP_DOWN:
          file.print(",\"planeBounds\":{{\"left\":{:.17g},\"top\":{:.17g},\"right\":{:.17g},\"bottom\":{:.17g}}}", l, -t, r, -b);
          break;
        }
      }
      glyph.getQuadAtlasBounds(l, b, r, t);
      if (l || b || r || t) {
        switch (yDirection) {
        case YDirection::BOTTOM_UP:
          file.print(",\"atlasBounds\":{{\"left\":{:.17g},\"bottom\":{:.17g},\"right\":{:.17g},\"top\":{:.17g}}}", l, b, r, t);
          break;
        case YDirection::TOP_DOWN:
          file.print(",\"atlasBounds\":{{\"left\":{:.17g},\"bottom\":{:.17g},\"right\":{:.17g},\"top\":{:.17g}}}", l, atlasHeight - t, r, atlasHeight - b);
          break;
        }
      }
      file.print("}}");
      firstGlyph = false;
    }
    file.print("]");

    // Kerning pairs
    if (kerning) {
      file.print(",\"kerning\":[");
      bool firstPair = true;
      switch (font.getPreferredIdentifierType()) {
      case GlyphIdentifierType::GLYPH_INDEX:
        for (const std::pair<std::pair<int, int>, double> &kernPair :
             font.getKerning()) {
          file.print(firstPair ? "{{" : ",{{");
          file.print("\"index1\":{:d},", kernPair.first.first);
          file.print("\"index2\":{:d},", kernPair.first.second);
          file.print("\"advance\":{:.17g}", kernPair.second);
          file.print("}}");
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
            file.print(firstPair ? "{{" : ",{{");
            file.print("\"unicode1\":{},", glyph1->getCodepoint());
            file.print("\"unicode2\":{},", glyph2->getCodepoint());
            file.print("\"advance\":{:.17g}", kernPair.second);
            file.print("}}");
            firstPair = false;
          }
        }
        break;
      }
      file.print("]");
    }

    if (fontCount > 1)
      file.print("}}");
  }
  if (fontCount > 1)
    file.print("]");

  file.print("}}\n");
  file.close();
  return true;
}

} // namespace msdf_atlas
