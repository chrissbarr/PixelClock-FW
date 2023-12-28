#ifndef brightnessSensor_h
#define brightnessSensor_h

/* C++ Standard Library */
#include <cstdint>
#include <memory>

class BrightnessSensor {
public:
    virtual float getBrightness() const = 0;
    virtual void update() = 0;
};

#ifndef PIXELCLOCK_DESKTOP
class TSL2591I2C;

class BrightnessSensorTSL2591 : public BrightnessSensor {
public:
    BrightnessSensorTSL2591();
    ~BrightnessSensorTSL2591();
    float getBrightness() const override final { return lastBrightness; }
    void update() override final;

private:
    std::unique_ptr<TSL2591I2C> sensor;
    uint32_t pollingInterval = 200;
    uint32_t lastPollingTime = 0;
    float lastBrightness = 1;
};

#endif

class BrightnessSensorDummy : public BrightnessSensor {
public:
    BrightnessSensorDummy(float val) : val(val) {}
    ~BrightnessSensorDummy() {}
    float getBrightness() const override final { return val; }
    void update() override final {}

private:
    float val;
};

#endif // brightnessSensor_h