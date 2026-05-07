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
float pitch = 0;
float roll = 0;

float pitch_offset = 0;
float roll_offset = 0;

// ===== GYRO OFFSET =====
float gx_offset = 0;
float gy_offset = 0;

// ===== TIME =====
unsigned long prevTime = 0;

// ===== FILTER =====
float alpha = 0.98;

// ===== PID =====
float Kp = 0.7;
float Kd = 0.15;

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
int M1 = 8;
int M2 = 9;
int M3 = 5;
int M4 = 6;

// ===== MOTOR TRIM =====
int trimM1 = 0;
int trimM2 = 0;
int trimM3 = 2;
int trimM4 = 2;

// ===== WEB PAGE =====
String page = R"====(
<!DOCTYPE html>
<html>
<body style="text-align:center;font-family:sans-serif;">

<h2>Drone Control</h2>

<button onclick="fetch('/toggle')">
ARM / DISARM
</button>

<br><br>

<input type="range"
min="0"
max="100"
value="0"
oninput="setThrottle(this.value)">

<p>
Throttle:
<span id="throttleVal">0</span>%
</p>

<hr>

<h3>Motor Trim</h3>

<p>M1 Trim</p>
<input type="range" min="-20" max="20" value="0"
oninput="setTrim('m1',this.value)">
<span id="m1v">0</span>

<p>M2 Trim</p>
<input type="range" min="-20" max="20" value="0"
oninput="setTrim('m2',this.value)">
<span id="m2v">0</span>

<p>M3 Trim</p>
<input type="range" min="-20" max="20" value="2"
oninput="setTrim('m3',this.value)">
<span id="m3v">2</span>

<p>M4 Trim</p>
<input type="range" min="-20" max="20" value="2"
oninput="setTrim('m4',this.value)">
<span id="m4v">2</span>

<script>

function setThrottle(v){
  document.getElementById('throttleVal').innerText = v;
  fetch('/throttle?val=' + v);
}

function setTrim(motor,val){

  document.getElementById(motor + 'v').innerText = val;

  fetch('/trim?motor=' + motor + '&val=' + val);
}

</script>

<hr>

<h3>OTA Update</h3>

<form method='POST'
action='/update'
enctype='multipart/form-data'>

<input type='file' name='update'>
<input type='submit' value='Upload'>

</form>

</body>
</html>
)====";

// ===== WEB =====
void handleRoot() {
  server.send(200, "text/html", page);
}

void toggle() {

  motorsOn = !motorsOn;

  if (motorsOn) {

    startStage = 1;

    lastStageTime = millis();

    armTime = millis();

    stabilizeReady = false;

    Serial.println("ARMED");

  } else {

    startStage = 0;

    ledcWrite(M1, 0);
    ledcWrite(M2, 0);
    ledcWrite(M3, 0);
    ledcWrite(M4, 0);

    Serial.println("DISARMED");
  }

  server.send(
    200,
    "text/plain",
    motorsOn ? "ARMED" : "DISARMED"
  );
}

void setThrottle() {

  if (server.hasArg("val")) {

    throttlePercent =
      server.arg("val").toInt();
  }

  server.send(200, "text/plain", "OK");
}

// ===== OTA =====
void handleUpdate() {

  server.send(
    200,
    "text/plain",
    Update.hasError() ? "FAIL" : "SUCCESS"
  );

  delay(1000);

  ESP.restart();
}

void handleUpload() {

  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {

    Update.begin(UPDATE_SIZE_UNKNOWN);

  } else if (upload.status ==
             UPLOAD_FILE_WRITE) {

    Update.write(
      upload.buf,
      upload.currentSize
    );

  } else if (upload.status ==
             UPLOAD_FILE_END) {

    Update.end(true);
  }
}

void setTrim() {

  String motor = server.arg("motor");
  int val = server.arg("val").toInt();

  if (motor == "m1") trimM1 = val;
  if (motor == "m2") trimM2 = val;
  if (motor == "m3") trimM3 = val;
  if (motor == "m4") trimM4 = val;

  Serial.print("TRIMS => ");
  Serial.print(trimM1);
  Serial.print(", ");
  Serial.print(trimM2);
  Serial.print(", ");
  Serial.print(trimM3);
  Serial.print(", ");
  Serial.println(trimM4);

  server.send(200, "text/plain", "OK");
}

// ===== SETUP =====
void setup() {

  Serial.begin(115200);

  // ===== WIFI =====
  WiFi.mode(WIFI_AP);

  WiFi.softAP(ssid, password);

  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);

  server.on("/toggle", toggle);

  server.on("/throttle", setThrottle);

  server.on("/trim", setTrim);

  server.on(
    "/update",
    HTTP_POST,
    handleUpdate,
    handleUpload
  );

  server.begin();

  // ===== I2C =====
  Wire.begin(D3, D2);

  // ===== MPU =====
  if (!mpu.begin(0x68, &Wire)) {

    Serial.println("MPU FAIL");

    
  }

  Serial.println("MPU OK");

  delay(2000);

  // ===== GYRO CAL =====
  Serial.println("Keep drone flat");

  for (int i = 0; i < 500; i++) {

    sensors_event_t a, g, t;

    mpu.getEvent(&a, &g, &t);

    gx_offset += g.gyro.x;
    gy_offset += g.gyro.y;

    delay(5);
  }

  gx_offset /= 500.0;
  gy_offset /= 500.0;

  Serial.println("Gyro calibrated");

  // ===== INITIAL ANGLE =====
  sensors_event_t a, g, t;

  mpu.getEvent(&a, &g, &t);

  pitch = atan2(
            a.acceleration.y,
            sqrt(
              a.acceleration.x *
              a.acceleration.x +

              a.acceleration.z *
              a.acceleration.z
            )
          ) * 180 / PI;

  roll = atan2(
           -a.acceleration.x,
           a.acceleration.z
         ) * 180 / PI;

  pitch = 0;
  roll = 0;

  // ===== PWM =====
  ledcAttach(M1, 20000, 8);
  ledcAttach(M2, 20000, 8);
  ledcAttach(M3, 20000, 8);
  ledcAttach(M4, 20000, 8);

  prevTime = millis();

  gyroCheckStart = millis();

  Serial.println("Setup complete");
}

