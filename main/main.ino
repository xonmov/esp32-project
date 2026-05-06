#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// ===== WIFI =====
const char* ssid = "ESP_DRONE";
const char* password = "12345678";
WebServer server(80);

// ===== MPU =====
Adafruit_MPU6050 mpu;

// ===== ANGLES =====
float pitch = 0, roll = 0;
float pitch_offset = 0, roll_offset = 0;

// ===== GYRO OFFSET =====
float gx_offset = 0, gy_offset = 0;

// ===== TIME =====
unsigned long prevTime = 0;

// ===== FILTER =====
float alpha = 0.98;

// ===== PID =====
float Kp = 3.0;
float Kd = 0.8;

float pitch_last_error = 0;
float roll_last_error = 0;

// ===== CONTROL =====
bool motorsOn = false;
int throttlePercent = 0;

int idleSpeed = 30;
int maxLimit = 120;

// ===== PID DELAY =====
bool stabilizeReady = false;
unsigned long armTime = 0;

// ===== SEQUENTIAL START =====
int startStage = 0;
unsigned long lastStageTime = 0;
int stageDelay = 600;

// ===== GYRO SAFETY =====
bool gyroReady = false;
unsigned long gyroCheckStart = 0;

// ===== MOTOR PINS =====
int M1 = D0;
int M2 = D1;
int M3 = D4;
int M4 = D5;

// ===== WEB PAGE =====
String page = R"====(
<!DOCTYPE html>
<html>
<body style="text-align:center;font-family:sans-serif;">
<h2>Drone Control</h2>

<button onclick="fetch('/toggle')">ARM / DISARM</button>

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
    lastStageTime = millis();
    armTime = millis();
    stabilizeReady = false;
  } else {
    startStage = 0;
  }

  server.send(200,"text/plain", motorsOn ? "ARMED" : "DISARMED");
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
void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  server.on("/", handleRoot);
  server.on("/toggle", toggle);
  server.on("/throttle", setThrottle);
  server.on("/update", HTTP_POST, handleUpdate, handleUpload);
  server.begin();

  Wire.begin(D3, D2);

  if (!mpu.begin(0x68, &Wire)) {
    Serial.println("MPU FAIL");
  }

  delay(2000);

  // ===== GYRO CAL =====
  for (int i = 0; i < 500; i++) {
    sensors_event_t a, g, t;
    mpu.getEvent(&a, &g, &t);
    gx_offset += g.gyro.x;
    gy_offset += g.gyro.y;
    delay(5);
  }

  gx_offset /= 500;
  gy_offset /= 500;

  // ===== INITIAL ANGLE =====
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);

  pitch = atan2(-a.acceleration.x, sqrt(a.acceleration.y*a.acceleration.y + a.acceleration.z*a.acceleration.z)) * 180 / PI;
  roll  = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;

  pitch_offset = pitch;
  roll_offset  = roll;

  // PWM
  ledcAttach(M1, 20000, 8);
  ledcAttach(M2, 20000, 8);
  ledcAttach(M3, 20000, 8);
  ledcAttach(M4, 20000, 8);

  prevTime = millis();
  gyroCheckStart = millis();
}

// ===== LOOP =====
void loop() {

  server.handleClient();

  // ===== MPU READ =====
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);

  unsigned long now = millis();
  float dt = (now - prevTime) / 1000.0;
  prevTime = now;
  if (dt <= 0 || dt > 0.1) dt = 0.01;

  float gx = g.gyro.x - gx_offset;
  float gy = g.gyro.y - gy_offset;

  float gx_deg = gx * 180 / PI;
  float gy_deg = gy * 180 / PI;

  float pitchAcc = atan2(-a.acceleration.x, sqrt(a.acceleration.y*a.acceleration.y + a.acceleration.z*a.acceleration.z)) * 180 / PI;
  float rollAcc  = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;

  pitch = alpha * (pitch + gx_deg * dt) + (1 - alpha) * pitchAcc;
  roll  = alpha * (roll  + gy_deg * dt) + (1 - alpha) * rollAcc;

  pitch -= pitch_offset;
  roll  -= roll_offset;

  // ===== DEBUG PRINT =====
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 200) {
    lastPrint = millis();
    Serial.print("Pitch:");
    Serial.print(pitch);
    Serial.print(" Roll:");
    Serial.println(roll);
  }

  // ===== GYRO VALIDATION =====
  if (!gyroReady) {
    if (abs(pitch) < 5 && abs(roll) < 5) {
      if (millis() - gyroCheckStart > 2000) {
        gyroReady = true;
        Serial.println("GYRO OK ✅ Motors Enabled");
      }
    } else {
      gyroCheckStart = millis();
    }

    ledcWrite(M1, 0);
    ledcWrite(M2, 0);
    ledcWrite(M3, 0);
    ledcWrite(M4, 0);
    return;
  }

  if (!motorsOn) {
    ledcWrite(M1, 0);
    ledcWrite(M2, 0);
    ledcWrite(M3, 0);
    ledcWrite(M4, 0);
    return;
  }

  // ===== SEQUENTIAL =====
  if (millis() - lastStageTime > stageDelay && startStage < 4) {
    startStage++;
    lastStageTime = millis();
  }

  // ===== THROTTLE =====
  int throttleSpeed = map(throttlePercent, 0, 100, 0, maxLimit);
  int baseThrottle = idleSpeed + throttleSpeed;

  // ===== PID DELAY =====
  if (!stabilizeReady && millis() - armTime > 2000) {
    stabilizeReady = true;
  }

  float pitch_output = 0;
  float roll_output  = 0;

  if (stabilizeReady) {
    float pitch_error = -pitch;
    float roll_error  = -roll;

    float pitch_derivative = (pitch_error - pitch_last_error) / dt;
    float roll_derivative  = (roll_error - roll_last_error) / dt;

    pitch_output = Kp * pitch_error + Kd * pitch_derivative;
    roll_output  = Kp * roll_error  + Kd * roll_derivative;

    pitch_output = constrain(pitch_output, -80, 80);
    roll_output  = constrain(roll_output, -80, 80);

    pitch_last_error = pitch_error;
    roll_last_error  = roll_error;
  }

  // ===== MIXING =====
  int m1 = baseThrottle + pitch_output - roll_output;
  int m2 = baseThrottle + pitch_output + roll_output;
  int m3 = baseThrottle - pitch_output + roll_output;
  int m4 = baseThrottle - pitch_output - roll_output;

  m1 = constrain(m1, 0, 255);
  m2 = constrain(m2, 0, 255);
  m3 = constrain(m3, 0, 255);
  m4 = constrain(m4, 0, 255);

  int s1 = (startStage >= 1) ? m1 : 0;
  int s2 = (startStage >= 2) ? m2 : 0;
  int s3 = (startStage >= 3) ? m3 : 0;
  int s4 = (startStage >= 4) ? m4 : 0;

  ledcWrite(M1, s1);
  ledcWrite(M2, s2);
  ledcWrite(M3, s3);
  ledcWrite(M4, s4);

  delay(5);
}
