#include <WiFi.h>
#include <PubSubClient.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>

const char* mqttServer = "192.168.1.13";
const int mqttPort = 1883;  // Puerto MQTT por defecto

WiFiClient espClient;
PubSubClient client(espClient);

String ssid = "";
String password = "";

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CREDENTIALS_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer* pServer = NULL;
BLECharacteristic* pCredentialsCharacteristic = NULL;

void connectToWiFi();
void connectToMQTT();
void sendData();

class CredentialsCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
      String credentials = String(value.c_str());
      int separatorIndex = credentials.indexOf(';');
      ssid = credentials.substring(0, separatorIndex);
      password = credentials.substring(separatorIndex + 1);
      Serial.println("Credenciales recibidas:");
      Serial.print("SSID: ");
      Serial.println(ssid);
      Serial.print("Password: ");
      Serial.println(password);
      connectToWiFi();
    }
  }
};

void setup() {
  Serial.begin(115200);

  BLEDevice::init("ESP32-S3");
  pServer = BLEDevice::createServer();
  BLEService* pService = pServer->createService(SERVICE_UUID);
  pCredentialsCharacteristic = pService->createCharacteristic(
    CREDENTIALS_UUID,
    BLECharacteristic::PROPERTY_WRITE);
  pCredentialsCharacteristic->setCallbacks(new CredentialsCallbacks());
  pService->start();
  BLEAdvertising* pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  client.setServer(mqttServer, mqttPort);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      connectToMQTT();
    }
    client.loop();
    sendData();
    delay(10000);  // Enviar datos cada 10 segundos
  }
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid.c_str(), password.c_str());
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to the WiFi network");
    connectToMQTT();
  } else {
    Serial.println("Failed to connect to WiFi");
  }
}

void connectToMQTT() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void sendData() {
  if (client.connected()) {
    StaticJsonDocument<200> jsonDocument;
    jsonDocument["peso"] = 10;
    jsonDocument["temperatura"] = rand() % 11 + 20;  // Genera un número entre 20 y 30
    String jsonStr;
    serializeJson(jsonDocument, jsonStr);
    client.publish("datosTest", jsonStr.c_str());
  }
}
