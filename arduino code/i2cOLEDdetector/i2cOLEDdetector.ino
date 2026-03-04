#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);   // SDA = 21, SCL = 22 for ESP32
  Serial.println("\nI2C Scanner Starting...");
}

void loop() {

  byte error, address;
  int deviceCount = 0;

  Serial.println("Scanning...");

  for(address = 1; address < 127; address++ ) {

    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Device found at address: 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);

      deviceCount++;
    }
  }

  if (deviceCount == 0)
    Serial.println("No I2C devices found.");
  else
    Serial.println("Scan complete.");

  delay(5000);  // Scan every 5 seconds
}