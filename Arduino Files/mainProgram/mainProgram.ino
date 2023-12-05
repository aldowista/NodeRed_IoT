#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "DHT.h"
#include "AESLib.h"
#include <SHA256.h>
#include <Base64.h> 
//#include "BlockCipher.h"
// DEVICE NO
String device_suffix = "2";

// Pin definitions
#define DHTPIN 25
#define LED1_PIN 12
#define LED2_PIN 27
#define BUTTON_PIN 39
#define DHTTYPE DHT11

// Wi-Fi and MQTT Credentials
const char* ssid = "estudines Xl2";
const char* password = "couronne470";
const char* mqtt_server = "d9bbc92ca3ee4c19814e290829169611.s2.eu.hivemq.cloud"; // replace with your broker url
// Dynamic generation of MQTT credentials
String mqtt_username = String("sensorNode") + String(device_suffix);
String mqtt_password = String("sensorNode") + String(device_suffix);
const int mqtt_port = 8883;

// MQTT Topics
String usrAction_topic = String("usrAction") + String(device_suffix);
String pirSensor_topic = String("pirSensor") + String(device_suffix);
String temperature_topic = String("Temperature") + String(device_suffix);
String humidity_topic = String("Humidity") + String(device_suffix);
String encryptedTemperature_topic = String("encryptTemp") + String(device_suffix);
String encryptedHumidity_topic = String("encryptHum") + String(device_suffix);

// global variables
WiFiClientSecure espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
AESLib aesLib;
byte aes_key[] = { 0x65, 0x72, 0x61, 0x73, 0x6d, 0x75, 0x73, 0x6d, 0x75, 0x6e, 0x64, 0x75, 0x73, 0x31, 0x32, 0x33 };//Key : erasmusmundus123
byte aes_iv[N_BLOCK] = { 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30 };//IV : 0000000000000000

// Sensor Data Structure Definition
struct SensorData {
  float temperature;
  float humidity;
};

// Function Declarations
void setupWifi();
void setupMQTT();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);
void publishMessage(const char* topic, String payload , boolean retained);
void aes_init();
void handleButton();
SensorData readSensor();

void setup() {
  // connect serial terminal
  Serial.begin(9600);
  while (!Serial) delay(1);
  
  setupWiFi();
  setupMQTT();

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  dht.begin();
  aes_init();
  aesLib.set_paddingmode(paddingMode::CMS);
}

void loop() {
  Serial.println("\n\n###");
  if (!client.connected()) reconnect(); // check if client is connected
  client.loop();
  
  handleButtonClick();


  SensorData sensorData = readSensor();
  if (!isnan(sensorData.temperature) && !isnan(sensorData.humidity)) { // if data read

      String encryptedTemperature = encrypt_impl(strdup(String("23.60").c_str()));
      String hashedTemperature = hash_impl(strdup(String("23.60").c_str()));
      String encryptedHashedTemperature = encrypt_impl(strdup(hashedTemperature.c_str()));
      //Serial.printf("\nencryptedTemperature: %s", encryptedTemperature);
      //Serial.printf("\nhashedTemperature: %s",hashedTemperature );
      String temperatureToSend = encryptedTemperature + "|" + encryptedHashedTemperature;
      Serial.println("Hashed Temperature "+hashedTemperature);
      //String encryptedHumidity = encrypt_impl(strdup(String(sensorData.humidity).c_str()));
      //Serial.printf("\nencryptedHumidity: %s", encryptedHumidity);

      //publishMessage(temperature_topic,String(sensorData.temperature),true);
      //publishMessage(humidity_topic,String(sensorData.humidity),true);
      publishMessage(encryptedTemperature_topic, temperatureToSend, true);
      //publishMessage(encryptedHumidity_topic, encryptedHumidity, true);

  }
}





