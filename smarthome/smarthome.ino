/*
ESP32 publish telemetry data to Google Cloud (DHT11 sensor)
*/
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <ESP32Servo.h>

const char *WIFI_SSID = "MH"; //your WiFi SSID
const char *WIFI_PASSWORD = "abcde123"; // your password
const char *MQTT_SERVER = "34.31.15.95"; // your VM instance public IP address
const int MQTT_PORT = 1883;
const char *MQTT_TOPIC = "-iot"; // MQTT topic

// DEFFINE INPUT PIN
const int tempPin = A4;        // LEFT SIDE MAKER PORT
const int rainPin = A2;       // MIDDLE MAKER PORT
const int IRPin = 42;     // RIGHT SIDE MAKER PORT

// DEFINE OUTPUT PIN
const int ledNbuzzer = 21; // FOR HOME SECURITY
const int light = 48;
const int fan = 47;
const int servoPin = 40;

// FOR DHT SENSOR
#define DHTTYPE DHT11
DHT dht(tempPin, DHTTYPE);

//  FOR SERVO
Servo Myservo;

// FOR RAIN SENSOR
int MinDepthValue = 4095;
int MaxDepthValue = 2170;
int MinDepth = 0;
int MaxDepth = 100;
int depth = 0;
const int rainThreshold = 50; // Set this according to your needs
const int shadingPosition = 180; // Set this to the angle you want for shading

// FOR IR SENSOR
unsigned long lastTrigger = 0;
boolean obstacle = false;

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

  // actuator
  Myservo.attach(servoPin);
  Myservo.write(0);
  pinMode(ledNbuzzer, OUTPUT);
  pinMode(light, OUTPUT);
  pinMode(fan, OUTPUT);

  // SET INITAL OUTPUT TO LOW
  digitalWrite(ledNbuzzer, LOW);
  digitalWrite(light, LOW);
  digitalWrite(fan, LOW);

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
  if (temperature > 31) {
    Serial.println("Temperature is above 31C. Turning fan ON.");
    digitalWrite(fan, HIGH); // Turn the fan on
  } else {
    Serial.println("Temperature is below 31C. Turning fan OFF.");
    digitalWrite(fan, LOW); // Turn the fan off
  }

  // Read rain sensor data
  int rain = analogRead(rainPin);
  sprintf(payload, "Rain Depth: %.2f", rain);
  client.publish(MQTT_TOPIC, payload);
  depth = map(rain, MinDepthValue, MaxDepthValue, MinDepth, MaxDepth);
  // Activate shading when rain is detected
  if (depth > rainThreshold) {
      Myservo.write(shadingPosition); // Move servo to shading position
  } else {
      Myservo.write(0); // Reset servo position when there's no rain
  }

  // Handle motion detection by IR Sensor
  int obstacle = !digitalRead(IRPin);
  sprintf(payload, "Obstacle: %d", obstacle);
  client.publish(MQTT_TOPIC, payload);
  //  Activate LED and buzzer when motion detected
  if (obstacle == HIGH) {
      // Turn on LED and buzzer
      digitalWrite(ledNbuzzer, HIGH);
  }
  else {
    digitalWrite(ledNbuzzer, LOW);
  }

}
