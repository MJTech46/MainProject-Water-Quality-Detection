#include <Arduino.h>
#include <WiFi.h>

/* ================= PIN DEFINITIONS ================= */
#define TDS_PIN 36

/* ================= ADC SETTINGS ================= */
#define ADC_MAX 4095.0
#define VREF    3.3

void setup() {
  Serial.begin(115200);

  // Proper ESP32 ADC setup
  analogSetWidth(12);                 // 12-bit resolution
  analogSetAttenuation(ADC_11db);     // Full 0–3.3V range

  pinMode(TDS_PIN, INPUT);

  Serial.println("TDS Sensor Debug Start...");
}

void loop() {

  /* ========== RAW ADC READ ========== */
  int tdsRaw = analogRead(TDS_PIN);

  /* ========== VOLTAGE CALCULATION ========== */
  float tdsVoltage = (tdsRaw / ADC_MAX) * VREF;

  /* ========== SIMPLE TDS CALCULATION (NO TEMP COMP) ========== */
  float tdsPPM = (133.42 * pow(tdsVoltage, 3)
                - 255.86 * pow(tdsVoltage, 2)
                + 857.39 * tdsVoltage) * 0.5;

  if (tdsPPM < 0) tdsPPM = 0;

  /* ========== SERIAL OUTPUT ========== */
  Serial.print("RAW ADC: ");
  Serial.print(tdsRaw);

  Serial.print(" | Voltage: ");
  Serial.print(tdsVoltage, 3);

  Serial.print(" V | TDS: ");
  Serial.print(tdsPPM, 1);
  Serial.println(" ppm");

  delay(2000);
}