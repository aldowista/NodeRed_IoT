#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "DHT.h"
#include <SHA256.h>
#include "AES.h"

// DEVICE NO
String device_suffix = "1";

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
AES128 aes128;
uint8_t aes_key[] = { 0x65, 0x72, 0x61, 0x73, 0x6D, 0x75, 0x73, 0x6D, 0x75, 0x6E, 0x64, 0x75, 0x73, 0x31, 0x32, 0x33 };
//byte aes_iv[N_BLOCK] = { 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30 };//IV : 0000000000000000

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
  setupAES();
}

void loop() {
  Serial.println("\n\n###");
  if (!client.connected()) reconnect(); // check if client is connected
  client.loop();
  
  handleButtonClick();


  SensorData sensorData = readSensor();
  if (!isnan(sensorData.temperature) && !isnan(sensorData.humidity)) { // if data read
    // Encrypt Temperature
    String temperature = (String)sensorData.temperature;
    const uint8_t* tempByteArray = (const uint8_t*)temperature.c_str();
    size_t tempByteArrayLen = temperature.length();
    String encryptedTemperature = encryptAndEncodeBase64(tempByteArray, tempByteArrayLen);

    // Hash Temperature
    SHA256 hasher = SHA256();
    hasher.update(tempByteArray, tempByteArrayLen);
    int tempHashLen = hasher.hashSize();
    uint8_t tempHash[tempHashLen] = {0};
    hasher.finalize(tempHash, tempHashLen);

    // Encode Hashed Temperature to Base64
    String hashString = base64_encode(tempHash, tempHashLen);
    const uint8_t* hashByteArray = (const uint8_t*)hashString.c_str();
    size_t hashByteArrayLength = hashString.length();

    // Encrypt Temperature Hash
    String encryptedHashedTemperature = encryptAndEncodeBase64(hashByteArray, hashByteArrayLength);
    String temperatureToSend = encryptedTemperature + "|" + encryptedHashedTemperature;
    publishMessage(encryptedTemperature_topic, temperatureToSend, true);

    // ################ HUMIDITY ####################

    // Encrypt Humidity
    String humidity = (String)sensorData.humidity;
    const uint8_t* humidityByteArray = (const uint8_t*)humidity.c_str();
    size_t humidityByteArrayLen = humidity.length();
    String encryptedHumidity = encryptAndEncodeBase64(humidityByteArray, humidityByteArrayLen);

    // Hash Humidity
    // Reuse hasher object
    hasher.reset(); // Reset the hasher before reuse
    hasher.update(humidityByteArray, humidityByteArrayLen);
    int humidityHashLen = hasher.hashSize();
    uint8_t humidityHash[humidityHashLen] = {0};
    hasher.finalize(humidityHash, humidityHashLen);

    // Encode Hashed Humidity to Base64
    String humidityHashString = base64_encode(humidityHash, humidityHashLen);
    // Reuse hashByteArray variable
    hashByteArray = (const uint8_t*)humidityHashString.c_str();
    hashByteArrayLength = humidityHashString.length();

    // Encrypt Humidity Hash
    String encryptedHashedHumidity = encryptAndEncodeBase64(hashByteArray, hashByteArrayLength);
    String humidityToSend = encryptedHumidity + "|" + encryptedHashedHumidity;
    publishMessage(encryptedHumidity_topic, humidityToSend, true);

  }
}





