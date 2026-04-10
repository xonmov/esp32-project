#include <WiFi.h>
#include "ota.h"
#include <Wire.h>

const char* ssid = "POCO";
const char* password = "123456789";

const int motorPin = D1; 



void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println(WiFi.localIP());

  setupOTA();   // 🔥 just one line


// Start I2C on your specific pins
  Wire.begin(D3, D2); // SDA = D3, SCL = D2
  Serial.println("\nScanning for MPU6050...");


}

void loop() {
  handleOTA();  // 🔥 just one line
byte error, address;
  int nDevices = 0;

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("SUCCESS! Device found at address 0x");
      Serial.println(address, HEX); // Should show 0x68
      nDevices++;
    }
  }

  if (nDevices == 0) {
    Serial.println("No device found. Check wires.");
  }
  delay(3000);


}
