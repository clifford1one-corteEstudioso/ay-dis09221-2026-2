#include <Adafruit_NeoPixel.h>
const int ledPin = 6;

Adafruit_NeoPixel ring(7, ledPin, NEO_GRB + NEO_KHZ800);




const int trigPin = 9;
const int echoPin = 10;
//este es el pin del led
float duration, distance;


void setup() {
  /**/
  pinMode(trigPin, OUTPUT);
  //este es el pin del led
    // pinMode(ledPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600);

  ring.begin();
  ring.setBrightness(50);   // 0–255, evita consumo excesivo
  ring.show();              // arranca apagado

}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration * .0343) / 2;
  Serial.print("Distance: ");
  Serial.println(distance);
  // if (distance < 20) { digitalWrite(ledPin, HIGH); }

// en loop:
if (distance < 20) {
  ring.fill(ring.Color(0, 0, 255));
} else {
  ring.clear();
}
ring.show();
}