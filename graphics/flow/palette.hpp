//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef PALETTE_HPP
#define PALETTE_HPP

#include "cellular.hpp"
#include "image.h"
#include "photo.h"
#include <cstdlib>
#include <vector>
#include <sstream>
#include <iostream>


typedef unsigned char color_type;


struct Colorizer
{
    std::string filename_prefix;
    std::vector<Color> palette;

    virtual ~Colorizer() {}
    virtual void initialize(Grid<color_type> * grid) = 0;
    Colorizer(std::string fp) : filename_prefix(fp) {}
    void image(Grid<color_type> * colors, size_t i)
    {
        std::ostringstream oss;
        oss << filename_prefix << i << ".jpeg";
        ::image out = image_create(oss.str().c_str(), colors->w, colors->h);
        for (PositionIterator it = colors->positions(); it.more(); ++it) {
            color_type & c = colors->cell(it);
            assert(c <= palette.size());
            image_write(out, palette[c]);
        }
        image_close(out);
        std::cout << i << "\r" << std::flush;
    }
};


struct Painting
{
    ~Painting() { photo_delete(ph); }
    Painting(std::string path, size_t n) : n(n)
    {
        assert(n <= 256);
        std::ostringstream q_pnm;
        q_pnm << "q" << n << ".pnm";
        std::string qpath(path);
        size_t i = qpath.find(".png");
        assert(i != std::string::npos);
        qpath.replace(i + 1, 3, q_pnm.str());
        std::ostringstream cmd;
        // note: pngquant may have --nofs
        cmd << "pngquant " << n << " < " << path << " | pngtopnm - > " << qpath;
        std::cerr << cmd.str() << "\n";
        ph = std::system(cmd.str().c_str()) ? NULL : photo_create(qpath.c_str());
        if ( ! ph) std::cerr << "error: got no photo\n";
        std::remove(qpath.c_str());  // unlink, but we still have file open
    }

    std::vector<Color> print(Grid<color_type> * board)
    {
        std::vector<Color> palette(n, {0, 0, 0});
        if ( ! ph) return palette;
        double h_m = ph->height /(double) board->h;
        double w_m = ph->width /(double) board->w;
        std::map<unsigned, color_type> colors;
        for (PositionIterator it = board->positions(); it.more(); ++it) {
            const Position & pos = it.position;
            size_t x = pos.j * w_m;
            size_t y = pos.i * h_m;
            unsigned rgb = photo_rgb(ph, x, y);
            if (colors.find(rgb) == colors.end()) {
                color_type c = colors.size();
                assert(c < n);
                colors[rgb] = c;
            }
            board->cell(it) = colors[rgb];
        }
        for (std::map<unsigned, color_type>::iterator me = colors.begin();
                me != colors.end(); ++me) {
            unsigned rgb = me->first;
            palette[me->second] = (Color){ (rgb & 0xff) / 255.0,
                    ((rgb >> 8) & 0xff) / 255.0, (rgb >> 16) / 255.0 };
        }
        return palette;
    }

private:
    Photo * ph;
    const size_t n;
};


struct PhotoColorizer : Colorizer
{
    const std::string path;
    const size_t n;

    PhotoColorizer(std::string path, std::string filename_prefix,
            size_t n = 256)
        : Colorizer(filename_prefix)
        , path(path)
        , n(n)
    {}
    void initialize(Grid<color_type> * grid)
    {
        palette = Painting(path, n).print(grid);
    }
};


#endif

