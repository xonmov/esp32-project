#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>

// ===== WIFI =====
const char* ssid = "ESP_DRONE";
const char* password = "12345678";

WebServer server(80);

// ===== MOTOR PINS =====
int M1 = 8;
int M2 = 9;
int M3 = 5;
int M4 = 6;

// ===== MOTOR SPEED =====
int speedValue = 30;

// ===== HTML =====
String page = R"====(
<!DOCTYPE html>
<html>

<body style="text-align:center;font-family:sans-serif;">

<h2>Motor Placement Test</h2>

<button onclick="fetch('/m1')">M1</button>
<button onclick="fetch('/m2')">M2</button>
<button onclick="fetch('/m3')">M3</button>
<button onclick="fetch('/m4')">M4</button>

<br><br>

<button onclick="fetch('/all')">ALL</button>
<button onclick="fetch('/stop')">STOP</button>

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

// ===== STOP ALL =====
void stopAll() {

  ledcWrite(M1, 0);
  ledcWrite(M2, 0);
  ledcWrite(M3, 0);
  ledcWrite(M4, 0);
}

// ===== ROUTES =====
void handleRoot() {
  server.send(200, "text/html", page);
}

void runM1() {

  stopAll();

  ledcWrite(M1, speedValue);

  Serial.println("M1 RUNNING");

  server.send(200, "text/plain", "M1");
}

void runM2() {

  stopAll();

  ledcWrite(M2, speedValue);

  Serial.println("M2 RUNNING");

  server.send(200, "text/plain", "M2");
}

void runM3() {

  stopAll();

  ledcWrite(M3, speedValue);

  Serial.println("M3 RUNNING");

  server.send(200, "text/plain", "M3");
}

void runM4() {

  stopAll();

  ledcWrite(M4, speedValue);

  Serial.println("M4 RUNNING");

  server.send(200, "text/plain", "M4");
}

void runAll() {

  ledcWrite(M1, speedValue);
  ledcWrite(M2, speedValue);
  ledcWrite(M3, speedValue);
  ledcWrite(M4, speedValue);

  Serial.println("ALL RUNNING");

  server.send(200, "text/plain", "ALL");
}

void stopMotors() {

  stopAll();

  Serial.println("STOP");

  server.send(200, "text/plain", "STOP");
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

// ===== SETUP =====
void setup() {

  Serial.begin(115200);

  // ===== WIFI =====
  WiFi.mode(WIFI_AP);

  WiFi.softAP(ssid, password);

  Serial.println(WiFi.softAPIP());

  // ===== PWM =====
  ledcAttach(M1, 20000, 8);
  ledcAttach(M2, 20000, 8);
  ledcAttach(M3, 20000, 8);
  ledcAttach(M4, 20000, 8);

  stopAll();

  // ===== WEB =====
  server.on("/", handleRoot);

  server.on("/m1", runM1);
  server.on("/m2", runM2);
  server.on("/m3", runM3);
  server.on("/m4", runM4);

  server.on("/all", runAll);

  server.on("/stop", stopMotors);

  server.on(
    "/update",
    HTTP_POST,
    handleUpdate,
    handleUpload
  );

  server.begin();

  Serial.println("READY");
}

// ===== LOOP =====
void loop() {

  server.handleClient();
}