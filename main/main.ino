#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include "esp_system.h"

WebServer server(80);

// ===== WiFi =====
const char* ssid = "ESP_MOTOR";
const char* password = "12345678";

// ===== MOTOR PINS =====
int M1 = 8;
int M2 = 9;
int M3 = 5;
int M4 = 6;

// ===== CONTROL =====
bool motorsOn = false;
int throttlePercent = 0;

int currentSpeed = 0;
int targetSpeed = 0;

// ===== SETTINGS =====
int idleSpeed = 20;     // adjust if motors don't spin
int maxLimit = 180;
int rampStep = 1;
int rampDelay = 40;

// ===== SEQUENTIAL =====
int startStage = 0;
unsigned long lastStageTime = 0;
int stageDelay = 800;

// ===== HTML =====
String page = R"====(
<!DOCTYPE html>
<html>
<body style="text-align:center;font-family:sans-serif;">
<h2>Drone Motor Control</h2>

<button onclick="fetch('/toggle')">ON / OFF</button>

<br><br>

<input type="range" min="0" max="100" value="0"
oninput="setThrottle(this.value)">
<p>Throttle: <span id="val">0</span>%</p>

<script>
function setThrottle(v){
  document.getElementById('val').innerText = v;
  fetch('/throttle?val=' + v);
}
</script>

<hr>

<h3>OTA Update</h3>
<form method='POST' action='/update' enctype='multipart/form-data'>
<input type='file' name='update'>
<input type='submit' value='Upload'>
</form>

</body>
</html>
)====";

// ===== WEB =====
void handleRoot(){ server.send(200,"text/html",page); }

void toggle(){
  motorsOn = !motorsOn;

  if (motorsOn) {
    startStage = 1;
    currentSpeed = idleSpeed;
    lastStageTime = millis();
  } else {
    startStage = 0;
    currentSpeed = 0;
  }

  server.send(200,"text/plain", motorsOn ? "ON" : "OFF");
}

void setThrottle(){
  if (server.hasArg("val")) {
    throttlePercent = server.arg("val").toInt();
  }
  server.send(200,"text/plain","OK");
}

// ===== OTA =====
void handleUpdate(){
  server.send(200,"text/plain",Update.hasError()?"FAIL":"SUCCESS");
  delay(1000);
  ESP.restart();
}

void handleUpload(){
  HTTPUpload& upload = server.upload();

  if(upload.status == UPLOAD_FILE_START){
    Update.begin(UPDATE_SIZE_UNKNOWN);
  }
  else if(upload.status == UPLOAD_FILE_WRITE){
    Update.write(upload.buf, upload.currentSize);
  }
  else if(upload.status == UPLOAD_FILE_END){
    Update.end(true);
  }
}

// ===== SETUP =====
void setup(){
  Serial.begin(115200);
delay(500);

esp_reset_reason_t reason = esp_reset_reason();

Serial.print("Reset reason: ");

switch(reason) {
  case ESP_RST_POWERON: Serial.println("Power ON"); break;
  case ESP_RST_BROWNOUT: Serial.println("Brownout (LOW VOLTAGE)"); break;
  case ESP_RST_SW: Serial.println("Software reset"); break;
  case ESP_RST_PANIC: Serial.println("Crash / Panic"); break;
  case ESP_RST_INT_WDT: Serial.println("Watchdog reset"); break;
  case ESP_RST_TASK_WDT: Serial.println("Task Watchdog"); break;
  default: Serial.println("Unknown"); break;
}



  ledcAttach(M1, 20000, 8);
  ledcAttach(M2, 20000, 8);
  ledcAttach(M3, 20000, 8);
  ledcAttach(M4, 20000, 8);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid,password);

  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/",handleRoot);
  server.on("/toggle",toggle);
  server.on("/throttle",setThrottle);
  server.on("/update", HTTP_POST, handleUpdate, handleUpload);

  server.begin();
}

// ===== LOOP =====
void loop(){
  server.handleClient();

  // ===== TARGET SPEED =====
  int throttleSpeed = map(throttlePercent, 0, 100, 0, maxLimit);
  targetSpeed = idleSpeed + throttleSpeed;

  if (!motorsOn) {
    ledcWrite(M1, 0);
    ledcWrite(M2, 0);
    ledcWrite(M3, 0);
    ledcWrite(M4, 0);
    return;
  }

  // ===== SEQUENTIAL START =====
  if (millis() - lastStageTime > stageDelay && startStage < 4) {
    startStage++;
    lastStageTime = millis();
  }

  // ===== RAMP =====
  if (currentSpeed < targetSpeed) currentSpeed += rampStep;
  else if (currentSpeed > targetSpeed) currentSpeed -= rampStep;

  // ===== OUTPUT =====
  int s1 = (startStage >= 1) ? currentSpeed : 0;
  int s2 = (startStage >= 2) ? currentSpeed : 0;
  int s3 = (startStage >= 3) ? currentSpeed : 0;
  int s4 = (startStage >= 4) ? currentSpeed : 0;

  ledcWrite(M1, s1);
  ledcWrite(M2, s2);
  ledcWrite(M3, s3);
  ledcWrite(M4, s4);

  delay(rampDelay);
}