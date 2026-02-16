/*
 * calibration.h — Startup calibration routine for MPU6050
 *
 * On power-up the sensor's "zero" reading depends on its physical
 * orientation and any manufacturing bias.  CalibrationRoutine samples
 * a configurable number of readings while the user holds still, then
 * computes per-axis offsets.  Subsequent readings can be corrected by
 * subtracting these offsets.
 */

#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// ---------------------------------------------------------------------------
// Default calibration settings
// ---------------------------------------------------------------------------
#define DEFAULT_CALIBRATION_SAMPLES 50   // Number of readings to average
#define CALIBRATION_DELAY_MS        20   // Delay between samples (ms)

// ---------------------------------------------------------------------------
// CalibrationData — stores computed offsets
// ---------------------------------------------------------------------------
struct CalibrationData {
    float offsetX;
    float offsetY;
    float offsetZ;
    bool  valid;  // true after a successful calibration run
};

// ---------------------------------------------------------------------------
// CalibrationRoutine
// ---------------------------------------------------------------------------
class CalibrationRoutine {
public:
    /**
     * @param numSamples Number of readings to average during calibration
     */
    CalibrationRoutine(uint16_t numSamples = DEFAULT_CALIBRATION_SAMPLES);

    /**
     * Run the calibration.  The user should keep the sensor stationary
     * while this is executing.
     *
     * @param sensor  Reference to an initialised Adafruit_MPU6050
     * @return true on success, false if the sensor could not be read
     */
    bool run(Adafruit_MPU6050 &sensor);

    /** Get the computed calibration data. */
    CalibrationData getData() const;

    /**
     * Apply the stored offsets to a raw reading.
     * @param rawX  Raw X acceleration (modified in-place)
     * @param rawY  Raw Y acceleration (modified in-place)
     * @param rawZ  Raw Z acceleration (modified in-place)
     */
    void apply(float &rawX, float &rawY, float &rawZ) const;

private:
    uint16_t        _numSamples;
    CalibrationData _data;
};

#endif // CALIBRATION_H
