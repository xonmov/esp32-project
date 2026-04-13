#ifndef APP_H
#define APP_H

// we use external log function from main
extern void logPrint(String msg);

// ===== YOUR CODE =====
void appSetup() {
  logPrint("App Started");
}

void appLoop() {
  logPrint("Hello from my app!");
  delay(2000);
}

#endif