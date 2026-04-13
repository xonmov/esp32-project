#ifndef APP_H
#define APP_H

void appSetup() {
  Serial.println("App started");
}

void appLoop() {
  Serial.println("Hello from my code!");
  delay(2000);
}

#endif