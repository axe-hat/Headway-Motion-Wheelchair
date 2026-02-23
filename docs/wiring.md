# Wiring Guide

## Component List

| # | Component              | Qty | Notes                                |
|---|------------------------|-----|--------------------------------------|
| 1 | Arduino Uno / Mega     | 2   | One for main unit, one for RFID unit |
| 2 | MPU6050 (GY-521)       | 1   | 6-axis accelerometer / gyroscope     |
| 3 | L298N Motor Driver     | 1   | Dual H-bridge, supports 2 DC motors  |
| 4 | DC Geared Motors       | 2   | 12V recommended                      |
| 5 | MFRC522 RFID Module    | 1   | 13.56 MHz MIFARE reader              |
| 6 | MIFARE Classic Card    | 1+  | At least one authorised tag          |
| 7 | 16x2 I2C LCD           | 1   | PCF8574 backpack, address 0x27       |
| 8 | Jumper Wires           | --  | Male-to-male, male-to-female         |
| 9 | 12V Battery / PSU      | 1   | Powers motors via L298N              |
| 10| Breadboard (optional)  | 1   | For prototyping connections           |

## Main Processing Unit Wiring

### MPU6050 to Arduino (I2C)

```
  MPU6050          Arduino
  -------          -------
  VCC  ---------- 5V
  GND  ---------- GND
  SDA  ---------- A4  (I2C data)
  SCL  ---------- A5  (I2C clock)
```

### L298N Motor Driver to Arduino

```
  L298N            Arduino          Motor
  -----            -------          -----
  IN1  ---------- Pin 4  (RIGHT_FWD)
  IN2  ---------- Pin 2  (RIGHT_BWD)
  ENA  ---------- Pin 11 (RIGHT_PWM)  ---- Right Motor +/-
  IN3  ---------- Pin 7  (LEFT_FWD)
  IN4  ---------- Pin 8  (LEFT_BWD)
  ENB  ---------- Pin 10 (LEFT_PWM)   ---- Left Motor +/-
  12V  ---------- Battery +
  GND  ---------- Battery - / Arduino GND (common ground)
  5V   ---------- (can power Arduino if jumper is set)
```

### RFID Receiver Signal

```
  RFID Unit Pin 7 (MSG_PIN) ---- Main Unit Pin 5 (RECEIVER_PIN)
  RFID Unit GND  --------------- Main Unit GND   (common ground)
```

## RFID Unit Wiring

### MFRC522 to Arduino (SPI)

```
  MFRC522          Arduino
  -------          -------
  SDA  ---------- Pin 10 (SS)
  SCK  ---------- Pin 13 (SCK)
  MOSI ---------- Pin 11 (MOSI)
  MISO ---------- Pin 12 (MISO)
  IRQ  ---------- (not connected)
  GND  ---------- GND
  RST  ---------- Pin 9
  3.3V ---------- 3.3V  (important: NOT 5V)
```

### 16x2 I2C LCD to Arduino

```
  LCD (I2C)        Arduino
  ---------        -------
  VCC  ---------- 5V
  GND  ---------- GND
  SDA  ---------- A4
  SCL  ---------- A5
```

### Message Output to Main Unit

```
  Arduino Pin 7 (MSG_PIN) ---- Main Unit Pin 5 (RECEIVER_PIN)
```

## Full System Diagram (ASCII)

```
                         +--- 12V Battery ---+
                         |                   |
  +------------+    +----+----+         +---------+
  |  MPU6050   |    |  L298N  |         |  Motor  |
  | (on head)  |    |  Motor  +---------+  Right  |
  |   SDA/SCL  |    | Driver  |         +---------+
  +-----+------+    |         |         +---------+
        |  I2C      |         +---------+  Motor  |
  +-----+------+    +----+----+         |  Left   |
  |  Arduino   |         |             +---------+
  |  MAIN UNIT |--Pins---+
  |            |
  | Pin 5 <----+---- Pin 7 ---+
  +------------+    (wire)    |
                         +----+-------+    +-----------+
                         |  Arduino   |    |  MFRC522  |
                         | RFID UNIT  +----+   RFID    |
                         |            |SPI |  Reader   |
                         |   A4/A5 ---+    +-----------+
                         +----+-------+
                              |  I2C
                         +----+-------+
                         | 16x2 LCD   |
                         | (I2C 0x27) |
                         +------------+
```

## Important Notes

- The MFRC522 runs on 3.3V logic. Connect its VCC to the Arduino 3.3V pin, not 5V.
- Both Arduinos must share a common GND for the receiver signal to work.
- Remove the ENA/ENB jumpers on the L298N so PWM speed control works.
- The MPU6050 is mounted on the user's head (e.g., attached to a headband).