// ===== LOOP =====
void loop() {

  server.handleClient();

  // ===== MPU READ =====
  sensors_event_t a, g, t;

  mpu.getEvent(&a, &g, &t);

  // ===== TIME =====
  unsigned long now = millis();

  float dt =
    (now - prevTime) / 1000.0;

  prevTime = now;

  if (dt <= 0 || dt > 0.1) {
    dt = 0.01;
  }

  // ===== GYRO =====
  float gx =
    g.gyro.x - gx_offset;

  float gy =
    g.gyro.y - gy_offset;

  float gx_deg =
    gx * 180 / PI;

  float gy_deg =
    gy * 180 / PI;

  // ===== ACCEL =====
  float ax =
    a.acceleration.x;

  float ay =
    a.acceleration.y;

  float az =
    a.acceleration.z;

  // ===== ANGLES =====
  float pitchAcc = atan2(
                     ay,
                     sqrt(
                       ax * ax +
                       az * az
                     )
                   ) * 180 / PI;

  float rollAcc = atan2(
                    -ax,
                    az
                  ) * 180 / PI;

  // ===== FILTER =====
  pitch =
    alpha *
    (pitch + gx_deg * dt)
    +
    (1 - alpha) * pitchAcc;

  roll =
    alpha *
    (roll + gy_deg * dt)
    +
    (1 - alpha) * rollAcc;

  // ===== OFFSET =====
  

  // ===== DEBUG =====
  static unsigned long lastPrint = 0;

  if (millis() - lastPrint > 200) {

    lastPrint = millis();

    Serial.print("Pitch:");
    Serial.print(pitch);

    Serial.print(" Roll:");
    Serial.println(roll);
  }

  // ===== GYRO CHECK =====
  if (!gyroReady) {

    if (abs(pitch) < 5 &&
        abs(roll) < 5) {

      if (millis() -
          gyroCheckStart > 2000) {

        gyroReady = true;

        Serial.println("GYRO OK");
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

  // ===== DISARM =====
  if (!motorsOn) {

    ledcWrite(M1, 0);
    ledcWrite(M2, 0);
    ledcWrite(M3, 0);
    ledcWrite(M4, 0);

    return;
  }

  // ===== SEQUENTIAL START =====
  if (millis() -
      lastStageTime > stageDelay &&
      startStage < 4) {

    startStage++;

    lastStageTime = millis();
  }

  // ===== THROTTLE =====
  int throttleSpeed =
    map(
      throttlePercent,
      0,
      100,
      0,
      maxLimit
    );

  int baseThrottle =
    idleSpeed +
    throttleSpeed;

  // ===== PID DELAY =====
  if (!stabilizeReady &&
      millis() - armTime > 2000) {

    stabilizeReady = true;

    Serial.println("PID ENABLED");
  }

  float pitch_output = 0;
  float roll_output = 0;

  // ===== PID =====
  if (stabilizeReady) {

    float pitch_error =
      -pitch;

    float roll_error =
      -roll;

    float pitch_derivative =
      (pitch_error -
       pitch_last_error) / dt;

    float roll_derivative =
      (roll_error -
       roll_last_error) / dt;

    pitch_output =
      Kp * pitch_error +
      Kd * pitch_derivative;

    roll_output =
      Kp * roll_error +
      Kd * roll_derivative;

    pitch_output = constrain(pitch_output, -25, 25);
roll_output  = constrain(roll_output, -25, 25);

    pitch_last_error =
      pitch_error;

    roll_last_error =
      roll_error;
  }

  // ===== MIXING =====
  
int m1 =
  baseThrottle
  - pitch_output
  - roll_output;

int m2 =
  baseThrottle
  + pitch_output
  - roll_output;

int m3 =
  baseThrottle
  + pitch_output
  + roll_output;

int m4 =
  baseThrottle
  - pitch_output
  + roll_output;

m1 += trimM1;
m2 += trimM2;
m3 += trimM3;
m4 += trimM4;

Serial.print("M1:");
Serial.print(m1);

Serial.print(" M2:");
Serial.print(m2);

Serial.print(" M3:");
Serial.print(m3);

Serial.print(" M4:");
Serial.println(m4);

  // ===== LIMIT =====
  m1 = constrain(m1, idleSpeed, 255);
m2 = constrain(m2, idleSpeed, 255);
m3 = constrain(m3, idleSpeed, 255);
m4 = constrain(m4, idleSpeed, 255);

  // ===== SEQUENTIAL =====
  int s1 =
    (startStage >= 1)
    ? m1 : 0;

  int s2 =
    (startStage >= 2)
    ? m2 : 0;

  int s3 =
    (startStage >= 3)
    ? m3 : 0;

  int s4 =
    (startStage >= 4)
    ? m4 : 0;

  // ===== OUTPUT =====
  ledcWrite(M1, s1);
  ledcWrite(M2, s2);
  ledcWrite(M3, s3);
  ledcWrite(M4, s4);

  delay(5);
}