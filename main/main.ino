#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>

WebServer server(80);

// ===== WiFi =====
const char* ssid = "ESP_MOTOR";
const char* password = "12345678";

// ===== MOTOR PINS =====
int M1 = 8;  // D9
int M2 = 9;  // D10
int M3 = 5;  // D4
int M4 = 6;  // D5

// ===== SPEED CONTROL =====
int targetSpeed = 0;
int currentSpeed = 0;

// ===== SAFE SETTINGS =====
int minStart = 40;
int maxLimit = 180;
int rampStep = 1;
int rampDelay = 20;

// ===== SEQUENTIAL START =====
int startStage = 0;
unsigned long lastStageTime = 0;
int stageDelay = 400; // time between motors

// ===== HTML =====
String page = R"====(
<!DOCTYPE html>
<html>
<body style="text-align:center;">
<h2>4 Motor Sequential Start</h2>

<button onclick="fetch('/low')">LOW</button>
<button onclick="fetch('/mid')">MID</button>
<button onclick="fetch('/high')">HIGH</button>
<button onclick="fetch('/off')">OFF</button>

<hr>

<form method='POST' action='/update' enctype='multipart/form-data'>
<input type='file' name='update'>
<input type='submit' value='Upload'>
</form>

</body>
</html>
)====";

// ===== WEB =====
void handleRoot(){ server.send(200,"text/html",page); }

void low(){ targetSpeed = 70; startStage = 1; server.send(200,"text/plain","LOW"); }
void mid(){ targetSpeed = 120; startStage = 1; server.send(200,"text/plain","MID"); }
void high(){ targetSpeed = 160; startStage = 1; server.send(200,"text/plain","HIGH"); }

void off(){
  targetSpeed = 0;
  startStage = 0;
  currentSpeed = 0;
  server.send(200,"text/plain","OFF");
}

// ===== OTA =====
void handleUpdate(){
  server.send(200,"text/plain",Update.hasError()?"FAIL":"OK");
  delay(1000);
  ESP.restart();
}

void handleUpload(){
  HTTPUpload& upload = server.upload();

  if(upload.status==UPLOAD_FILE_START){
    Update.begin();
  }
  else if(upload.status==UPLOAD_FILE_WRITE){
    Update.write(upload.buf, upload.currentSize);
  }
  else if(upload.status==UPLOAD_FILE_END){
    Update.end(true);
  }
}

// ===== SETUP =====
void setup(){
  Serial.begin(115200);

  ledcAttach(M1, 20000, 8);
  ledcAttach(M2, 20000, 8);
  ledcAttach(M3, 20000, 8);
  ledcAttach(M4, 20000, 8);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid,password);

  Serial.println(WiFi.softAPIP());

  server.on("/",handleRoot);
  server.on("/low",low);
  server.on("/mid",mid);
  server.on("/high",high);
  server.on("/off",off);
  server.on("/update",HTTP_POST,handleUpdate,handleUpload);

  server.begin();
}

// ===== LOOP =====
void loop(){
  server.handleClient();

  int safeTarget = constrain(targetSpeed, 0, maxLimit);

  // ===== SEQUENTIAL START =====
  if (startStage > 0 && currentSpeed == 0) {
    currentSpeed = minStart;
    lastStageTime = millis();
  }

  // move to next motor step-by-step
  if (startStage > 0 && millis() - lastStageTime > stageDelay) {
    startStage++;
    lastStageTime = millis();
  }

  // ===== MOTOR OUTPUT =====
  int s1 = 0, s2 = 0, s3 = 0, s4 = 0;

  if (startStage >= 1) s1 = currentSpeed;
  if (startStage >= 2) s2 = currentSpeed;
  if (startStage >= 3) s3 = currentSpeed;
  if (startStage >= 4) s4 = currentSpeed;

  // ===== RAMP AFTER ALL STARTED =====
  if (startStage >= 4) {
    if (currentSpeed < safeTarget) currentSpeed += rampStep;
    else if (currentSpeed > safeTarget) currentSpeed -= rampStep;
  }

  // ===== WRITE =====
  ledcWrite(M1, s1);
  ledcWrite(M2, s2);
  ledcWrite(M3, s3);
  ledcWrite(M4, s4);

  delay(rampDelay);
}