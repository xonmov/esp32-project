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

// ===== CONTROL =====
bool motorsOn = false;
int throttlePercent = 0;   // 0–100
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
int stageDelay = 400;

// ===== HTML =====
String page = R"====(
<!DOCTYPE html>
<html>
<body style="text-align:center;font-family:sans-serif;">
<h2>Drone Motor Control</h2>

<button onclick="toggle()">ON / OFF</button>

<br><br>

<input type="range" min="0" max="100" value="0" id="slider"
oninput="setThrottle(this.value)">
<p>Throttle: <span id="val">0</span>%</p>

<script>
function toggle(){
  fetch('/toggle');
}

function setThrottle(v){
  document.getElementById('val').innerText = v;
  fetch('/throttle?val=' + v);
}
</script>

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

void toggle(){
  motorsOn = !motorsOn;

  if (motorsOn) {
    startStage = 1;
    currentSpeed = 0;
  } else {
    startStage = 0;
    currentSpeed = 0;
    targetSpeed = 0;
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
  server.on("/toggle",toggle);
  server.on("/throttle",setThrottle);
  server.on("/update",HTTP_POST,handleUpdate,handleUpload);

  server.begin();
}

// ===== LOOP =====
void loop(){
  server.handleClient();

  // convert throttle % → PWM
  targetSpeed = map(throttlePercent, 0, 100, 0, maxLimit);

  // ===== SEQUENTIAL START =====
  if (motorsOn && startStage > 0 && currentSpeed == 0) {
    currentSpeed = minStart;
    lastStageTime = millis();
  }

  if (motorsOn && startStage > 0 && millis() - lastStageTime > stageDelay) {
    startStage++;
    lastStageTime = millis();
  }

  // ===== MOTOR OUTPUT =====
  int s1 = 0, s2 = 0, s3 = 0, s4 = 0;

  if (motorsOn) {
    if (startStage >= 1) s1 = currentSpeed;
    if (startStage >= 2) s2 = currentSpeed;
    if (startStage >= 3) s3 = currentSpeed;
    if (startStage >= 4) s4 = currentSpeed;
  }

  // ===== RAMP =====
  if (motorsOn && startStage >= 4) {
    if (currentSpeed < targetSpeed) currentSpeed += rampStep;
    else if (currentSpeed > targetSpeed) currentSpeed -= rampStep;
  }

  // ===== WRITE =====
  ledcWrite(M1, s1);
  ledcWrite(M2, s2);
  ledcWrite(M3, s3);
  ledcWrite(M4, s4);

  delay(rampDelay);
}