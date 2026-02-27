# Headway Motion Wheelchair

A head-movement controlled wheelchair prototype using an MPU6050 gyroscope/accelerometer for tilt detection and an MFRC522 RFID module for access control. The user tilts their head to steer, and the wheelchair only operates when an authorised RFID card has been scanned.

## How It Works

The system consists of two Arduino-based units connected by a single signal wire:

1. **RFID Unit** -- Reads MIFARE cards via an RC522 module. If the scanned UID matches the stored master tag, it sets a digital pin HIGH to authorise the main unit. A 16x2 I2C LCD provides visual feedback.

2. **Main Processing Unit** -- Continuously reads the MPU6050 accelerometer while the authorisation pin is HIGH. Head tilt along the X-axis drives forward/backward; tilt along the Y-axis triggers left/right pivoting. A dead zone around the neutral position prevents unintended movement.

## Hardware Requirements

| Component              | Qty | Notes                              |
|------------------------|-----|------------------------------------|
| Arduino Uno or Mega    | 2   | One per unit                       |
| MPU6050 (GY-521)       | 1   | Mounted on user's head             |
| L298N Motor Driver     | 1   | Dual H-bridge for 2 DC motors     |
| DC Geared Motors       | 2   | 12V recommended                    |
| MFRC522 RFID Module    | 1   | 13.56 MHz, 3.3V logic              |
| MIFARE Classic Card    | 1+  | Pre-programmed authorised tag      |
| 16x2 I2C LCD           | 1   | PCF8574 backpack at 0x27           |
| 12V Battery / PSU      | 1   | Motor power supply                 |
| Jumper wires           | --  | Various M-M, M-F                   |

## Software Dependencies

Install via the Arduino IDE Library Manager:

- **Adafruit MPU6050** (and its dependency Adafruit Unified Sensor)
- **MFRC522** by miguelbalboa
- **LiquidCrystal I2C** by Frank de Brabander

See `requirements.txt` for the full list.

## Wiring Overview

Full pin-by-pin tables and an ASCII system diagram are in [`docs/wiring.md`](docs/wiring.md). Key connections:

- **MPU6050** connects to the main Arduino via I2C (A4/A5).
- **L298N** motor driver uses 6 digital pins (direction + PWM for each motor).
- **MFRC522** connects to the RFID Arduino via SPI (pins 9-13).
- **Signal wire** from RFID unit pin 7 to main unit pin 5 carries the authorisation signal.
- Both Arduinos must share a common GND.

## Project Structure

```
Headway-Motion-Wheelchair/
  src/
    main_processing_unit/
      main_processing_unit.ino   -- Motor control + gyro reading
      config.h                   -- Pin assignments and constants
      calibration.h / .cpp       -- Startup zero-offset calibration
      filters.h / .cpp           -- MovingAverage and LowPass filters
    rfid_unit/
      rfid_unit.ino              -- RFID reader + LCD access control
  tests/
    test_filters.py              -- Python unit tests for filter algorithms
  docs/
    wiring.md                    -- Pin tables and ASCII wiring diagram
  requirements.txt               -- Arduino library dependencies
  .gitignore
  README.md
```

## Uploading the Code

1. Open the Arduino IDE (1.8+ or 2.x).
2. Install the libraries listed in **Software Dependencies** above.
3. Open `src/main_processing_unit/main_processing_unit.ino`.
4. Select the correct board and port, then click Upload.
5. Open `src/rfid_unit/rfid_unit.ino` in a separate IDE window.
6. Select the RFID Arduino's board and port, then click Upload.

## Calibration

On power-up the main unit runs an automatic calibration routine:

1. Keep the MPU6050 sensor **stationary and level** for about 1 second.
2. The serial monitor will display progress dots and the computed X/Y/Z offsets.
3. Once calibration finishes, the system is ready -- tilt your head to drive.

The calibration compensates for mounting angle and sensor manufacturing bias. If readings seem off, power-cycle and re-calibrate on a flat surface.

## Running Tests

The filter algorithms have Python unit tests (no Arduino hardware required):

```bash
python3 -m pytest tests/test_filters.py -v
```

## Safety Disclaimer

This is an educational prototype. It is **not** a medical device and has not been tested for real-world wheelchair use. If you adapt this project for actual mobility assistance:

- Add emergency stop hardware (physical kill switch).
- Implement speed ramping and maximum speed limits.
- Add obstacle detection (ultrasonic / LiDAR sensors).
- Perform thorough testing in a controlled environment before any real use.
- Consult applicable safety standards for powered mobility devices.
