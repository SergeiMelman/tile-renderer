#ifndef PNGWRAP_H
#define PNGWRAP_H

#include "types.h"

class PNGWrap
{
    using puint8_t = uint8_t *;
    
    size_t len = 0;
    uint8_t * png_buf;

public:
    explicit PNGWrap(const vRGBA & buffer);
    ~PNGWrap();

    size_t getBufLength() const;
    const uint8_t * getBufBytes() const;
};

#endif // PNGWRAP_H
