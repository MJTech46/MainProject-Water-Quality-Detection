#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <math.h>

/* ================= PIN DEFINITIONS ================= */
#define TDS_PIN        36
#define TURBIDITY_PIN  33
#define ONE_WIRE_BUS    4
#define TRIG_PIN       18
#define ECHO_PIN       19

/* ================= ADC SETTINGS ================= */
#define ADC_MAX 4095.0
#define VREF    3.3

/* ================= WiFi & SERVER CONFIG ================= */
const char* ssid = "WATER1234";
const char* password = "WATER1234";
String apiKeyValue = "tPmAT5Ab3j7F9";
const char* SENSOR_NAME = "WATER_QUALITY_01";
const char* SENSOR_LOCATION = "TANK_1";
String serverURL = "https://seccloudstorage.in/ieswaterqualitysensordata/post-esp-data.php";

/* ================= OBJECTS ================= */
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

/* ================= ULTRASONIC FUNCTION ================= */
float readUltrasonicCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.0343 / 2.0;
}

/* ================= WATER QUALITY CHECK FUNCTION ================= */
bool isWaterGood(float tds, float clarity, float tempC) {
  if (tds <= 500 && clarity >= 60 && tempC >= 5 && tempC <= 40) {
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);

  /* ===== Proper ESP32 ADC Setup ===== */
  analogSetWidth(12);
  analogSetAttenuation(ADC_11db);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  tempSensor.begin();

  Wire.begin(21, 22);
  oled.begin();
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(0, 12, "Water Quality System");
  oled.drawStr(0, 28, "Initializing...");
  oled.sendBuffer();
  delay(1500);

  /* ===== WiFi ===== */
  oled.clearBuffer();
  oled.drawStr(0, 12, "Connecting WiFi...");
  oled.sendBuffer();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  oled.clearBuffer();
  oled.drawStr(0, 12, "WiFi Connected!");
  oled.sendBuffer();
  delay(1500);
}

void loop() {

  /* ========== TEMPERATURE ========== */
  tempSensor.requestTemperatures();
  float tempC = tempSensor.getTempCByIndex(0);

  /* ========== TDS (STABLE VERSION) ========== */
  float tdsRaw = 0;
  for (int i = 0; i < 10; i++) {
    tdsRaw += analogRead(TDS_PIN);
    delay(10);
  }
  tdsRaw /= 10.0;

  float tdsVoltage = (tdsRaw * VREF) / ADC_MAX;

  float compensationCoefficient = 1.0 + 0.02 * (tempC - 25.0);
  float compensatedVoltage = tdsVoltage / compensationCoefficient;

  float tdsPPM = (133.42 * compensatedVoltage * compensatedVoltage * compensatedVoltage
                - 255.86 * compensatedVoltage * compensatedVoltage
                + 857.39 * compensatedVoltage) * 0.5;

  if (tdsPPM < 0) tdsPPM = 0;

  /* ========== TURBIDITY ========== */
  float turbRaw = analogRead(TURBIDITY_PIN);
  float clarity = (turbRaw / 1500.0) * 100.0;
  clarity = constrain(clarity, 0, 100);

  /* ========== ULTRASONIC ========== */
  float distanceCM = readUltrasonicCM();

  /* ========== REFRACTIVE INDEX ========== */
  float refractiveIndex = 1.3330 + (tdsPPM * 0.0000016);

  /* ========== QUALITY CHECK ========== */
  bool waterGood = isWaterGood(tdsPPM, clarity, tempC);
  String qualityStatus = waterGood ? "GOOD" : "BAD";

  /* ========== SERIAL DEBUG ========== */
  Serial.print("Temp: "); Serial.print(tempC, 1);
  Serial.print(" | RAW: "); Serial.print(tdsRaw);
  Serial.print(" | Volt: "); Serial.print(tdsVoltage, 3);
  Serial.print(" | TDS: "); Serial.print(tdsPPM, 1);
  Serial.print(" | Clarity: "); Serial.print(clarity, 1);
  Serial.print(" | Level: "); Serial.print(distanceCM, 1);
  Serial.print(" | Water: ");
  Serial.println(qualityStatus);

  /* ========== OLED DISPLAY ========== */
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x10_tf);

  oled.setCursor(0, 12);
  oled.print("Temp: "); oled.print(tempC, 1); oled.print(" C");

  oled.setCursor(0, 24);
  oled.print("TDS: "); oled.print(tdsPPM, 0); oled.print(" ppm");

  oled.setCursor(0, 36);
  oled.print("Clarity: "); oled.print(clarity, 0); oled.print(" %");

  oled.setCursor(0, 48);
  oled.print("Level: ");
  if (distanceCM < 0) oled.print("--");
  else {
    oled.print(distanceCM, 0);
    oled.print(" cm");
  }

  oled.setFont(u8g2_font_7x14_tf);
  oled.setCursor(0, 63);
  oled.print("Water: ");
  oled.print(qualityStatus);

  oled.sendBuffer();

  /* ========== SEND DATA TO SERVER ========== */
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData =
      "api_key=" + apiKeyValue +
      "&sensor=" + String(SENSOR_NAME) +
      "&location=" + String(SENSOR_LOCATION) +
      "&value1=" + String(tempC) +
      "&value2=" + String(tdsPPM) +
      "&value3=" + String(clarity) +
      "&value4=" + String(distanceCM) +
      "&value5=" + String(refractiveIndex) +
      "&value6=" + qualityStatus;

    int httpResponseCode = http.POST(postData);

    Serial.print("HTTP Response: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {
      Serial.println(http.getString());
    }

    http.end();
  }

  delay(2000);
}