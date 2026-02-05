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

// ---------------------------------------------------------------------------
// Pin definitions
// ---------------------------------------------------------------------------

// Receiver pin — goes HIGH when RFID unit grants access
#define RECEIVER_PIN 5

// Right motor (H-bridge channel A)
#define RIGHT_FWD_PIN  4    // IN1
#define RIGHT_BWD_PIN  2    // IN2
#define RIGHT_PWM_PIN  11   // ENA (PWM speed)

// Left motor (H-bridge channel B)
#define LEFT_FWD_PIN   7    // IN3
#define LEFT_BWD_PIN   8    // IN4
#define LEFT_PWM_PIN   10   // ENB (PWM speed)

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
#define SERIAL_BAUD    115200
#define MOTOR_SPEED    100    // Default PWM duty (0-255)
#define TILT_THRESHOLD 6.00  // Acceleration threshold for tilt detection (m/s^2)

// ---------------------------------------------------------------------------
// Global objects
// ---------------------------------------------------------------------------
Adafruit_MPU6050 head;  // MPU6050 sensor instance

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

        // Read accelerometer data
        sensors_event_t a, g, temp;
        head.getEvent(&a, &g, &temp);

        float tiltY = a.acceleration.y;  // left / right tilt
        float tiltX = a.acceleration.x;  // forward / backward tilt

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
