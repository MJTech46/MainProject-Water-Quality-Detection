#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <HTTPClient.h>

/* ================= PIN DEFINITIONS ================= */
#define TDS_PIN        36
#define TURBIDITY_PIN  33
#define ONE_WIRE_BUS    4
#define TRIG_PIN       18
#define ECHO_PIN       19

/* ================= ADC SETTINGS ================= */
#define ADC_MAX 4095.0
#define VREF    3.3

/* ================= RI CONSTANTS ================= */
#define TDS_MAX        1000.0
#define TEMP_CENTER    30.0
#define TEMP_RANGE     10.0

/* ================= WiFi CONFIG ================= */
const char* ssid = "WATER1234";
const char* password = "WATER1234";

String apiKeyValue = "tPmAT5Ab3j7F9";
const char* SENSOR_NAME = "WATER_QUALITY_01";
const char* SENSOR_LOCATION = "TANK_1";

String serverURL =
"https://team7.share.zrok.io/api/water-data/";

/* ================= OBJECTS ================= */
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);

/* SH1106 1.3" OLED */
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

/* ================= HELPER FUNCTIONS ================= */

float clamp01(float x)
{
  if (x < 0) return 0;
  if (x > 1) return 1;
  return x;
}

/* ================= RI CALCULATION ================= */

float computeRI(float tempC, float tdsPPM, float turbPercent)
{
  float turbFactor = clamp01(turbPercent / 100.0);
  float tdsFactor  = clamp01(1.0 - (tdsPPM / TDS_MAX));
  float tempFactor = clamp01(1.0 - (abs(tempC - TEMP_CENTER) / TEMP_RANGE));

  float RI = 100.0 * (
      0.45 * turbFactor +
      0.45 * tdsFactor +
      0.10 * tempFactor
  );

  /* WHO safety overrides */

  if(tdsPPM > 1500)
      RI = min(RI, 20.0f);

  else if(tdsPPM > 1000)
      RI = min(RI, 35.0f);

  return RI;
}

/* ================= RI STATUS ================= */

String getRIStatus(float RI)
{
  if (RI >= 80) return "EXCELLENT";
  if (RI >= 60) return "GOOD";
  if (RI >= 40) return "MODERATE";
  return "POOR";
}

/* ================= ULTRASONIC ================= */

float readUltrasonicCM()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) return -1;

  return duration * 0.0343 / 2.0;
}

/* ================= SETUP ================= */

void setup()
{

  Serial.begin(115200);

  analogSetWidth(12);
  analogSetAttenuation(ADC_11db);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  tempSensor.begin();

  Wire.begin(21,22);

  oled.begin();
  oled.clearBuffer();

  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(20,20,"AI Water Monitor");
  oled.drawStr(30,35,"Starting...");
  oled.sendBuffer();

  delay(1500);

  /* ===== WiFi ===== */

  oled.clearBuffer();
  oled.drawStr(25,30,"Connecting WiFi");
  oled.sendBuffer();

  WiFi.begin(ssid,password);

  while(WiFi.status()!=WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");

  oled.clearBuffer();
  oled.drawStr(25,30,"WiFi Connected");
  oled.sendBuffer();

  delay(1500);
}

/* ================= LOOP ================= */

void loop()
{

  /* ===== TEMPERATURE ===== */

  tempSensor.requestTemperatures();
  float tempC = tempSensor.getTempCByIndex(0);

  /* ===== TDS SENSOR ===== */

  float tdsRaw = 0;

  for(int i=0;i<10;i++)
  {
    tdsRaw += analogRead(TDS_PIN);
    delay(10);
  }

  tdsRaw /= 10.0;

  float voltage = (tdsRaw * VREF) / ADC_MAX;

  float compensationCoefficient =
  1.0 + 0.02 * (tempC - 25.0);

  float compensatedVoltage =
  voltage / compensationCoefficient;

  float tdsPPM =
  (133.42 * compensatedVoltage * compensatedVoltage * compensatedVoltage
  -255.86 * compensatedVoltage * compensatedVoltage
  +857.39 * compensatedVoltage) * 0.5;

  if(tdsPPM < 0) tdsPPM = 0;

  /* ===== TURBIDITY ===== */

  float turbRaw = analogRead(TURBIDITY_PIN);

  float turbPercent =
  (turbRaw / 1500.0) * 100.0;

  turbPercent = constrain(turbPercent,0,100);

  /* ===== WATER LEVEL ===== */

  float distanceCM = readUltrasonicCM();

  /* ===== RI CALCULATION ===== */

  float RI = computeRI(tempC,tdsPPM,turbPercent);

  String riStatus = getRIStatus(RI);

  /* ===== SERIAL DEBUG ===== */

  Serial.print("Temp: ");
  Serial.print(tempC);

  Serial.print(" | TDS: ");
  Serial.print(tdsPPM);

  Serial.print(" | Turb: ");
  Serial.print(turbPercent);

  Serial.print(" | Level: ");
  Serial.print(distanceCM);

  Serial.print(" | RI: ");
  Serial.print(RI);

  Serial.print(" | Status: ");
  Serial.println(riStatus);

  /* ===== OLED DISPLAY ===== */

  oled.clearBuffer();

  oled.setFont(u8g2_font_6x10_tf);

  /* Level */
  oled.setCursor(0,10);
  oled.print("Level : ");

  if(distanceCM < 0)
    oled.print("--");
  else
  {
    oled.print(distanceCM,0);
    oled.print(" cm");
  }

  /* Temperature */
  oled.setCursor(0,22);
  oled.print("Temp  : ");
  oled.print(tempC,1);
  oled.print(" C");

  /* TDS */
  oled.setCursor(0,34);
  oled.print("TDS   : ");
  oled.print(tdsPPM,0);
  oled.print(" ppm");

  /* Turbidity */
  oled.setCursor(0,46);
  oled.print("Turb  : ");
  oled.print(turbPercent,0);
  oled.print(" %");

  /* Divider */
  oled.drawStr(0,54,"----------------");

  /* RI Section */
  oled.setFont(u8g2_font_7x14_tf);

  oled.setCursor(0,64);
  oled.print("RI:");
  oled.print(RI,0);

  oled.setCursor(70,64);
  oled.print(riStatus);

  oled.sendBuffer();

  /* ===== SEND DATA TO SERVER ===== */

  if(WiFi.status()==WL_CONNECTED)
  {
    HTTPClient http;

    http.begin(serverURL);

    http.addHeader("Content-Type","application/x-www-form-urlencoded");

    String postData =
    "api_key=" + apiKeyValue +
    "&sensor=" + String(SENSOR_NAME) +
    "&location=" + String(SENSOR_LOCATION) +
    "&value1=" + String(tempC) +
    "&value2=" + String(tdsPPM) +
    "&value3=" + String(turbPercent) +
    "&value4=" + String(distanceCM) +
    "&value5=" + String(RI) +
    "&value6=" + riStatus;

    int httpResponseCode = http.POST(postData);

    Serial.print("HTTP Response: ");
    Serial.println(httpResponseCode);

    if(httpResponseCode > 0)
    {
      Serial.println(http.getString());
    }

    http.end();
  }

  delay(2000);
}
