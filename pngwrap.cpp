#include "pngwrap.h"
#include "miniz/miniz.h"

PNGWrap::PNGWrap(const vRGBA & buffer)
{
    const int w = int(buffer.getWidth());
    const int h = int(buffer.getHeight());
    png_buf = puint8_t(tdefl_write_image_to_png_file_in_memory_ex(
        buffer.data(), w, h, 4 /*bytesPerPixel*/, &len, 6 /*compress level*/, false /*flip*/));
}

PNGWrap::~PNGWrap()
{
    if(png_buf)
        mz_free(png_buf);
}

size_t
PNGWrap::getBufLength() const
{
    return len;
}

const uint8_t *
PNGWrap::getBufBytes() const
{
    return png_buf;
}
