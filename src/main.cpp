#include <Arduino.h>
#include <SoftwareSerial.h>
// Creación de objeto que representa el sensor
SoftwareSerial rfid = SoftwareSerial(12,   true);

boolean autentification;
boolean hayLectura;

const int led=14;
const int trigPin = 5;
const int echoPin = 4;
const int buzzer = 0;

int maxDistance=20;
long duration;
int currentDistance;

void setup()
{
  Serial.begin(9600);
  rfid.begin(9600);
  
  autentification = false;

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
}

void doSound()
{
  if (!autentification && currentDistance > maxDistance)
    digitalWrite(buzzer, HIGH);
  else
    digitalWrite(buzzer, LOW);
}

void lockSystem()
{
  if (!autentification){
    autentification = true;
    Serial.println("Desbloqueado");
  }else{
    autentification = false;
    Serial.println("Bloqueado");
  }
}

void lockLed()
{
  if (!autentification)
    digitalWrite(led, HIGH);
  else
    digitalWrite(led, LOW);
}

void calcularDistancia(){
  digitalWrite(trigPin, LOW);  
  delayMicroseconds(2); 
  
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); 
  
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  currentDistance = (duration/2) / 29.1;
}

void loop()
{
  while (rfid.available() > 0)
  {
    rfid.read();
    hayLectura = true;
  }
  if (hayLectura)
    lockSystem();

  lockLed();
  calcularDistancia();
  Serial.println(currentDistance);
  doSound();
  hayLectura = false;
  delay(500);
}