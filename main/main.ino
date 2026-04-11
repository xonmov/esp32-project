#include <WiFi.h>
#include "ota.h"
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

const char* ssid = "POCO";
const char* password = "123456789";

const int motorPin = D1; 

Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println(WiFi.localIP());

  setupOTA();   // 🔥 just one line

  while (!Serial) delay(10);
// 1. Setup the pins first
  Serial.println("Starting I2C on D3/D2...");
  Wire.begin(D3, D2); 
  
  // 2. Add a tiny delay to let the sensor wake up
  delay(100);

  // 3. Now try to start the library
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    // Check if the I2C scanner still sees it
    while (1) delay(10);
  }
  
  Serial.println("MPU6050 Found!");

}

void loop() {
  handleOTA();  // 🔥 just one line
sensors_event_t a, g, temp;
  
  // Now 'mpu' is declared, so this line will work!
  mpu.getEvent(&a, &g, &temp);

  Serial.print("Accel X: ");
  Serial.print(a.acceleration.x);
  Serial.print(" | Y: ");
  Serial.println(a.acceleration.y);

  delay(100);


}
