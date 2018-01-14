/************************** Inclusão das Bibliotecas **************************/

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/****************************** User Config ***********************************/

#include "user_config.h"
#include "user_config_override.h"

#ifdef MCP9808_SENSOR

#include <Wire.h>
#include "Adafruit_MCP9808.h"

#else
#include "DHT.h"
#endif

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

char temp[6];
char humi[6];

char statisticsPayload[100];
char sensorPayload[100];

/************************* Declaração dos Prototypes **************************/

void initSerial();
void incrementBootCount();
void initWiFi();
void initMQTT();
void initSensor();
void readSensor();
void sendData();

/************************* Instanciação dos objetos  **************************/

#ifdef MCP9808_SENSOR
Adafruit_MCP9808 mcp9808 = Adafruit_MCP9808();
#else
#define DHTPIN D5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
#endif

WiFiClient client;
PubSubClient mqtt(client);

/********************************** Sketch ************************************/

void setup() {
  initSerial();
  incrementBootCount();
  initWiFi();
  initMQTT();
  initSensor();
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

void readSensor()
{
  float t, h = 0;

#ifdef MCP9808_SENSOR

  //  mcp9808.shutdown_wake(0);
  t = mcp9808.readTempC();
  dtostrf(t, 2, 2, temp);
  dtostrf(h, 2, 2, humi);

  //  mcp9808.shutdown_wake(1);

#else

  t = dht.readTemperature();
  h = dht.readHumidity();

  if (isnan(t)) {
    t = 0;
  }

  if (isnan(h)) {
    h = 0;
  }

  dtostrf(t, 2, 2, temp);
  dtostrf(h, 2, 2, humi);

#endif
}

void sendData()
{

  String payload;
  payload += "{";
  payload += "\"id\":";
  payload += ESP.getChipId();
  payload += ",\"t\":";
  payload += temp;
  payload += ",\"h\":";
  payload += humi;
  payload += "}";

  payload.toCharArray(sensorPayload, 100);
  mqtt.publish(MQTT_TOPIC_SENSOR, sensorPayload);
  DEBUG_PRINTLN("[SENSOR] " + String(sensorPayload));
}

/* Função responsável por publicar a cada X segundos o valor do sensor */
void sensorLoop() {
  unsigned long currentMillis = millis();
  if (currentMillis - sensorPreviousMillis >= sensor_interval && mqtt.connected()) {
    sensorPreviousMillis = currentMillis;

    readSensor();
    sendData();
  }
}

void statisticsLoop() {
  unsigned long currentMillis = millis();

  if (currentMillis - statisticsPreviousMillis >= statistics_interval) {
    statisticsPreviousMillis = currentMillis;

    String payload;
    payload += "{";
    payload += "\"id\":";
    payload += ESP.getChipId();
    payload += ",\"vcc\":";
    payload += ESP.getVcc() / 1024.00f;
    payload += ",\"memory\":";
    payload += ESP.getFreeHeap();
    payload += ",\"uptime\":";
    payload += millis();
    payload += ",\"boot_count\":";
    payload += String(boot_count);
    payload += "}";

    DEBUG_PRINTLN("[STATISTICS] " + payload);

    payload.toCharArray(statisticsPayload, 100);
    mqtt.publish(MQTT_TOPIC_STATISTICS, statisticsPayload);
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
void initSensor() {

#ifdef MCP9808_SENSOR

  if (!mcp9808.begin()) {
    DEBUG_PRINTLN("[SENSOR] MCP9808 não pode ser iniciado!");
    while (1);
  }

#endif

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
