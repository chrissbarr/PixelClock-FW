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

class Button {
public:
    Button(std::string buttonText, sf::Vector2f buttonSize, int textSize, sf::Color bgColor, sf::Color textColor) {
        rectangle.setSize(buttonSize);
        rectangle.setFillColor(bgColor);

        text.setString(buttonText);
        text.setCharacterSize(textSize);
        text.setColor(textColor);

        width = buttonSize.x;
        height = buttonSize.y;
    }

    void setSize(sf::Vector2f buttonSize) {
        rectangle.setSize(buttonSize);
        width = buttonSize.x;
        height = buttonSize.y;
    }

    void setPosition(sf::Vector2f point) {
        rectangle.setPosition(point);
        float xPos = (point.x + width / 2) - (text.getLocalBounds().width / 2);
        float yPos = (point.y + height / 2) - (text.getLocalBounds().height / 2);
        text.setPosition(xPos, yPos);
    }

    void setFont(sf::Font& font) {
        text.setFont(font);
    }

    void setBackColor(sf::Color color) {
        rectangle.setFillColor(color);
    }

    void setTextColor(sf::Color color) {
        text.setColor(color);
    }

    void drawTo(sf::RenderWindow& window) {
        window.draw(rectangle);
        window.draw(text);
    }

    bool isMouseOver(sf::RenderWindow& window) {
        auto mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        int mouseX = mousePos.x;
        int mouseY = mousePos.y;

        int btnPosX = rectangle.getPosition().x;
        int btnPosY = rectangle.getPosition().y;

        int btnxPosWidth = btnPosX + width;
        int btnyPosHeight = btnPosY + height;

        if (mouseX < btnxPosWidth && mouseX > btnPosX && mouseY < btnyPosHeight && mouseY > btnPosY) { return true; }
        return false;
    }

    void registerCallback(std::function<void()> callback) {
        callbacks.push_back(callback);
    }

    void click() {
        for (auto& callback : callbacks) {
            callback();
        }
        currentlyClicked = true;
    }

    void release() {
        currentlyClicked = false;
    }

    bool isClicked() const { return currentlyClicked; }

    std::string getName() const { return text.getString(); }

private:
    sf::RectangleShape rectangle;
    sf::Text text;
    int width;
    int height;
    std::vector<std::function<void()>> callbacks;
    bool currentlyClicked{false};
};

#endif

#endif // sfml_util_h