#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>

WebServer server(80);

// WiFi
const char* ssid = "ESP_MOTOR";
const char* password = "12345678";

// MOTOR PINS
int M1 = 8;  // D9
int M2 = 9;  // D10
int M3 = 5;  // D4
int M4 = 6;  // D5

// SPEED CONTROL
int targetSpeed = 0;
int currentSpeed = 0;

// ===== HTML =====
String page = R"====(
<!DOCTYPE html>
<html>
<body style="text-align:center;">
<h2>4 Motor Test</h2>

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

void low(){ targetSpeed = 80; server.send(200,"text/plain","LOW"); }
void mid(){ targetSpeed = 150; server.send(200,"text/plain","MID"); }
void high(){ targetSpeed = 255; server.send(200,"text/plain","HIGH"); }
void off(){ targetSpeed = 0; server.send(200,"text/plain","OFF"); }

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

  // PWM setup
  ledcAttach(M1, 20000, 8);
  ledcAttach(M2, 20000, 8);
  ledcAttach(M3, 20000, 8);
  ledcAttach(M4, 20000, 8);

  // WiFi
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid,password);

  Serial.println(WiFi.softAPIP());

  // Routes
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

  // ===== RAMP LOGIC =====
  if (currentSpeed < targetSpeed) {
    currentSpeed += 2;   // smooth increase
  }
  else if (currentSpeed > targetSpeed) {
    currentSpeed -= 2;   // smooth decrease
  }

  // ===== WRITE TO MOTORS =====
  ledcWrite(M1, currentSpeed);
  ledcWrite(M2, currentSpeed);
  ledcWrite(M3, currentSpeed);
  ledcWrite(M4, currentSpeed);

  delay(10); // controls ramp speed
}