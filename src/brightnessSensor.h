#ifndef brightnessSensor_h
#define brightnessSensor_h

/* C++ Standard Library */
#include <cstdint>
#include <memory>

class TSL2591I2C;

class BrightnessSensor {
public:
    BrightnessSensor();
    ~BrightnessSensor();

    float getBrightness() const { return lastBrightness; }

    void update();

private:
    std::unique_ptr<TSL2591I2C> sensor;
    uint32_t pollingInterval = 200;
    uint32_t lastPollingTime = 0;
    float lastBrightness = 1;
};

#endif // brightnessSensor_h