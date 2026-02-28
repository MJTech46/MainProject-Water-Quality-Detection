#include <Arduino.h>

/* ===== PIN CONFIG ===== */
#define TURBIDITY_PIN 33   // change if needed

/* ===== ADC CONFIG ===== */
#define ADC_MAX 4095.0
#define VREF 3.3

/* ===== SAMPLE SETTINGS ===== */
#define SAMPLE_COUNT 200
#define SAMPLE_DELAY 10   // ms between samples

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("=== TURBIDITY IN-AIR CALIBRATION ===");

  analogSetWidth(12);
  analogSetAttenuation(ADC_11db);
  pinMode(TURBIDITY_PIN, INPUT);

  Serial.println("Keep sensor in AIR (no water).");
  Serial.println("Collecting samples...");
  delay(3000);

  float sum = 0;
  float minVal = 4095;
  float maxVal = 0;

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    float reading = analogRead(TURBIDITY_PIN);

    sum += reading;

    if (reading < minVal) minVal = reading;
    if (reading > maxVal) maxVal = reading;

    delay(SAMPLE_DELAY);
  }

  float average = sum / SAMPLE_COUNT;
  float voltage = average * (VREF / ADC_MAX);

  Serial.println("\n=== CALIBRATION RESULTS ===");
  Serial.print("Average Raw Value: ");
  Serial.println(average);

  Serial.print("Minimum Raw Value: ");
  Serial.println(minVal);

  Serial.print("Maximum Raw Value: ");
  Serial.println(maxVal);

  Serial.print("Average Voltage: ");
  Serial.print(voltage, 3);
  Serial.println(" V");

  Serial.println("\nUse the Average Raw Value as your MAX calibration value.");
  Serial.println("======================================");
}

void loop() {
  // Do nothing
}