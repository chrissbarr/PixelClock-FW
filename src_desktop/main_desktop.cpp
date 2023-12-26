
#ifdef PIXELCLOCK_DESKTOP

#include "display/effects/gravityfill.h"
#include "display/effects/utilities.h"

#include <canvas.h>

#include <SFML/Graphics.hpp>


#include <iostream>

sf::Texture canvasToTex(const canvas::Canvas& c) {
    std::vector<uint8_t> pixels;
    pixels.reserve(c.getSize() * 4);
    for (int i = 0; i < c.getSize(); i++) {
        pixels.push_back(c[i].r);
        pixels.push_back(c[i].g);
        pixels.push_back(c[i].b);
        // pixels.push_back(255);
        // pixels.push_back(255);
        // pixels.push_back(255);
        pixels.push_back(255);
    }
    sf::Texture tex;
    tex.create(c.getWidth(), c.getHeight());
    tex.update(pixels.data());
    return tex;
}

int main(int argc, char** argv) {
    std::cout << "Hello World!\n";

    canvas::Canvas c(30, 10);

    auto effect = GravityFill(c, 10, 10, colourGenerator::randomHSV);

    auto window = sf::RenderWindow{ { 800u, 400u }, "CMake SFML Project" };
    window.setFramerateLimit(60);

    while (window.isOpen())
    {
        for (auto event = sf::Event{}; window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        window.clear();

        c = effect.run();
        if (effect.finished()) {
            effect.reset();
        }
        auto t = canvasToTex(c);
        sf::Sprite sprite(t);

        sprite.setPosition(sf::Vector2f(0.f, 0.f));
        sprite.setScale(20.f, 20.f);

        window.draw(sprite);

        window.display();
    }

    return 0;
}

#endif