/*
 * RFID Access Control Unit — Headway Motion Wheelchair
 *
 * Reads MIFARE RFID cards via an RC522 module and compares the UID
 * against a stored master tag.  If the card matches, the unit:
 *   1. Displays "Access granted" on a 16x2 I2C LCD
 *   2. Sets the message pin HIGH so the Main Processing Unit enables motors
 *
 * Hardware:
 *   - Arduino Uno / Nano
 *   - MFRC522 RFID reader on SPI (SS=10, RST=9)
 *   - 16x2 I2C LCD at address 0x27
 *   - Message pin (digital 7) wired to Main Unit receiver pin
 */

#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>

// ---------------------------------------------------------------------------
// Pin definitions
// ---------------------------------------------------------------------------
#define RFID_RST_PIN  9   // RC522 reset pin
#define RFID_SS_PIN   10  // RC522 slave-select (SDA) pin
#define MSG_PIN       7   // Output to main processing unit

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
#define SERIAL_BAUD   115200
#define LCD_ADDR      0x27
#define LCD_COLS      16
#define LCD_ROWS      2

// ---------------------------------------------------------------------------
// Global objects
// ---------------------------------------------------------------------------
MFRC522 rfidReader(RFID_SS_PIN, RFID_RST_PIN);
String  masterTag = "D3FD4C2E";  // Authorised RFID tag UID (hex, uppercase)
String  scannedTag = "";

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

// ===========================================================================
// Setup
// ===========================================================================
void setup() {
    pinMode(MSG_PIN, OUTPUT);
    Serial.begin(SERIAL_BAUD);

    // Initialise SPI bus and RFID reader
    SPI.begin();
    rfidReader.PCD_Init();
    delay(4);

    // Initialise LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("System ready");
    delay(3000);
}

// ===========================================================================
// Main loop
// ===========================================================================
void loop() {
    lcd.clear();

    while (readCardUID()) {
        if (scannedTag == masterTag) {
            // Authorised card detected
            lcd.clear();
            lcd.print("User detected");
            delay(2000);
            lcd.setCursor(0, 1);
            lcd.print("Access granted");
            digitalWrite(MSG_PIN, HIGH);
        } else {
            // Unknown card
            lcd.print("Access denied");
            delay(2000);
            lcd.setCursor(0, 1);
            lcd.print("Try again");
        }

        lcd.setCursor(0, 1);
        delay(2000);
    }
}

// ===========================================================================
// Helper — read a card UID into scannedTag
// ===========================================================================

/**
 * Attempt to read an RFID card.
 * @return true if a card was successfully read (UID stored in scannedTag)
 */
bool readCardUID() {
    if (!rfidReader.PICC_IsNewCardPresent()) {
        return false;
    }
    if (!rfidReader.PICC_ReadCardSerial()) {
        return false;
    }

    // Build hex string from UID bytes
    scannedTag = "";
    for (uint8_t i = 0; i < 4; i++) {
        scannedTag.concat(String(rfidReader.uid.uidByte[i], HEX));
    }
    scannedTag.toUpperCase();

    // Halt card communication
    rfidReader.PICC_HaltA();
    return true;
}
