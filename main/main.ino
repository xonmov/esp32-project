#include <WiFi.h>
#include "ota.h"

const char* ssid = "POCO";
const char* password = "123456789";

const int motorPin = D0;



void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println(WiFi.localIP());

  setupOTA();   // 🔥 just one line

  ledcAttach(motorPin, 10000, 8); // pin, frequency, resolution
}

}

void loop() {
  handleOTA();  // 🔥 just one line
delay(1000);
 Serial.println("ok");
  
ledcWrite(motorPin, 200); // constant speed


}
