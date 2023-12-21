#ifndef EMA_h
#define EMA_h

/* Arduino Core */
#include <Arduino.h>
#include <Wire.h>

/* C++ Standard Library */
#include <iterator>
#include <numeric>

namespace utility {

/* Exponential Moving Average */
class EMA {
public:
    EMA(float alpha) : alpha(alpha) {}

    float update(float val) {
        if (!started) {
            value = val;
        } else {
            value = (alpha * val) + (1 - alpha) * value;
        }
        return getValue();
    }

    float getValue() const { return value; }

private:
    float alpha{};
    float value{};
    bool started{false};
};

} // namespace utility

#endif // EMA_h