/* Project Scope */
#include "display/effects/utilities.h"

/* C++ Standard Library */
#include <algorithm>

float calculateBarHeight(float val, float valMin, float valMax, float barMax) {
    float barMin = 0;
    float height = std::clamp((val - valMin) * (barMax - barMin) / (valMax - valMin) + barMin, barMin, barMax);
    return height;
}
