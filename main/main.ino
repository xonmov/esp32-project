#include <WiFi.h>
#include "ota.h"
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

const char* ssid = "POCO";
const char* password = "123456789";

Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);

  // 🔌 Connect WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected ✅");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // 🔥 OTA start
  setupOTA();

  // 🔧 I2C with YOUR WORKING PINS
  Wire.begin(D3, D2);  // SDA = D3, SCL = D2

  // 🔍 MPU init (IMPORTANT fix)
  if (!mpu.begin(0x68, &Wire)) {
    Serial.println("Failed to find MPU6050 ❌");
    while (1) delay(10);
  }

  Serial.println("MPU6050 Found ✅");

  // ⚙️ Sensor settings
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void loop() {
  // 🔥 OTA handler
  handleOTA();

  // 📊 Read sensor
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  Serial.print("Accel: ");
  Serial.print(a.acceleration.x); Serial.print(", ");
  Serial.print(a.acceleration.y); Serial.print(", ");
  Serial.println(a.acceleration.z);

  Serial.print("Gyro: ");
  Serial.print(g.gyro.x); Serial.print(", ");
  Serial.print(g.gyro.y); Serial.print(", ");
  Serial.println(g.gyro.z);

  Serial.println("------");

  delay(300);
}