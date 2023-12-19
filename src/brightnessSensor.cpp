#include "brightnessSensor.h"

#include <TSL2591I2C.h>

BrightnessSensor::BrightnessSensor() {
    Serial.print("Initialising TSL2591: ");
    sensor = std::make_unique<TSL2591I2C>();

    if (!sensor->begin()) {
        Serial.println("Error!");
        sensor.reset();
    }

    if (sensor) {
        Serial.println("Success!");
        Serial.print("Sensor ID: ");
        Serial.println(sensor->getID(), HEX);
        sensor->resetToDefaults();
        sensor->setChannel(TSL2591MI::TSL2591_CHANNEL_0);
        sensor->setGain(TSL2591MI::TSL2591_GAIN_MED);
        sensor->setIntegrationTime(TSL2591MI::TSL2591_INTEGRATION_TIME_100ms);

        Serial.println("Performing test measurement...");

        if (!sensor->measure()) {
            Serial.println("Could not start measurement. ");
            sensor.reset();
        } else {
            while (!sensor->hasValue()) { delay(1); }
            Serial.print("Irradiance: ");
            Serial.print(sensor->getIrradiance(), 7);
            Serial.println(" W / m^2");
            Serial.print("Brightness: ");
            Serial.print(sensor->getBrightness(), 7);
            Serial.println(" lux");
        }
    }
}

BrightnessSensor::~BrightnessSensor() {}

void BrightnessSensor::update() {
    if (millis() - lastPollingTime > pollingInterval) {
        if (sensor) {
            if (sensor->hasValue()) {
                float brightness = sensor->getBrightness();
                float irradiance = sensor->getIrradiance();
                // Serial.print("Irradiance: "); Serial.print(irradiance, 7); Serial.println(" W / m^2");
                // Serial.print("Brightness: "); Serial.print(brightness, 7); Serial.println(" lux");
                lastBrightness = brightness;
            }
        }
        lastPollingTime = millis();
    }
}