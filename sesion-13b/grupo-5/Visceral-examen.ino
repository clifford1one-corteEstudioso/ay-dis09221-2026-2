/*
  Proyecto ESP32: brillo de tira WS2812B con HC-SR04 + 3 LDR como teclas MIDI
  Cada LDR dispara una nota independiente.
  MIDI viaja por Serial (115200) hacia HairlessMIDI -> loopMIDI -> Dexed

  Libreria necesaria: Adafruit NeoPixel
*/

#include <Adafruit_NeoPixel.h>

// ---------------- CONFIGURACION ----------------
#define DEBUG false          // true = imprime texto (usar con Monitor Serie, NO con Hairless)

// Pines
#define PIN_LED   5
#define NUM_LEDS  29
#define PIN_TRIG  25
#define PIN_ECHO  26

// Sensor de distancia -> brillo
const float   DIST_MIN   = 5.0;    // cm: brillo maximo
const float   DIST_MAX   = 60.0;   // cm: brillo minimo
const uint8_t BRIGHT_MAX = 150;    // limite para cuidar la fuente (0-255)

// MIDI
const uint8_t MIDI_CH = 0;         // canal 1

// Comportamiento de las LDR
// true  = la nota suena al TAPAR la LDR (sombra)
// false = la nota suena cuando le llega LUZ (ej. con linterna)
const bool TRIGGER_ON_SHADOW = true;

// Umbrales con histeresis (0.0 - 1.0) para evitar parpadeos de nota
const float THRESH_ON  = 0.35;
const float THRESH_OFF = 0.50;

// ---------------- LDR / NOTAS ----------------
struct Voice {
  uint8_t pin;
  uint8_t note;     // nota MIDI de esta LDR
  float smooth;
  int minV;
  int maxV;
  bool active;
};

// Notas por defecto: C4 (60), E4 (64), G4 (67) = acorde de Do mayor
Voice voices[3] = {
  {34, 60, 0, 4095, 0, false},
  {35, 64, 0, 4095, 0, false},
  {32, 67, 0, 4095, 0, false}
};

// ---------------- VARIABLES ----------------
Adafruit_NeoPixel strip(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);

float brightSmooth = 0;
unsigned long lastDist = 0;
unsigned long lastMidi = 0;

// ---------------- MIDI ----------------
void sendNoteOn(uint8_t note, uint8_t vel) {
  Serial.write(0x90 | MIDI_CH);
  Serial.write(note);
  Serial.write(vel);
}

void sendNoteOff(uint8_t note) {
  Serial.write(0x80 | MIDI_CH);
  Serial.write(note);
  Serial.write((uint8_t)0);
}

// ---------------- SENSORES ----------------
float readDistanceCm() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  unsigned long d = pulseIn(PIN_ECHO, HIGH, 25000);  // timeout ~4 m
  if (d == 0) return -1;
  return d * 0.0343 / 2.0;
}

void updateVoice(Voice &v) {
  int raw = analogRead(v.pin);
  v.smooth += 0.25 * (raw - v.smooth);
}

// Devuelve 0.0 (oscuro) - 1.0 (mucha luz) segun el rango calibrado
float lightLevel(const Voice &v) {
  if (v.maxV - v.minV < 200) return 1.0;  // sin rango util: considerar "sin accion"
  float n = (v.smooth - v.minV) / (float)(v.maxV - v.minV);
  return constrain(n, 0.0, 1.0);
}

// ---------------- CALIBRACION ----------------
// Durante 5 s (LEDs azules): tapa y destapa cada LDR varias veces
void calibrate() {
  unsigned long start = millis();
  while (millis() - start < 5000) {
    strip.setBrightness(40);
    strip.fill(strip.Color(0, 0, 255));
    strip.show();

    for (int i = 0; i < 3; i++) {
      int raw = analogRead(voices[i].pin);
      voices[i].smooth = raw;
      if (raw < voices[i].minV) voices[i].minV = raw;
      if (raw > voices[i].maxV) voices[i].maxV = raw;
    }
    delay(10);
  }
  strip.clear();
  strip.show();
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);  // debe coincidir con HairlessMIDI

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  analogReadResolution(12);

  strip.begin();
  strip.show();

  calibrate();
}

// ---------------- LOOP ----------------
void loop() {
  unsigned long now = millis();

  // --- Parte 1: distancia -> brillo (cada 50 ms) ---
  if (now - lastDist >= 50) {
    lastDist = now;
    float dist = readDistanceCm();

    if (dist > 0) {
      float t = constrain((dist - DIST_MIN) / (DIST_MAX - DIST_MIN), 0.0, 1.0);
      float target = (1.0 - t) * BRIGHT_MAX;   // cerca = mas brillo
      brightSmooth += 0.3 * (target - brightSmooth);
    }

    strip.setBrightness((uint8_t)brightSmooth);
    strip.fill(strip.Color(255, 80, 20));      // color base, cambialo a gusto
    strip.show();
  }

  // --- Parte 2: cada LDR dispara su propia nota ---
  for (int i = 0; i < 3; i++) updateVoice(voices[i]);

  if (now - lastMidi >= 20) {
    lastMidi = now;

#if DEBUG
    Serial.print("LDR1: "); Serial.print(voices[0].smooth);
    Serial.print("  LDR2: "); Serial.print(voices[1].smooth);
    Serial.print("  LDR3: "); Serial.print(voices[2].smooth);
    Serial.print("  Dist-bright: "); Serial.println((int)brightSmooth);
#else
    for (int i = 0; i < 3; i++) {
      Voice &v = voices[i];
      float level = lightLevel(v);
      // "intensidad" de la accion: 0 = reposo, 1 = maximo (tapada o iluminada)
      float amount = TRIGGER_ON_SHADOW ? (1.0 - level) : level;
      float onTh  = TRIGGER_ON_SHADOW ? (1.0 - THRESH_ON)  : THRESH_ON;
      float offTh = TRIGGER_ON_SHADOW ? (1.0 - THRESH_OFF) : THRESH_OFF;

      bool turnOn, turnOff;
      if (TRIGGER_ON_SHADOW) {
        turnOn  = (level < THRESH_ON);
        turnOff = (level > THRESH_OFF);
      } else {
        turnOn  = (level > THRESH_OFF);
        turnOff = (level < THRESH_ON);
      }

      if (!v.active && turnOn) {
        uint8_t vel = 40 + (uint8_t)(constrain(amount, 0.0, 1.0) * 87);  // 40-127
        sendNoteOn(v.note, vel);
        v.active = true;
      } else if (v.active && turnOff) {
        sendNoteOff(v.note);
        v.active = false;
      }
    }
#endif
  }
}