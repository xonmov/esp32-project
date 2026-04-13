#ifndef APP_H
#define APP_H

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// external log function
extern void logPrint(String msg);

Adafruit_MPU6050 mpu;

// Angles
float pitch = 0, roll = 0;
unsigned long prevTime = 0;
float alpha = 0.98;

// ===== YOUR CODE =====
void appSetup() {
  logPrint("App Started");

  // I2C start
  Wire.begin(D3, D2);

  // Start MPU (force start for clone)
  mpu.begin(0x68, &Wire);

  logPrint("MPU Started");
}

void appLoop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Time
  unsigned long now = millis();
  float dt = (now - prevTime) / 1000.0;
  prevTime = now;

  // Raw data
  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;

  float gx = g.gyro.x;
  float gy = g.gyro.y;

  // Convert gyro to deg/s
  float gx_deg = gx * 180 / PI;
  float gy_deg = gy * 180 / PI;

  // Accel angle
  float pitchAcc = atan2(-ax, sqrt(ay * ay + az * az)) * 180 / PI;
  float rollAcc  = atan2(ay, az) * 180 / PI;

  // Complementary filter
  pitch = alpha * (pitch + gx_deg * dt) + (1 - alpha) * pitchAcc;
  roll  = alpha * (roll  + gy_deg * dt) + (1 - alpha) * rollAcc;

  // Print using logPrint
  logPrint("Pitch: " + String(pitch) + " | Roll: " + String(roll));

  delay(200);
}

#endif