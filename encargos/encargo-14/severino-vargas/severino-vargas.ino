#include <Adafruit_NeoPixel.h>

// --- Configuración de Pines ---
const int PIN_MIC = 34;    
const int PIN_SERVO = 18;  
const int PIN_LED = 13;      // Pin de datos para la tira LED

// --- Parámetros de la Tira LED ---
const int NUM_LEDS = 30;    // Ajusta según la cantidad de LEDs de tu tira
Adafruit_NeoPixel tira = Adafruit_NeoPixel(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);

// --- Configuración PWM ESP32 v3.x ---
const int FREC_PWM = 50;
const int RES_PWM = 16;

// --- Parámetros Calibrados ---
const int PULSO_PARADO = 4700;    // Tu valor neutro exacto
const int PULSO_MAX_GIRO = 5300;  // Velocidad calibrada

int umbralRuido = 1500;           // Sensibilidad al ruido
unsigned long tiempoInicio = 0;
const unsigned long TIEMPO_LIMITE = 10 * 60 * 1000UL; // 10 minutos (600,000 ms)

// --- Variables para la Acumulación de Ruido y Animación ---
float nivelEstrésRuido = 0.0;    // Transición de 0.0 (Azul) a 1.0 (Rojo)
float anguloRespiracion = 0.0;   // Controla la onda de brillo

void setup() {
  Serial.begin(115200);
 
  // Configurar Servo
  ledcAttach(PIN_SERVO, FREC_PWM, RES_PWM);
  ledcWrite(PIN_SERVO, PULSO_PARADO);

  // Configurar Tira LED
  tira.begin();
  tira.setBrightness(255); // Brillo máximo global (el efecto controla el matiz)
  tira.show();

  tiempoInicio = millis();
  Serial.println("==========================================");
  Serial.println("   SISTEMA CON SERVO Y TIRA LED INICIADO  ");
  Serial.println("==========================================");
}

void loop() {
  unsigned long tiempoActual = millis();
 
  // 1. Reinicio automático a los 10 minutos (Vuelve a Azul y detiene motor)
  if (tiempoActual - tiempoInicio >= TIEMPO_LIMITE) {
    Serial.println("--> ¡Se cumplieron 10 minutos! Reiniciando a estado Azul...");
    ledcWrite(PIN_SERVO, PULSO_PARADO);
    nivelEstrésRuido = 0.0; // Resetea el color a azul puro
    tiempoInicio = tiempoActual;
    delay(2000);
  }

  // 2. Medir amplitud del sonido
  int amplitudSonido = obtenerAmplitudSonido(30);

  // 3. Evaluar Ruido y Mover Motor
  if (amplitudSonido > umbralRuido) {
    int valorPWM = map(amplitudSonido, umbralRuido, 2048, PULSO_PARADO + 250, PULSO_MAX_GIRO);
    valorPWM = constrain(valorPWM, PULSO_PARADO + 250, PULSO_MAX_GIRO);
    ledcWrite(PIN_SERVO, valorPWM);

    // Si hay ruido constante, incrementamos la transición hacia el ROJO
    nivelEstrésRuido += 0.015;
  } else {
    ledcWrite(PIN_SERVO, PULSO_PARADO);
   
    // En silencio, el color se enfría lentamente volviendo hacia el AZUL
    nivelEstrésRuido -= 0.003;
  }

  // Mantener el factor entre 0.0 (Totalmente Azul) y 1.0 (Totalmente Rojo)
  nivelEstrésRuido = constrain(nivelEstrésRuido, 0.0, 1.0);

  // 4. Efecto Ondulatorio de "Respiración" en los LEDs
  // Velocidad de respiración: Más rápido cuando está Rojo, más lento cuando está Azul
  float velocidadRespiracion = mapFloat(nivelEstrésRuido, 0.0, 1.0, 0.03, 0.12);
  anguloRespiracion += velocidadRespiracion;
  if (anguloRespiracion > TWO_PI) anguloRespiracion -= TWO_PI;

  // Calculamos la intensidad luminosa usando una onda senoidal suave (0 a 255)
  float factorBrillo = (sin(anguloRespiracion) + 1.0) / 2.0;

  // Mezcla de colores (Interpolación de Azul [0,0,255] a Rojo [255,0,0])
  int r = (int)(255.0 * nivelEstrésRuido * factorBrillo);
  int g = 0;
  int b = (int)(255.0 * (1.0 - nivelEstrésRuido) * factorBrillo);

  // Aplicar el color calculado a todos los LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    tira.setPixelColor(i, tira.Color(r, g, b));
  }
  tira.show();

  delay(10);
}

// Función Muestreadora de Micrófono
int obtenerAmplitudSonido(int ventanaMs) {
  unsigned long tiempoInicioMuestra = millis();
  int senalMinima = 4095;
  int senalMaxima = 0;

  while (millis() - tiempoInicioMuestra < ventanaMs) {
    int lectura = analogRead(PIN_MIC);
    if (lectura < senalMinima) senalMinima = lectura;
    if (lectura > senalMaxima) senalMaxima = lectura;
  }

  return (senalMaxima - senalMinima);
}

// Función auxiliar para mapear números decimales (float)
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}