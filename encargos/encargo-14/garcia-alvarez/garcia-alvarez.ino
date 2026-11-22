#include <Adafruit_NeoPixel.h>
#include "DFRobotDFPlayerMini.h"   // [DF]

#define LED_PIN      13
#define NUM_LEDS     200
#define TRIG_PIN     32
#define ECHO_PIN     33

// [DF] UART2 del ESP32: RX=16 (al TX del DFPlayer), TX=17 (al RX del DFPlayer, con resistencia de 1k)
#define DF_RX        16
#define DF_TX        17
#define DF_CARPETA   1        // carpeta "01"
#define DF_PISTAS    2        // 001.mp3 y 002.mp3
#define DF_VOLUMEN   20       // 0-30
#define DIST_PRESENCIA 160
#define COOLDOWN_MS  5000     // tiempo mínimo entre audios

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
DFRobotDFPlayerMini myDFPlayer;   // [DF]

float distanciaSuave = 180.0;
float animTiempo = 0;

bool dfOk = false;                // [DF]
bool habiaPresencia = false;      // [DF]
unsigned long ultimoAudio = 0;    // [DF]

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  strip.begin();
  strip.setBrightness(85);

  strip.fill(strip.Color(255, 0, 0)); strip.show(); delay(300);
  strip.fill(strip.Color(0, 255, 0)); strip.show(); delay(300);
  strip.fill(strip.Color(0, 0, 255)); strip.show(); delay(300);
  strip.clear(); strip.show();

  // [DF] Inicialización del DFPlayer
  Serial2.begin(9600, SERIAL_8N1, DF_RX, DF_TX);
  if (myDFPlayer.begin(Serial2, true, true)) {
    dfOk = true;
    myDFPlayer.volume(DF_VOLUMEN);
    Serial.println("DFPlayer OK");
  } else {
    Serial.println("Error DFPlayer: revisar conexion y SD (las luces siguen funcionando)");
  }

  Serial.println("=================================");
  Serial.println("SISTEMA CON DEGRADÉ Y ESPUMA OK");
  Serial.println("=================================");
}

long obtenerDistanciaFiltrada() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duracion = pulseIn(ECHO_PIN, HIGH, 25000);
  long lecturaRaw = duracion * 0.034 / 2;

  if (lecturaRaw > 2 && lecturaRaw < 300) {
    distanciaSuave = (distanciaSuave * 0.6) + (lecturaRaw * 0.4);
  }

  return (long)distanciaSuave;
}

uint32_t mezclarColores(uint32_t color1, uint32_t color2, float factor) {
  if (factor <= 0.0) return color1;
  if (factor >= 1.0) return color2;

  uint8_t r1 = (uint8_t)(color1 >> 16);
  uint8_t g1 = (uint8_t)(color1 >> 8);
  uint8_t b1 = (uint8_t)color1;

  uint8_t r2 = (uint8_t)(color2 >> 16);
  uint8_t g2 = (uint8_t)(color2 >> 8);
  uint8_t b2 = (uint8_t)color2;

  uint8_t r = r1 + (r2 - r1) * factor;
  uint8_t g = g1 + (g2 - g1) * factor;
  uint8_t b = b1 + (b2 - b1) * factor;

  return strip.Color(r, g, b);
}

uint32_t getColorArena(int i, float t) {
  float respiro = (sin(t * 1.2) + 1.0) / 2.0;
  int brillo = 30 + (int)(respiro * 180);

  int r = constrain(brillo, 0, 255);
  int g = constrain(brillo * 0.5, 0, 255);
  int b = constrain(brillo * 0.08, 0, 255);

  return strip.Color(r, g, b);
}

uint32_t getColorMar(int i, float t) {
  float onda = sin((t * 2.5) + (i * 0.40));
  int brillo = map(onda * 100, -100, 100, 0, 255);

  if (brillo > 210)      return strip.Color(220, 240, 255);
  else if (brillo > 130) return strip.Color(0, brillo, brillo);
  else if (brillo > 60)  return strip.Color(0, brillo / 2, brillo);
  else                   return strip.Color(0, 10, brillo);
}

// [DF] Reproduce una pista al azar solo cuando alguien recién llega
void manejarAudio(long distancia) {
  bool hayPresencia = distancia <= DIST_PRESENCIA;

  if (hayPresencia && !habiaPresencia && dfOk &&
      millis() - ultimoAudio > COOLDOWN_MS) {
    int pista = random(1, DF_PISTAS + 1);   // 1 o 2
    myDFPlayer.playFolder(DF_CARPETA, pista);
    ultimoAudio = millis();
    Serial.print("Reproduciendo pista "); Serial.println(pista);
  }

  habiaPresencia = hayPresencia;
}

void loop() {
  long distancia = obtenerDistanciaFiltrada();

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  manejarAudio(distancia);   // [DF]

  animTiempo += 0.08;

  if (distancia > DIST_PRESENCIA) {
    strip.clear();
    strip.show();
  } else {
    float factorAzul = 0.0;
    if (distancia <= 60)       factorAzul = 1.0;
    else if (distancia >= 140) factorAzul = 0.0;
    else                       factorAzul = (140.0 - distancia) / (140.0 - 60.0);

    for (int i = 0; i < NUM_LEDS; i++) {
      uint32_t cArena = getColorArena(i, animTiempo);
      uint32_t cMar   = getColorMar(i, animTiempo);
      strip.setPixelColor(i, mezclarColores(cArena, cMar, factorAzul));
    }
    strip.show();
  }

  delay(25);
}