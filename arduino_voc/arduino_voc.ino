/*
 * Arduino Uno - VOC Detector
 * Reads an MQ-138 VOC sensor, notes the value on a 16x2 LCD and sounds a buzzer 
 * when it reaches over a threshold, and forwards each reading to an ESP32 over a
 * SoftwareSerial link. The ESP32 then publishes it over BLE to the web page.
 */

#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

// LCD pins: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// SoftwareSerial link to the ESP32: RX = D6, TX = D7
SoftwareSerial espSerial(6, 7); // RX, TX

const int MQ138_PIN = A0;
const int BUZZER_PIN = 8;
const int VOC_THRESHOLD = 300;

int voc_value = 0;

void setup() {
  // Sensor needs time before giving accurate raedings
  delay(60000);

  lcd.begin(16, 2);
  lcd.noAutoscroll();
  lcd.clear();
  lcd.print("VOC Detector");

  pinMode(BUZZER_PIN, OUTPUT);

  Serial.begin(115200);  // USB serial for debugging
  espSerial.begin(9600); // Link to ESP32
}

void loop() {
  voc_value = analogRead(MQ138_PIN); // uncalibrated 0-1023

  // Update the LCD's second line
  lcd.setCursor(0, 1);
  lcd.print("                "); // clear the line
  lcd.setCursor(0, 1);
  lcd.print("VOC: ");
  lcd.print(voc_value);

  // Debug over USB
  Serial.println(voc_value);

  // Send the reading to the ESP32 (one value per line)
  espSerial.println(voc_value);

  // Buzzer alert when the reading crosses the threshold
  digitalWrite(BUZZER_PIN, (voc_value > VOC_THRESHOLD) ? HIGH : LOW);

  delay(1000);
}
