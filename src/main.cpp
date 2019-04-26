#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>

// Creación de objeto que representa el sensor
SoftwareSerial rfid = SoftwareSerial(12, true);

bool autentification;
bool hayLectura;

const int led = 14;
const int trigPin = 5;
const int echoPin = 4;
const int buzzer = 0;

const char *ssid = "DCI_1Piso";
const char *password = "DCI.2019";
const char *mqttServer = "broker.hivemq.com";
const char *mqtt_topic = "dci/test";
const int mqttPort = 1883; // MQTT PORT

WiFiClient espClient;           // Creación de cliente WIFI
PubSubClient client(espClient); // Creación de cliente MQTT

int maxDistance = 20;
long duration;
int currentDistance;

void setup_wifi()
{
  delay(100);
  Serial.print("Conectando a: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("Coneccion WiFi realizada");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      client.subscribe(mqtt_topic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(6000);
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  int valor = 0;
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  String pwmValue_str = "";
  Serial.print("Message:");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    pwmValue_str = pwmValue_str + (char)payload[i];
  }
  Serial.println();
  Serial.println("-----------------------");
  Serial.println(pwmValue_str);
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
  if (!autentification)
  {
    autentification = true;
    client.publish(mqtt_topic, "Desbloqueado");
    client.subscribe(mqtt_topic);
  
  }
  else
  {
    autentification = false;
    client.publish(mqtt_topic, "Bloqueado");
    client.subscribe(mqtt_topic);
    
  }
}

void lockLed()
{
  if (!autentification)
    digitalWrite(led, HIGH);
  else
    digitalWrite(led, LOW);
}

void calcularDistancia()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  currentDistance = (duration / 2) / 29.1;
}

void setup()
{
  Serial.begin(115200); // baudios del dispositivo
  rfid.begin(9600);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  while (!client.connected())
  {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP8266Client"))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  client.publish(mqtt_topic, "Succesfull");
  client.subscribe(mqtt_topic);

  autentification = false;

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
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
  doSound();
  hayLectura = false;

  if (!client.connected())
  {
    reconnect();
  }
  //client.loop();

  delay(500);
}