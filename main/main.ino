#include <WiFi.h>
#include "ota.h"

const char* ssid = "POCO";
const char* password = "123456789";

const int motorPin = D1; 



void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println(WiFi.localIP());

  setupOTA();   // 🔥 just one line


pinMode(motorPin, OUTPUT);


}

void loop() {
  handleOTA();  // 🔥 just one line
delay(1000);
 Serial.println("ok");
  
analogWrite(motorPin, 75); 
  delay(2000);

  // 2. Medium Speed
  analogWrite(motorPin, 150);
  delay(2000);

  // 3. Full Speed
  analogWrite(motorPin, 255);
  delay(2000);

  // 4. Stop
  analogWrite(motorPin, 0);
  delay(3000);


}
