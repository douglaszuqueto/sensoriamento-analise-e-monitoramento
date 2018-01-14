#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

/* WIFI */

#define WIFI_SSID     "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASS"

IPAddress ip(192, 168, 0, 100);
IPAddress gw(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

/* MQTT */

const char* BROKER_MQTT = "192.168.0.10";
int BROKER_PORT         = 1883;

const char* MQTT_TOPIC_SENSOR     = "douglaszuqueto/casa_01/cozinha/temperatura/sensor_01";
const char* MQTT_TOPIC_STATISTICS = "douglaszuqueto/casa_01/cozinha/temperatura/sensor_01/statistics";

/* Intervals */

const long sensor_interval      = 5000;
const long statistics_interval  = 10000;

/* SENSOR */

#define DHT_SENSOR

#endif
