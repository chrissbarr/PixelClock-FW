/* Project Scope */
#include "display/effects/gravity.h"

Gravity::Gravity(uint32_t moveInterval, bool empty, Gravity::Direction direction)
    : _moveInterval(moveInterval),
      _empty(empty),
      _direction(direction) {}

void Gravity::reset() {
    _lastMoveTime = millis();
    _finished = false;
}

canvas::Canvas Gravity::run() {
    uint32_t timenow = millis();
    if (timenow - _lastMoveTime > _moveInterval) {
        bool anyPixelsMovedThisUpdate = false;

        int xMax = _c.getWidth() - 1;
        int yMax = _c.getHeight() - 1;
        int xMin = 0;
        int yMin = 0;

        auto movePixel = [&](canvas::Canvas& c,
                             int x,
                             int y,
                             int xMove,
                             int yMove,
                             bool empty,
                             const canvas::Canvas& validPlaces = {}) {
            auto currentIndex = c.XYToIndex(x, y);
            flm::CRGB cellColour = c[currentIndex];

            if (cellColour != flm::CRGB(0)) {
                // if this is the last row
                if ((yMove == 1 && y == yMax) || (yMove == -1 && y == yMin) || (xMove == 1 && x == xMin) ||
                    (xMove == -1 && x == xMin)) {
                    if (empty) {
                        c[currentIndex] = 0;
                        return true;
                    }
                    return false;
                }

                if (validPlaces.getSize() != 0) {
                    // cast along from current position to end of canvas,
                    // checking if there are any empty 'valid' positions
                    int startX = x;
                    int startY = y;
                    int currentX = startX + xMove;
                    int currentY = startY + yMove;

                    bool furtherValidPixelsRemain = false;

                    while ((currentX >= xMin && currentX <= xMax) && (currentY >= yMin && currentY <= yMax)) {
                        if (validPlaces.getXY(currentX, currentY) != 0) {
                            if (c.getXY(currentX, currentY) == 0) {
                                furtherValidPixelsRemain = true;
                                break;
                            }
                        }
                        currentX += xMove;
                        currentY += yMove;
                    }

                    if (!furtherValidPixelsRemain) { return false; }
                }

                auto moveIntoIndex = c.XYToIndex(x + xMove, y + yMove);
                if (c[moveIntoIndex] == flm::CRGB(0)) {
                    c[moveIntoIndex] = cellColour;
                    c[currentIndex] = 0;
                    return true;
                }
            }
            return false;
        };

        switch (_direction) {
        case Gravity::Direction::down: {
            for (int y = yMax; y >= yMin; y--) {
                for (int x = xMin; x <= xMax; x++) {
                    if (movePixel(_c, x, y, 0, 1, _empty, validPixelsMask)) { anyPixelsMovedThisUpdate = true; }
                }
            }
            break;
        }
        case Gravity::Direction::up: {
            for (int y = yMin; y <= yMax; y++) {
                for (int x = xMin; x <= xMax; x++) {
                    if (movePixel(_c, x, y, 0, -1, _empty, validPixelsMask)) { anyPixelsMovedThisUpdate = true; }
                }
            }
            break;
        }
        case Gravity::Direction::left: {
            for (int y = yMin; y <= yMax; y++) {
                for (int x = xMin; x <= xMax; x++) {
                    if (movePixel(_c, x, y, -1, 0, _empty, validPixelsMask)) { anyPixelsMovedThisUpdate = true; }
                }
            }
            break;
        }
        case Gravity::Direction::right: {
            for (int y = yMin; y <= yMax; y++) {
                for (int x = xMax; x >= xMin; x--) {
                    if (movePixel(_c, x, y, 1, 0, _empty, validPixelsMask)) { anyPixelsMovedThisUpdate = true; }
                }
            }
            break;
        }
        }

        if (!anyPixelsMovedThisUpdate) { _finished = true; }
        _lastMoveTime = timenow;
    }
    return _c;
}