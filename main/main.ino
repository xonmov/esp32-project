#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>

WebServer server(80);

// WiFi
const char* ssid = "ESP_TEST";
const char* password = "12345678";

// Pins
int P1 = 8;   // D9
int P2 = 9;   // D10
int P3 = 5;   // D4
int P4 = 6;   // D5

// ===== HTML =====
String page = R"====(
<!DOCTYPE html>
<html>
<body style="text-align:center;">
<h2>PIN TEST</h2>

<h3>D9</h3>
<button onclick="fetch('/p1on')">ON</button>
<button onclick="fetch('/p1off')">OFF</button>

<h3>D10</h3>
<button onclick="fetch('/p2on')">ON</button>
<button onclick="fetch('/p2off')">OFF</button>

<h3>D4</h3>
<button onclick="fetch('/p3on')">ON</button>
<button onclick="fetch('/p3off')">OFF</button>

<h3>D5</h3>
<button onclick="fetch('/p4on')">ON</button>
<button onclick="fetch('/p4off')">OFF</button>

<hr>

<h3>OTA Upload</h3>
<form method='POST' action='/update' enctype='multipart/form-data'>
<input type='file' name='update'>
<input type='submit' value='Upload'>
</form>

</body>
</html>
)====";

// ===== HANDLERS =====
void handleRoot(){ server.send(200,"text/html",page); }

// P1
void p1on(){ digitalWrite(P1,HIGH); server.send(200,"text/plain","P1 ON"); }
void p1off(){ digitalWrite(P1,LOW); server.send(200,"text/plain","P1 OFF"); }

// P2
void p2on(){ digitalWrite(P2,HIGH); server.send(200,"text/plain","P2 ON"); }
void