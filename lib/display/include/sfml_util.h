#ifndef sfml_util_h
#define sfml_util_h

#ifdef PIXELCLOCK_DESKTOP

/* Project Scope */
#include "canvas.h"

/* SFML */
#include <SFML/Graphics.hpp>

/* C++ Standard Library */
#include <cstdint>
#include <vector>

namespace canvas {

sf::Texture canvasToTex(const canvas::Canvas& c) {
    std::vector<uint8_t> pixels;
    pixels.reserve(c.getSize() * 4);
    for (int i = 0; i < c.getSize(); i++) {
        pixels.push_back(c[i].r);
        pixels.push_back(c[i].g);
        pixels.push_back(c[i].b);
        pixels.push_back(255);
    }
    sf::Texture tex;
    tex.create(c.getWidth(), c.getHeight());
    tex.update(pixels.data());
    return tex;
}

} // namespace canvas

#endif

#endif // sfml_util_h