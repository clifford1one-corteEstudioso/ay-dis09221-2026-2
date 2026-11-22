#include <Adafruit_NeoPixel.h>

// ==========================================
// FLAGS DE DEBUG (true = Activo | false = Inactivo)
// ==========================================
const bool ENABLE_SENSOR_1 = true; // Activar/desactivar Sensor 1 (Rojo)
const bool ENABLE_SENSOR_2 = true; // Activar/desactivar Sensor 2 (Azul)
const bool ENABLE_SENSOR_3 = true; // Activar/desactivar Sensor 3 (Verde)
const bool ENABLE_SENSOR_4 = true; // Activar/desactivar Sensor 4 (Amarillo)

// ==========================================
// CONFIGURACIÓN DE PINES Y HARDWARE
// ==========================================
const int TRIG_1 = 22, ECHO_1 = 23;
const int TRIG_2 = 19, ECHO_2 = 21;
const int TRIG_3 = 27, ECHO_3 = 34;
const int TRIG_4 = 32, ECHO_4 = 35;

const int PIN_LEDS = 13;
const int NUM_LEDS = 28;
const float UMBRAL = 50.0; // Distancia máxima de detección en cm

Adafruit_NeoPixel leds(NUM_LEDS, PIN_LEDS, NEO_GRB + NEO_KHZ800);

// ==========================================
// FUNCIONES AUXILIARES
// ==========================================

// Mide distancia en cm con el sensor indicado
float medirDistancia(int pinTrig, int pinEcho) {
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);

  long duracion = pulseIn(pinEcho, HIGH, 30000);
  return (duracion == 0) ? 999.0 : (duracion * 0.0343) / 2.0;
}

// Enciende un rango de LEDs con un color RGB específico
void pintarRango(int inicio, int fin, uint8_t r, uint8_t g, uint8_t b) {
  for (int i = inicio; i <= fin; i++) {
    leds.setPixelColor(i, leds.Color(r, g, b));
  }
}

// ==========================================
// CONFIGURACIÓN INICIAL
// ==========================================
void setup() {
  Serial.begin(115200);

  // Configurar pines de los sensores
  pinMode(TRIG_1, OUTPUT); pinMode(ECHO_1, INPUT);
  pinMode(TRIG_2, OUTPUT); pinMode(ECHO_2, INPUT);
  pinMode(TRIG_3, OUTPUT); pinMode(ECHO_3, INPUT);
  pinMode(TRIG_4, OUTPUT); pinMode(ECHO_4, INPUT);

  // Inicializar LEDs
  leds.begin();
  leds.clear();
  leds.show();
}

// ==========================================
// BUCLE PRINCIPAL
// ==========================================
void loop() {
  leds.clear(); // Limpia el buffer de colores al inicio de cada ciclo

  // --- SENSOR 1 (LEDs 0-6 | Rojo) ---
  if (ENABLE_SENSOR_1) {
    float d1 = medirDistancia(TRIG_1, ECHO_1);
    Serial.print("S1: "); Serial.print(d1); Serial.print(" cm | ");
    if (d1 > 0 && d1 <= UMBRAL) pintarRango(0, 6, 255, 0, 0);
  } else {
    Serial.print("S1: OFF | ");
  }

  // --- SENSOR 2 (LEDs 7-13 | Azul) ---
  if (ENABLE_SENSOR_2) {
    float d2 = medirDistancia(TRIG_2, ECHO_2);
    Serial.print("S2: "); Serial.print(d2); Serial.print(" cm | ");
    if (d2 > 0 && d2 <= UMBRAL) pintarRango(7, 13, 0, 0, 255);
  } else {
    Serial.print("S2: OFF | ");
  }

  // --- SENSOR 3 (LEDs 14-20 | Verde) ---
  if (ENABLE_SENSOR_3) {
    float d3 = medirDistancia(TRIG_3, ECHO_3);
    Serial.print("S3: "); Serial.print(d3); Serial.print(" cm | ");
    if (d3 > 0 && d3 <= UMBRAL) pintarRango(14, 20, 0, 255, 0);
  } else {
    Serial.print("S3: OFF | ");
  }

  // --- SENSOR 4 (LEDs 21-27 | Amarillo) ---
  if (ENABLE_SENSOR_4) {
    float d4 = medirDistancia(TRIG_4, ECHO_4);
    Serial.print("S4: "); Serial.print(d4); Serial.print(" cm");
    if (d4 > 0 && d4 <= UMBRAL) pintarRango(21, 27, 255, 255, 0);
  } else {
    Serial.print("S4: OFF");
  }

  Serial.println(); // Salto de línea en el monitor serie
  leds.show();     // Envía las órdenes a la tira de LEDs

  delay(50); // Pequeña pausa de estabilidad
}