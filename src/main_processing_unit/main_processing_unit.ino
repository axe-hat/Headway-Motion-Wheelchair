/*
 * Main Processing Unit — Headway Motion Wheelchair
 *
 * Reads head-tilt data from an MPU6050 accelerometer/gyroscope and
 * translates it into motor commands for a two-motor differential-drive
 * wheelchair.  An external RFID unit gates access: the wheelchair moves
 * only when the receiver pin is HIGH (RFID authorised).
 *
 * Hardware:
 *   - Arduino Uno / Mega
 *   - MPU6050 on I2C (SDA/SCL)
 *   - L298N dual H-bridge motor driver
 *   - Receiver pin from RFID unit (digital pin 5)
 */

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#include "config.h"       // Pin assignments, thresholds, and tuneable constants
#include "calibration.h"  // Startup calibration routine

// ---------------------------------------------------------------------------
// Global objects
// ---------------------------------------------------------------------------
Adafruit_MPU6050   head;   // MPU6050 sensor instance
CalibrationRoutine calib;  // Startup calibration (computes axis offsets)

// ===========================================================================
// Setup
// ===========================================================================
void setup(void) {
    Wire.begin();
    pinMode(LED_BUILTIN, OUTPUT);

    // --- Motor pins ---
    // Right motor
    pinMode(RIGHT_FWD_PIN, OUTPUT);
    pinMode(RIGHT_BWD_PIN, OUTPUT);
    pinMode(RIGHT_PWM_PIN, OUTPUT);
    // Left motor
    pinMode(LEFT_FWD_PIN, OUTPUT);
    pinMode(LEFT_BWD_PIN, OUTPUT);
    pinMode(LEFT_PWM_PIN, OUTPUT);

    // --- Serial / Gyroscope initialisation ---
    Serial.begin(SERIAL_BAUD);
    while (!Serial) {
        delay(10);
    }

    Serial.println("Gyroscope coming online...");

    if (!head.begin()) {
        Serial.println("ERROR: MPU6050 not detected — check wiring");
        while (1) {
            delay(10);
        }
    }

    Serial.println("MPU6050 initialised successfully");
    head.setAccelerometerRange(MPU6050_RANGE_2_G);
    Serial.println("Accelerometer range set to +/- 2G");
    Serial.println();

    // --- Run startup calibration ---
    // The user should keep the sensor stationary during this phase.
    calib.run(head);

    Serial.println("Setup complete — ready for input");
    delay(100);
}

// ===========================================================================
// Motor helpers
// ===========================================================================

/**
 * Drive the right motor.
 * @param dir  1 = forward, 0 = backward
 * @param rate PWM duty cycle (0-255)
 */
void rightMotor(int dir, int rate) {
    if (dir == 1) {
        digitalWrite(RIGHT_FWD_PIN, HIGH);
        digitalWrite(RIGHT_BWD_PIN, LOW);
    } else {
        digitalWrite(RIGHT_FWD_PIN, LOW);
        digitalWrite(RIGHT_BWD_PIN, HIGH);
    }
    analogWrite(RIGHT_PWM_PIN, rate);
}

/**
 * Drive the left motor.
 * @param dir  1 = forward, 0 = backward
 * @param rate PWM duty cycle (0-255)
 */
void leftMotor(int dir, int rate) {
    if (dir == 1) {
        digitalWrite(LEFT_BWD_PIN, LOW);
        digitalWrite(LEFT_FWD_PIN, HIGH);
    } else {
        digitalWrite(LEFT_BWD_PIN, HIGH);
        digitalWrite(LEFT_FWD_PIN, LOW);
    }
    analogWrite(LEFT_PWM_PIN, rate);
}

/**
 * Stop both motors immediately.
 */
void stopMotors() {
    rightMotor(0, 0);
    leftMotor(0, 0);
}

// ===========================================================================
// Main loop
// ===========================================================================
void loop() {
    // The wheelchair only responds to head tilt while the RFID receiver
    // pin is HIGH (access granted by the RFID unit).
    while (digitalRead(RECEIVER_PIN)) {
        digitalWrite(LED_BUILTIN, HIGH);

        // Read accelerometer data and apply calibration offsets
        sensors_event_t a, g, temp;
        head.getEvent(&a, &g, &temp);

        float tiltX = a.acceleration.x;  // forward / backward tilt
        float tiltY = a.acceleration.y;  // left / right tilt
        float tiltZ = a.acceleration.z;
        calib.apply(tiltX, tiltY, tiltZ);

        // --- Movement decision tree ---
        if (tiltY <= TILT_THRESHOLD) {
            if (tiltY >= -TILT_THRESHOLD) {
                // Y-axis within dead zone — check X for forward/backward
                if (tiltX >= TILT_THRESHOLD) {
                    // Tilt forward — drive forward
                    rightMotor(1, MOTOR_SPEED);
                    leftMotor(1, MOTOR_SPEED);
                } else if (tiltX <= -TILT_THRESHOLD) {
                    // Tilt backward — drive backward
                    rightMotor(0, MOTOR_SPEED);
                    leftMotor(0, MOTOR_SPEED);
                } else {
                    // Head level — stop
                    stopMotors();
                }
            } else {
                // Tilt right — pivot right (right backward, left forward)
                rightMotor(0, MOTOR_SPEED);
                leftMotor(1, MOTOR_SPEED);
            }
        } else {
            // Tilt left — pivot left (right forward, left backward)
            rightMotor(1, MOTOR_SPEED);
            leftMotor(0, MOTOR_SPEED);
        }
    }
}
