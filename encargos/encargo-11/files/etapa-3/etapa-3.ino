// Ejercicio 3: LED controlado desde una página web (ESP32-S3)
#include <WiFi.h>
#include <WebServer.h>

// ---- Configuración ----
const char* NOMBRE_RED = "esp-clifford";  // Cambia por el de tu grupo
const char* CLAVE_RED  = "alcachofa11";     // Mínimo 8 caracteres
const int PIN_LED = 5;                     // En la S3 no existe el GPIO 23

WebServer servidor(80);
bool ledEncendido = false;

String crearPagina() {
  String estado = ledEncendido ? "ENCENDIDO" : "APAAAAAGADO";
  String html = "<!DOCTYPE html><html lang='es'><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>Control LED ESP32</title>";
  html += "<style>";
  html += "body{font-family:sans-serif;text-align:center;padding:40px;background:#FF00A6;color:#eee;}";
  html += "a{display:block;margin:16px auto;padding:20px;width:200px;border-radius:12px;";
  html += "font-size:22px;text-decoration:none;color:#fff;}";
  html += ".on{background:#2e7d32;} .off{background:#c62828;}";
  html += "</style></head><body>";
  html += "<h1>LED ESP32</h1>";
  html += "<p>Estado: <strong>" + estado + "</strong></p>";
  html += "<a class='on' href='/encender'>Encender</a>";
  html += "<a class='off' href='/apagar'>Apagar</a>";
  html += "</body></html>";
  return html;
}

void volverAlInicio() {
  servidor.sendHeader("Location", "/");
  servidor.send(303);
}

void paginaPrincipal() {
  servidor.send(200, "text/html", crearPagina());
}

void encenderLed() {
  ledEncendido = true;
  digitalWrite(PIN_LED, HIGH);
  volverAlInicio();
}

void apagarLed() {
  ledEncendido = false;
  digitalWrite(PIN_LED, LOW);
  volverAlInicio();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  WiFi.softAP(NOMBRE_RED, CLAVE_RED);
  Serial.print("Red creada: ");
  Serial.println(NOMBRE_RED);
  Serial.print("Abre en el navegador: http://");
  Serial.println(WiFi.softAPIP());

  servidor.on("/", paginaPrincipal);
  servidor.on("/encender", encenderLed);
  servidor.on("/apagar", apagarLed);
  servidor.begin();
}

void loop() {
  servidor.handleClient();
}