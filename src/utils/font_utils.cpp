#include "font_utils.h"

namespace procdraw {

    void TextureFontMetrics::ClearGlyphs(int maxCharCode)
    {
        glyphMetrics_.clear();
        glyphMetrics_.resize(maxCharCode + 1);
    }

    int TextureFontMetrics::MaxCharCode()
    {
        return glyphMetrics_.size() - 1;
    }

    void TextureFontMetrics::SetGlyph(int charCode, const TextureGlyphMetrics &glyphMetrics)
    {
        glyphMetrics_.at(charCode) = glyphMetrics;
    }

    TextureGlyphMetrics& TextureFontMetrics::GetGlyph(int charCode)
    {
        return glyphMetrics_.at(charCode);
    }

    GlyphCoords LayOutGlyph(const procdraw::TextureGlyphMetrics &glyphMetrics,
                            const procdraw::TextureFontMetrics &fontMetrics)
    {
        GlyphCoords coords;
        coords.Left = glyphMetrics.LeftBearingPixels;
        coords.Right = coords.Left + glyphMetrics.WidthPixels;
        coords.Top = fontMetrics.AscenderPixels - glyphMetrics.TopBearingPixels;
        coords.Bottom = coords.Top + glyphMetrics.HeightPixels;
        coords.TextureLeft = ((float)glyphMetrics.XoffsetPixels) / fontMetrics.TextureWidth;
        coords.TextureRight = ((float)glyphMetrics.XoffsetPixels + glyphMetrics.WidthPixels) / fontMetrics.TextureWidth;
        coords.TextureTop = 0.0f;
        coords.TextureBottom = ((float)glyphMetrics.HeightPixels) / fontMetrics.TextureHeight;
        return coords;
    }

}
