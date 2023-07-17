#ifndef DRAWPRIMITIVES_HPP
#define DRAWPRIMITIVES_HPP

#include "mercantile.h"
#include "clut.h"

inline float
interpolateF(const vData & data, const float x, const float y)
{
    const size_t ix = size_t(x);

    const size_t iy = size_t(y);

    const float up_left = data(ix, iy);
    const float up_right = data(ix + 1, iy);
    const float bot_left = data(ix, iy + 1);
    const float bot_right = data(ix + 1, iy + 1);

    const float tx = x - ix;
    const float ul_ur = up_left + (up_right - up_left) * tx;
    const float bl_br = bot_left + (bot_right - bot_left) * tx;

    const float ty = y - iy;
    const float u_b = ul_ur + (bl_br - ul_ur) * ty;

    return u_b;
}

// R - радиус, X1, Y1 - координаты центра
inline void
circle(vRGBA & picture, const RGBA color, const size_t x0, const size_t y0, const size_t R)
{
    int x = 0;
    int y = int(R);
    int delta = 1 - 2 * int(R);
    int error = 0;
    while(y >= 0) {
        picture(x0 + size_t(x), y0 + size_t(y)) = color;
        picture(x0 + size_t(x), y0 - size_t(y)) = color;
        picture(x0 - size_t(x), y0 + size_t(y)) = color;
        picture(x0 - size_t(x), y0 - size_t(y)) = color;
        error = 2 * (delta + y) - 1;
        if((delta < 0) && (error <= 0)) {
            delta += 2 * ++x + 1;
            continue;
        }
        if((delta > 0) && (error > 0)) {
            delta -= 2 * --y + 1;
            continue;
        }
        delta += 2 * (++x - y--);
    }
}

// http://www.opita.net/node/699 - перспективненька выглядит.
// https://stackoverflow.com/questions/2025451/line-drawing-algorithm
// https://github.com/mylxiaoyi/uniclop/blob/master/lib/boost/gil/extension/toolbox/wuline.hpp
///////////////////////////////////////////////////////////////////
inline void
wuline(vRGBA & picture, const RGBA & pixel, int X0, int Y0, int X1, int Y1)
{
    constexpr int NumLevels = 256;
    constexpr int IntensityBits = 8;
    constexpr unsigned short IntensityShift = 16 - IntensityBits;
    constexpr unsigned short WeightingComplementMask = NumLevels - 1;

    unsigned short ErrorAdj, ErrorAcc;
    unsigned short ErrorAccTemp, Weighting;
    short DeltaX, DeltaY, Temp, XDir;

    if(Y0 > Y1) {
        Temp = Y0;
        Y0 = Y1;
        Y1 = Temp;
        Temp = X0;
        X0 = X1;
        X1 = Temp;
    }

    picture(X0, Y0) = pixel;

    if((DeltaX = X1 - X0) >= 0) {
        XDir = 1;
    } else {
        XDir = -1;
        DeltaX = -DeltaX;
    }

    if((DeltaY = Y1 - Y0) == 0) {
        while(DeltaX-- != 0) {
            X0 += XDir;
            picture(X0, Y0) = pixel;
        }

        return;
    }

    if(DeltaX == 0) {
        do {
            Y0++;
            picture(X0, Y0) = pixel;
        } while(--DeltaY != 0);

        return;
    }

    if(DeltaX == DeltaY) {
        do {
            X0 += XDir;
            Y0++;
            picture(X0, Y0) = pixel;
        } while(--DeltaY != 0);

        return;
    }

    ErrorAcc = 0;
    if(DeltaY > DeltaX) {
        ErrorAdj = ((unsigned long)DeltaX << 16) / (unsigned long)DeltaY;

        while(--DeltaY) {
            ErrorAccTemp = ErrorAcc;
            ErrorAcc += ErrorAdj;

            if(ErrorAcc <= ErrorAccTemp)
                X0 += XDir;

            Y0++;

            Weighting = ErrorAcc >> IntensityShift;

            RGBA dst = pixel;
            dst.a = (Weighting ^ WeightingComplementMask);
            picture(X0, Y0).blend(dst);

            dst.a = Weighting;
            picture(X0 + XDir, Y0).blend(dst);
        }

        picture(X1, Y1) = pixel;
        return;
    }

    ErrorAdj = ((unsigned long)DeltaY << 16) / (unsigned long)DeltaX;

    while(--DeltaX) {
        ErrorAccTemp = ErrorAcc;
        ErrorAcc += ErrorAdj;

        if(ErrorAcc <= ErrorAccTemp)
            Y0++;

        X0 += XDir;

        Weighting = ErrorAcc >> IntensityShift;

        RGBA dst = pixel;
        dst.a = (Weighting ^ WeightingComplementMask);
        picture(X0, Y0).blend(dst);

        dst.a = Weighting;
        picture(X0, Y0 + 1).blend(dst);
    }

    picture(X1, Y1) = pixel;
}

