/************************* Inclusão das Bibliotecas *************************/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <Wire.h>
#include "Adafruit_MCP9808.h"

/****************************** Conexão WiFi *********************************/

const char* SSID      = "homewifi_D68";
const char* PASSWORD  = "09114147";

/****************************** WebService **********************************/

String BASE_URL = "http://192.168.0.20:3000/";

/*************************** Variaveis globais *******************************/

unsigned long previousMillis = 0;
char temp[4];

/************************ Declaração dos Prototypes **************************/

void initSerial();
void initWiFi();
void initMCP9808();
void readTemperature();
void sendTemperature();

/************************ Instanciação dos objetos  **************************/

Adafruit_MCP9808 mcp9808 = Adafruit_MCP9808();
WiFiClient client;
HTTPClient http;

/********************************* Sketch ************************************/

void setup() {
  initSerial();
  initWiFi();
  initMCP9808();
}

void loop() {
  recconectWiFi();
  sensorLoop();
}

/*********************** Implementação dos Prototypes *************************/

void readTemperature()
{
  mcp9808.shutdown_wake(0);
  float c = mcp9808.readTempC();
  dtostrf(c, 2, 2, temp);
  delay(250);
  mcp9808.shutdown_wake(1);
}

void sendTemperature()
{
  http.begin(BASE_URL + "/sensor");
  http.addHeader("content-type", "application/x-www-form-urlencoded");

  String body = "";
  body += "id=";
  body += "01";
  body += "value=";
  body += temp;

  int httpCode = http.POST(body);

  if (httpCode < 0) {
    Serial.println("request error - " + httpCode);
    return;
  }

  if (httpCode != HTTP_CODE_OK) {
    Serial.println("request error - " + httpCode);
    return;
  }

  String response =  http.getString();
  Serial.println("response - " + response);

  http.end();
}

/* Função responsável por publicar a cada X segundos o valor do sensor */
void sensorLoop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > 1000) {
    previousMillis = currentMillis;

    readTemperature();
    sendTemperature();
  }
}

/* Conexao Serial */
void initSerial() {
  Serial.begin(115200);
}

/* Configuração da conexão WiFi */
void initWiFi() {
  delay(10);
  Serial.println("Conectando-se em: " + String(SSID));

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado na Rede " + String(SSID) + " | IP => ");
  Serial.println(WiFi.localIP());
}

/* Inicialização do Sensor */
void initMCP9808() {
  if (!mcp9808.begin()) {
    Serial.println("Sensor MCP não pode ser iniciado!");
    while (1);
  }
}

/* Demais implementações */

void recconectWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
}
