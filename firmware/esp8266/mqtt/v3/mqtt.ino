/************************** Inclusão das Bibliotecas **************************/

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Wire.h>
#include "Adafruit_MCP9808.h"

/****************************** User Config ***********************************/

#include "user_config.h"
#include "user_config_override.h"

/**************************** DEBUG *******************************/

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINTLN(m) Serial.println(m)
#define DEBUG_PRINT(m) Serial.print(m)

#else
#define DEBUG_PRINTLN(m)
#define DEBUG_PRINT(m)

#endif

/**************************** Variaveis globais *******************************/

ADC_MODE(ADC_VCC);

unsigned long sensorPreviousMillis      = 0;
unsigned long statisticsPreviousMillis  = 0;

uint8_t boot_count = 0;

char temp[4];
char statistics[100];

/************************* Declaração dos Prototypes **************************/

void initSerial();
void incrementBootCount();
void initWiFi();
void initMQTT();
void initMCP9808();
void readTemperature();
void sendTemperature();

/************************* Instanciação dos objetos  **************************/

Adafruit_MCP9808 mcp9808 = Adafruit_MCP9808();
WiFiClient client;
PubSubClient mqtt(client);

/********************************** Sketch ************************************/

void setup() {
  initSerial();
  incrementBootCount();
  initWiFi();
  initMQTT();
  initMCP9808();
}

void loop() {



  if (!mqtt.connected()) {
    reconnectMQTT();
  }

  yield();

  recconectWiFi();

  yield();

  mqtt.loop();

  yield();

  sensorLoop();
  statisticsLoop();

}

/*********************** Implementação dos Prototypes *************************/

void incrementBootCount() {
  EEPROM.begin(4);
  int value = (int)(EEPROM.read(25));
  boot_count = value + 1;
  EEPROM.write(25, boot_count);
  EEPROM.end();
}

void readTemperature()
{
  //  mcp9808.shutdown_wake(0);
  float c = mcp9808.readTempC();
  dtostrf(c, 2, 2, temp);
  //  mcp9808.shutdown_wake(1);
}

void sendTemperature()
{
  mqtt.publish(MQTT_TOPIC_SENSOR, temp, true);
  DEBUG_PRINTLN("[SENSOR] Temperatura: " + String(temp));
}

/* Função responsável por publicar a cada X segundos o valor do sensor */
void sensorLoop() {
  unsigned long currentMillis = millis();
  if (currentMillis - sensorPreviousMillis >= sensor_interval && mqtt.connected()) {
    sensorPreviousMillis = currentMillis;

    readTemperature();
    sendTemperature();
  }
}

void statisticsLoop() {
  unsigned long currentMillis = millis();

  if (currentMillis - statisticsPreviousMillis >= statistics_interval) {
    statisticsPreviousMillis = currentMillis;

    String payload;
    payload += "{";
    payload += "\"vcc\":";
    payload += ESP.getVcc() / 1024.00f;
    payload += ",\"memory\":";
    payload += ESP.getFreeHeap();
    payload += ",\"chip_id\":";
    payload += ESP.getChipId();
    payload += ",\"uptime\":";
    payload += millis();
    payload += ",\"boot_count\":";
    payload += String(boot_count);
    payload += "}";

    DEBUG_PRINTLN("[STATISTICS] " + payload);

    payload.toCharArray(statistics, 100);
    mqtt.publish(MQTT_TOPIC_STATISTICS, statistics);
  }
}

/* Conexao Serial */
void initSerial() {
#ifdef DEBUG
  Serial.begin(115200);
#endif
}

/* Configuração da conexão WiFi */
void initWiFi() {
  delay(10);
  DEBUG_PRINT("[WIFI] Conectando-se em " + String(WIFI_SSID));

  WiFi.config(ip, gw, subnet);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG_PRINT(".");
  }

  DEBUG_PRINTLN("");
  DEBUG_PRINT(F("[WIFI] SSID: "));
  DEBUG_PRINTLN(WIFI_SSID);
  DEBUG_PRINT(F("[WIFI] IP: "));
  DEBUG_PRINTLN(WiFi.localIP());
  DEBUG_PRINT(F("[WIFI] Mac: "));
  DEBUG_PRINTLN(WiFi.macAddress());
  DEBUG_PRINTLN("");
}

/* Configuração da conexão MQTT */
void initMQTT() {
  mqtt.setServer(BROKER_MQTT, BROKER_PORT);
}

/* Inicialização do Sensor */
void initMCP9808() {
  if (!mcp9808.begin()) {
    DEBUG_PRINTLN("[SENSOR] MCP9808 não pode ser iniciado!");
    while (1);
  }
}

/* Demais implementações */

void reconnectMQTT() {
  if (!mqtt.connected()) {
    DEBUG_PRINT(F("[BROKER] Tentando se conectar ao Broker MQTT: "));
    DEBUG_PRINTLN(BROKER_MQTT);

    while (!mqtt.connected()) {
      if (mqtt.connect("sensor_01", "douglaszuqueto/casa_01/cozinha/temperatura/sensor_01/offline", 0, false, "offline")) {
        DEBUG_PRINTLN("");
        DEBUG_PRINTLN(F("[BROKER] Conectado"));
      } else {
        DEBUG_PRINT(".");
        delay(500);
      }
    }
  }

  DEBUG_PRINTLN("");
}

void recconectWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG_PRINT(".");
  }
}
