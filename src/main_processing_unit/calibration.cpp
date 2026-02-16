/*
 * calibration.cpp — Implementation of CalibrationRoutine
 */

#include "calibration.h"

// ===========================================================================
// Constructor
// ===========================================================================
CalibrationRoutine::CalibrationRoutine(uint16_t numSamples) {
    _numSamples    = numSamples;
    _data.offsetX  = 0.0f;
    _data.offsetY  = 0.0f;
    _data.offsetZ  = 0.0f;
    _data.valid    = false;
}

// ===========================================================================
// run — collect samples and compute mean offset
// ===========================================================================
bool CalibrationRoutine::run(Adafruit_MPU6050 &sensor) {
    float sumX = 0.0f;
    float sumY = 0.0f;
    float sumZ = 0.0f;

    Serial.println("Calibrating — hold the sensor still...");

    for (uint16_t i = 0; i < _numSamples; i++) {
        sensors_event_t a, g, temp;
        sensor.getEvent(&a, &g, &temp);

        sumX += a.acceleration.x;
        sumY += a.acceleration.y;
        sumZ += a.acceleration.z;

        delay(CALIBRATION_DELAY_MS);

        // Print a dot every 10 samples for progress feedback
        if ((i + 1) % 10 == 0) {
            Serial.print(".");
        }
    }

    Serial.println(" done");

    _data.offsetX = sumX / _numSamples;
    _data.offsetY = sumY / _numSamples;
    _data.offsetZ = sumZ / _numSamples;
    _data.valid   = true;

    Serial.print("Offsets — X: ");
    Serial.print(_data.offsetX, 2);
    Serial.print("  Y: ");
    Serial.print(_data.offsetY, 2);
    Serial.print("  Z: ");
    Serial.println(_data.offsetZ, 2);

    return true;
}

// ===========================================================================
// getData
// ===========================================================================
CalibrationData CalibrationRoutine::getData() const {
    return _data;
}

// ===========================================================================
// apply — subtract offsets from raw readings
// ===========================================================================
void CalibrationRoutine::apply(float &rawX, float &rawY, float &rawZ) const {
    if (_data.valid) {
        rawX -= _data.offsetX;
        rawY -= _data.offsetY;
        rawZ -= _data.offsetZ;
    }
}
