// sensor_module.cpp
#include "sensor_module.h"
#include <ArduinoJson.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include "error_module.h"

// Konstanta
#define DHTPIN D4          // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22      // DHT 22  (AM2302)
#define SDA_PIN D2         // I2C SDA
#define SCL_PIN D1         // I2C SCL
#define BUZZER_PIN D6      // Buzzer
#define RAIN_SENSOR_PIN A0 // Rain sensor

// Inisialisasi sensor
DHT dht(DHTPIN, DHTTYPE);               // Initialize DHT sensor
Adafruit_BMP280 bmp;                    // Initialize BMP sensor
const int thresholdDrizzle = 900;       // Threshold for drizzle
const int thresholdHeavyRain = 500;     // Threshold for heavy rain
const float pressureDropThreshold = 2.0;// Pressure drop threshold


// Inisialisasi sensor dan pin buzzer
void init_sensors() {
  // Sda dan scl pin
  Wire.begin(SDA_PIN, SCL_PIN);
  // DHT
  dht.begin();
  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  // Rain
  digitalWrite(BUZZER_PIN, LOW);
  // BMP
  if (!bmp.begin(0x76)) {
    report_error("bmp280", "Failed to initialize BMP280 sensor");
  } else {
    report_recovery("bmp280");
  }
}


// Fungsi untuk mendeteksi hujan
String detect_rain(int value) {
  if (value < thresholdHeavyRain) return "heavy_rain";
  else if (value < thresholdDrizzle) return "drizzle";
  return "no_rain";
}

// Fungsi untuk menyalakan buzzer
void buzz(String condition) {
  if (condition == "drizzle") {
    for (int i = 0; i < 2; i++) {
      tone(BUZZER_PIN, 800); delay(500); noTone(BUZZER_PIN); delay(500);
    }
  } else if (condition == "heavy_rain") {
    for (int i = 0; i < 5; i++) {
      tone(BUZZER_PIN, 1500); delay(100); noTone(BUZZER_PIN); delay(100);
    }
  } else {
    noTone(BUZZER_PIN);
  }
}


// Fungsi untuk publish data
void publish_data(PubSubClient* client, float& lastPressure) {
  int rain = analogRead(RAIN_SENSOR_PIN);             // Read rain sensor
  String rainCondition = detect_rain(rain);           // Detect rain
  float temp = dht.readTemperature();                 // Read temperature
  float hum = dht.readHumidity();                     // Read humidity
  float pressure = bmp.readPressure() / 100.0F;       // Read pressure
  float altitude = bmp.readAltitude(1013.25);         // Read altitude

  // Check if any reads failed and exit early
  if (isnan(temp) || isnan(hum)) {
    report_error("dht22", "Failed to read DHT22 data");
    return;
  } else {
    report_recovery("dht22");
  }


  // Check if pressure drop
  bool sudden_drop = false;
  if (lastPressure != 0.0 && (lastPressure - pressure) >= pressureDropThreshold)
    sudden_drop = true;

  // Update last pressure
  lastPressure = pressure;

  // Play sound
  buzz(rainCondition);

  // Publish data
	StaticJsonDocument<512> doc;
  doc["temperature"] = temp;
  doc["humidity"] = hum;
  doc["pressure"] = pressure;
  doc["altitude"] = altitude;
  doc["rain_detected"] = rainCondition != "no_rain";
  doc["rain_level"] = rain;
  doc["local_conditions"]["conditions"] = rainCondition;
  doc["local_conditions"]["sudden_pressure_drop"] = sudden_drop;

  char buffer[512];
  serializeJson(doc, buffer);
  client->publish("weather/rain", buffer);
  serializeJsonPretty(doc, Serial); Serial.println();
}
