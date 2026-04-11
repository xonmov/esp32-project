#include <WiFi.h>
#include "ota.h"
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

const char* ssid = "POCO";
const char* password = "123456789";

Adafruit_MPU6050 mpu;

// Custom I2C pins
#define SDA_PIN D3
#define SCL_PIN D2

void setup() {
  Serial.begin(115200);

  // WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println(WiFi.localIP());

  // OTA
  setupOTA();

  // I2C init with custom pins
  Wire.begin(SDA_PIN, SCL_PIN);

  // MPU init
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 ❌");
    while (1) delay(10);
  }

  Serial.println("MPU6050 Found ✅");

  // Optional settings
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void loop() {
  handleOTA();

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  Serial.print("Accel X: ");
  Serial.print(a.acceleration.x);
  Serial.print(" Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(" Z: ");
  Serial.println(a.acceleration.z);

  Serial.print("Gyro X: ");
  Serial.print(g.gyro.x);
  Serial.print(" Y: ");
  Serial.print(g.gyro.y);
  Serial.print(" Z: ");
  Serial.println(g.gyro.z);

  Serial.println("------");

  delay(500);
}