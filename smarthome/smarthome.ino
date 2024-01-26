/*
ESP32 publish telemetry data to Google Cloud (DHT11 sensor)
*/
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

const char *WIFI_SSID = "MH"; //your WiFi SSID
const char *WIFI_PASSWORD = "abcde123"; // your password
const char *MQTT_SERVER = "34.31.15.95"; // your VM instance public IP address
const int MQTT_PORT = 1883;
const char *MQTT_TOPIC = "-iot"; // MQTT topic

// DEFFINE INPUT PIN
const int tempPin = A4;        // LEFT SIDE MAKER PORT
const int rainPin = A2;       // MIDDLE MAKER PORT
const int IRPin = 42;     // RIGHT SIDE MAKER PORT

// FOR DHT SENSOR
#define DHTTYPE DHT11
DHT dht(tempPin, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi()
{
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
 {
  delay(500);
  Serial.print(".");
 }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(115200);

  // sensor
  dht.begin();
  pinMode(rainPin, INPUT);
  pinMode(tempPin, INPUT);
  pinMode(IRPin, INPUT);

  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("ESP32Client"))
    {
      Serial.println("Connected to MQTT server");
    }
    else
    {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
 }
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  delay(5000); // adjust the delay according to your requirements

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  char payload[50];
  sprintf(payload, "Temperature: %.2f, Humidity: %.2f", temperature, humidity);
  client.publish(MQTT_TOPIC, payload);

  float rain = analogRead(rainPin);
  sprintf(payload, "Rain Depth: %.2f", rain);
  client.publish(MQTT_TOPIC, payload);

  int obstacle = digitalRead(IRPin);
  sprintf(payload, "Obstacle: %d", obstacle);
  client.publish(MQTT_TOPIC, payload);

}
