#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>  // Necesitarás instalar la librería ArduinoJson

const char* ssid = "WIFI HOME_CNT_FIBRA";
//const char* ssid = "DOCENCIA";
const char* password = "LjjFdiduscrapi789";
//const char* password = "Doc23#Acad";
const char* mqttServer = "192.168.1.12"; 
//const char* mqttServer = "172.16.132.82";  
const int mqttPort = 1883;                   // Puerto MQTT por defecto

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to the WiFi network");
  client.setServer(mqttServer, mqttPort);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Crear un objeto JSON para los datos
  StaticJsonDocument<200> jsonDocument;

  // Primero enviamos peso 10
  jsonDocument["peso"] = 10;
  jsonDocument["temperatura"] = rand() % 11 + 20;  // Genera un número entre 20 y 30
  String jsonStr;
  serializeJson(jsonDocument, jsonStr);
  client.publish("datosTest", jsonStr.c_str());
  delay(10000);  // Espera 10 segundos

  // Luego enviamos peso 5
  jsonDocument["peso"] = 5;
  jsonDocument["temperatura"] = rand() % 11 + 20;  // Genera un número entre 20 y 30
  serializeJson(jsonDocument, jsonStr);
  client.publish("datosTest", jsonStr.c_str());
  delay(10000);  // Espera 10 segundos
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println("Retrying in 5 seconds...");
      delay(5000);
    }
  }
}
