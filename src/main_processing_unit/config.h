/*
 * config.h — Central configuration for the Main Processing Unit
 *
 * All tuneable constants and pin assignments live here so they can be
 * adjusted in one place without touching the control logic.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ---------------------------------------------------------------------------
// Serial
// ---------------------------------------------------------------------------
#define SERIAL_BAUD    115200

// ---------------------------------------------------------------------------
// Motor speed
// ---------------------------------------------------------------------------
#define MOTOR_SPEED    100   // Default PWM duty cycle (0-255)

// ---------------------------------------------------------------------------
// Tilt detection
// ---------------------------------------------------------------------------
#define TILT_THRESHOLD 6.0   // Acceleration threshold (m/s^2) for movement

// ---------------------------------------------------------------------------
// Pin assignments — Receiver
// ---------------------------------------------------------------------------
#define RECEIVER_PIN   5     // HIGH when RFID unit grants access

// ---------------------------------------------------------------------------
// Pin assignments — Right motor (H-bridge channel A)
// ---------------------------------------------------------------------------
#define RIGHT_FWD_PIN  4     // IN1
#define RIGHT_BWD_PIN  2     // IN2
#define RIGHT_PWM_PIN  11    // ENA (PWM)

// ---------------------------------------------------------------------------
// Pin assignments — Left motor (H-bridge channel B)
// ---------------------------------------------------------------------------
#define LEFT_FWD_PIN   7     // IN3
#define LEFT_BWD_PIN   8     // IN4
#define LEFT_PWM_PIN   10    // ENB (PWM)

#endif // CONFIG_H
