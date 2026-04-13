#ifndef APP_H
#define APP_H

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// external log function
extern void logPrint(String msg);

Adafruit_MPU6050 mpu;
bool mpu_ok = false;

// ===== YOUR CODE =====
void appSetup() {
  logPrint("App Started");

  // Start I2C
  Wire.begin(D3, D2);

  // Try to start MPU (force start)
  if (mpu.begin(0x68, &Wire)) {
    logPrint("MPU6050 FOUND ✅");
    mpu_ok = true;
  } else {
    logPrint("MPU NOT FOUND ❌ (trying anyway)");
    mpu.begin(0x68, &Wire);  // force start for clone sensors
    mpu_ok = true; // allow reading even if check fails
  }
}

void appLoop() {
  if (mpu_ok) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    logPrint("Accel X: " + String(a.acceleration.x) +
             " Y: " + String(a.acceleration.y) +
             " Z: " + String(a.acceleration.z));

    logPrint("Gyro X: " + String(g.gyro.x) +
             " Y: " + String(g.gyro.y) +
             " Z: " + String(g.gyro.z));

    logPrint("Temp: " + String(temp.temperature));
  } else {
    logPrint("MPU not working...");
  }

  delay(1000);
}

#endif