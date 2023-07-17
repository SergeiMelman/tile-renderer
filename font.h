#ifndef FONT_H
#define FONT_H

#include <vector>
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

class Font
{
    FT_Library library{nullptr};
    FT_Face face{nullptr};
public:
    class Glyphs
    {
    public:
        FT_Vector pen{};
        FT_BBox box{};
        unsigned int rows;
        unsigned int width;
        FT_Int left;
        FT_Int top;
        FT_Fixed cs;
        FT_Fixed sn;
        std::vector<FT_BitmapGlyph> glyphs;
        ~Glyphs()
        {
            for(auto & glyph : glyphs)
                if(glyph)
                    FT_Done_Glyph(FT_Glyph(glyph));
        }
    };

public:
    Font() = default;
    Font(Font &&) = default;
    Font(const Font &) = delete;
    Font & operator=(const Font &) = delete;

public:
    ~Font();

public:
    static Font make(const std::string & fname, FT_F26Dot6 size);
    FT_BitmapGlyph makeGlyph(FT_ULong ch, FT_Matrix * matrix, FT_Vector * v);
    Glyphs makeString(const std::wstring & str, double angle = 0);
};

#endif // FONT_H
