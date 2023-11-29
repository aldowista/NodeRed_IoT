#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "DHT.h"
#include "AESLib.h"

#define DHTPIN 25
#define LED1_PIN 12
#define LED2_PIN 27
#define BUTTON_PIN 39

#define DHTTYPE DHT11

AESLib aesLib;

char cleartext[128];
char ciphertext[256];
//Key : erasmusmundus123
byte aes_key[] = { 0x65, 0x72, 0x61, 0x73, 0x6d, 0x75, 0x73, 0x6d, 0x75, 0x6e, 0x64, 0x75, 0x73, 0x31, 0x32, 0x33 };
//IV : 0000000000000000
byte aes_iv[N_BLOCK] = { 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30 };

String encrypt_impl(char * msg, byte iv[]) {
  int msgLen = strlen(msg);
  char encrypted[2 * msgLen] = {0};
  aesLib.encrypt64((const byte*)msg, msgLen, encrypted, aes_key, sizeof(aes_key), iv);
  return String(encrypted);
}

String decrypt_impl(char * msg, byte iv[]) {
  int msgLen = strlen(msg);
  char decrypted[msgLen] = {0}; // half may be enough
  aesLib.decrypt64(msg, msgLen, (byte*)decrypted, aes_key, sizeof(aes_key), iv);
  return String(decrypted);
}

// Generate IV (once)
void aes_init() {
  //Serial.println("gen_iv()");
  aesLib.gen_iv(aes_iv);
  //Serial.println("encrypt_impl()");
  //Serial.println(encrypt_impl(strdup(plaintext.c_str()), aes_iv));
}

//---- WiFi settings
const char* ssid = "Aagneya";
const char* password = "Aagneya123";

//---- MQTT Broker settings
const char* mqtt_server = "f01d92822ed54ac88447d7016a115c49.s1.eu.hivemq.cloud"; // replace with your broker url
const char* mqtt_username = "sensorNode2";
const char* mqtt_password = "sensorNode2";
const int mqtt_port = 8883;

WiFiClientSecure espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

const char* dht11_temperature= "Temperature2";
const char* dht11_humidity= "Humidity2";
const char* encryptTemp = "encryptTemp2";
const char* encryptHum = "encryptHum2";
const char* sensorPir= "pirSensor2";
const char* usrAction= "usrAction2";


float humidity = 0;
float temperature =0;
int usrActionValue =0;
char* enCOutput = "";

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());

  while (!Serial) delay(1);

  espClient.setCACert(root_ca);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  dht.begin();
  aes_init();
  aesLib.set_paddingmode(paddingMode::CMS);

  char b64in[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  char b64out[base64_enc_len(sizeof(aes_iv))];
  base64_encode(b64out, b64in, 16);
  char b64enc[base64_enc_len(10)];
  base64_encode(b64enc, (char*) "0123456789", 10);
  char b64dec[ base64_dec_len(b64enc, sizeof(b64enc))];
  base64_decode(b64dec, b64enc, sizeof(b64enc));
}

void reconnect() {
// Loop until we’re reconnected
  while (!client.connected()) {
  Serial.print("Attempting MQTT connection…");
  String clientId = "ESP8266Client-"; // Create a random client ID
  clientId += String(random(0xffff), HEX);
  // Attempt to connect
  if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
  Serial.println(": connected");
  client.subscribe(usrAction);
    //client.subscribe(dht11_temperature);   // subscribe the topics here
    //client.subscribe(dht11_humidity);   // subscribe the topics here
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");   // Wait 5 seconds before retrying
    delay(5000);
}
}
}

void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; i++) incommingMessage+=(char)payload[i];
  Serial.println("Message arrived ["+String(topic)+"]"+incommingMessage);
  // check for other commands
  /* else if( strcmp(topic,command2_topic) == 0){
  if (incommingMessage.equals(“1”)) { } // do something else
  }
  */
  if (String(topic) == "usrAction"){
    if(incommingMessage == "1"){
      Serial.println("Lamp : ON");
      digitalWrite(LED2_PIN, HIGH);
    }
    else if(incommingMessage == "0"){
      Serial.println("Lamp : OFF");
      digitalWrite(LED2_PIN, LOW);
    }
  }

}

void publishMessage(const char* topic, String payload , boolean retained){
  if (client.publish(topic, payload.c_str(), true))
  Serial.println("Message publised ["+String(topic)+"]: "+payload);
}

/* non-blocking wait function */
void wait(unsigned long milliseconds) {
  unsigned long timeout = millis() + milliseconds;
  while (millis() < timeout) {
    yield();
  }
}

byte enc_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // iv_block gets written to, provide own fresh copy...
byte dec_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static char outstr1[3];
static char outstr2[3];

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) reconnect(); // check if client is connected
  client.loop();

  if (digitalRead(BUTTON_PIN) == HIGH) {
    digitalWrite(LED1_PIN, HIGH);
    publishMessage(sensorPir,String(1),true);
  }
  else {
    digitalWrite(LED1_PIN, LOW);
    publishMessage(sensorPir,String(0),true);
  }

  readTemp();
  
  String encrypted = encrypt_impl(strdup(String(temperature).c_str()), enc_iv);
  Serial.print("Ciphertext: ");
  Serial.println(encrypted);
  for (int i = 0; i < 16; i++) {
      enc_iv[i] = 0;
      dec_iv[i] = 0;
  }

  String encrypted2 = encrypt_impl(strdup(String(humidity).c_str()), enc_iv);
  Serial.print("Ciphertext: ");
  Serial.println(encrypted2);
  for (int i = 0; i < 16; i++) {
      enc_iv[i] = 0;
      dec_iv[i] = 0;
  }
  
  publishMessage(dht11_temperature,String(temperature),true);
  publishMessage(dht11_humidity,String(humidity),true);
  publishMessage(encryptTemp, encrypted, true);
  publishMessage(encryptHum, encrypted2, true);
  //publishMessage(pirSensor,String(),true);
  
 
  //if (usrAction == "1") {
  //  digitalWrite(LED2_PIN, HIGH);
  // }
  //else {
  //  digitalWrite(LED2_PIN, LOW);
  //}
}
