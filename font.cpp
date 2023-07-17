#include "font.h"
#include "tools.h"

#include <cmath>
#include <algorithm>
#include <stdexcept>

Font::~Font()
{
    if(face != nullptr)
        FT_Done_Face(face);
    if(library != nullptr)
        FT_Done_FreeType(library);
}

Font
Font::make(const std::string & fname, FT_F26Dot6 size)
{
    Font font;
    if(FT_Init_FreeType(&font.library)) /* initialize library */
        throw std::runtime_error("Font::make - FT_Init_FreeType error.");

    if(FT_New_Face(font.library, fname.c_str(), 0, &font.face)) /* create face object */
        throw std::invalid_argument("Font::make - font:" + fname + " load error.");

    if(FT_Set_Char_Size(font.face, size * 64l, 0, 0, 0)) /* set character size */
        throw std::runtime_error("Font::make - unexpected FT_Set_Char_Size error");

    return font;
}

FT_BitmapGlyph
Font::makeGlyph(FT_ULong ch, FT_Matrix * matrix, FT_Vector * v)
{
    /* set transformation */
    FT_Set_Transform(face, matrix, v);

    if(FT_Load_Char(face, ch, FT_LOAD_DEFAULT))
        throw std::runtime_error(STR("FT_Load_Char error."));

    FT_Glyph glyph;
    FT_Get_Glyph(face->glyph, &glyph); // make a copy

    // convert current glyph to a bitmap (default render mode + destroying old)
    if(glyph->format != FT_GLYPH_FORMAT_BITMAP)
        if(FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, 1))
            throw std::runtime_error(STR("FT_Glyph_To_Bitmap error."));

    return FT_BitmapGlyph(glyph);
}

Font::Glyphs
Font::makeString(const std::wstring & str, double angle)
{
    Glyphs glyphs{};
    /* set up matrix */
    angle *= M_PI * 2.0 / 360.0; // Degree -> Rad  
    glyphs.cs = FT_Fixed(cos(angle) * 0x10000L);
    glyphs.sn = FT_Fixed(sin(angle) * 0x10000L);
    FT_Matrix matrix = {glyphs.cs, -glyphs.sn, glyphs.sn, glyphs.cs};

    glyphs.glyphs.reserve(str.size());
    glyphs.box = {10000, 10000, -100000, -100000};

    for(const auto & s : str) {
        // store bitmap content by typecasting
        FT_BitmapGlyph bglyph = makeGlyph(FT_ULong(s), &matrix, &glyphs.pen); //(FT_BitmapGlyph)glyph;
        glyphs.glyphs.push_back(bglyph);

        const FT_Pos y2 = bglyph->top - FT_Int(bglyph->bitmap.rows);
        glyphs.box.yMin = std::min(glyphs.box.yMin, y2);
        glyphs.box.yMax = std::max(glyphs.box.yMin, FT_Pos(bglyph->top));

        /* increment pen position for the next glyph*/
        glyphs.pen.x += face->glyph->advance.x;
        glyphs.pen.y += face->glyph->advance.y;
    }

    glyphs.pen.x /= 64; // convert to pixel's size
    glyphs.pen.y /= 64;

    auto gf = glyphs.glyphs.front();
    auto gb = glyphs.glyphs.back();

    glyphs.box.xMin = gf->left;
    glyphs.box.xMax = gb->left + FT_Int(gb->bitmap.width);

    return glyphs;
}
