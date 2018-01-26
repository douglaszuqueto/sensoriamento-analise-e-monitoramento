/************************** Inclusão das Bibliotecas **************************/

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

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

#ifdef DEBUG
#define DEBUG_PRINTLN(m) Serial.println(m)
#define DEBUG_PRINT(m) Serial.print(m)

#else
#define DEBUG_PRINTLN(m)
#define DEBUG_PRINT(m)

#endif

/**************************** Variaveis globais *******************************/

ADC_MODE(ADC_VCC);

char temp[6];
char humi[6];

uint32_t sleep_time = 60 * 1000000; // intervalo de 1 minuto

/************************* Declaração dos Prototypes **************************/

void initSerial();
void initWiFi();
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
HTTPClient http;

/*********************** Implementação dos Prototypes *************************/

void initSerial() {
#ifdef DEBUG
  Serial.begin(115200);
#endif
}

void initWiFi() {
  delay(10);
  DEBUG_PRINTLN("");
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

void initSensor() {

#ifdef MCP9808_SENSOR

  if (!mcp9808.begin()) {
    DEBUG_PRINTLN("[SENSOR] MCP9808 não pode ser iniciado!");
    while (1);
  }

#else

  dht.begin();

#endif

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

void sensorLoop() {
  readSensor();
  sendData();
}

void sendData() {

  String payload;
  payload += "{";
  payload += "\"id\":";
  payload += ESP.getChipId();
  payload += ",\"t\":";
  payload += temp;
  payload += ",\"h\":";
  payload += humi;
  payload += ",\"v\":";
  payload += ESP.getVcc() / 1024.00f;
  payload += ",\"m\":";
  payload += ESP.getFreeHeap();
  payload += ",\"u\":";
  payload += millis();
  payload += "}";

  http.begin(API_URL);
  http.addHeader("content-type", "application/json");

  int httpCode = http.POST(payload);

  if (httpCode < 0) {
    Serial.println("request error - " + httpCode);
  }

  if (httpCode != HTTP_CODE_OK) {
    Serial.println("request error - " + httpCode);
  }

  String response =  http.getString();
  http.end();

  DEBUG_PRINTLN("[SENSOR] Payload " + payload);
  DEBUG_PRINTLN("[SENSOR] Length " + String(payload.length()));

  DEBUG_PRINTLN("[HTTP] Response " + response);
  DEBUG_PRINTLN("[HTTP] Length: " + String(response.length()));
  DEBUG_PRINTLN("[HTTP] Code: " + String(httpCode));
}

/********************************** Sketch ************************************/

void setup() {
  initSerial();
  initWiFi();
  initSensor();

  sensorLoop();

  //  delay(250);
  DEBUG_PRINTLN("Sleeping...");
  ESP.deepSleep(sleep_time);
}

void loop() {
}