// http://members.chello.at/~easyfilter/canvas.html - converted from JScript
#define WUTYPE int
#define BLENDT blend
inline void
lineWidth(
    vRGBA & picture, const RGBA color, WUTYPE x0, WUTYPE y0, WUTYPE x1, WUTYPE y1, float thf)
{ /* plot an anti-aliased line of width th pixel */
    WUTYPE sx = x0 < x1 ? 1 : -1;
    WUTYPE sy = y0 < y1 ? 1 : -1;

    WUTYPE err = 0;
    WUTYPE dx = std::abs(x1 - x0);
    WUTYPE dy = std::abs(y1 - y0);
    if(dx == 0 && dy == 0)
        dy = 1;
    WUTYPE e2 = WUTYPE(std::sqrt(dx * dx + dy * dy)); /* length */
    WUTYPE th = WUTYPE(255.0f * (thf - 1.0f)); /* scale values */

    dx = dx * 255 / e2;
    dy = dy * 255 / e2;

    RGBA c = color;

    if(dx < dy) { /* steep line */
        x1 = WUTYPE(
            (e2 + th / 2) / dy + 1); // "+1" - serg                          /* start offset */
        err = x1 * dy - th / 2; /* shift error value to offset width */
        for(x0 -= x1 * sx;; y0 += sy) {
            c.a = uint8_t(255 - err);
            x1 = x0;
            picture(x0, y0).BLENDT(c); /* aliasing pre-pixel */
            for(e2 = dy - err - th; e2 + dy < 255; e2 += dy) {
                x1 += sx;
                picture(x1, y0) = (color); /* pixel on the line */
            }
            c.a = uint8_t(255 - e2);
            picture(x1 + sx, y0).BLENDT(c); /* aliasing post-pixel */
            if(int(y0) == int(y1)) {
                break;
            }
            err += dx; /* y-step */
            if(err > 255) {
                err -= dy;
                x0 += sx;
            } /* x-step */
        }
    } else { /* flat line */
        y1 = WUTYPE((e2 + th / 2) / dx + 1); //"+1" - serg                   /* start offset */
        err = y1 * dx - th / 2; /* shift error value to offset width */
        for(y0 -= y1 * sy;; x0 += sx) {
            c.a = uint8_t(255 - err);
            picture(x0, y1 = y0).BLENDT(c); /* aliasing pre-pixel */
            for(e2 = dx - err - th; e2 + dx < 255; e2 += dx) {
                picture(x0, y1 += sy) = (color); /* pixel on the line */
            }
            c.a = uint8_t(255 - e2);
            picture(x0, y1 + sy).BLENDT(c); /* aliasing post-pixel */
            if(int(x0) == int(x1))
                break;
            err += dy; /* x-step */

            if(err > 255) {
                err -= dx;
                y0 += sy;
            } /* y-step */
        }
    }
}
#undef WUTYPE

#endif // DRAWPRIMITIVES_HPP
