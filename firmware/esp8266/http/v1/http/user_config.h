#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

/* WIFI */

#define WIFI_SSID     "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASS"

IPAddress ip(192, 168, 0, 100);
IPAddress gw(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

/* HTTP */

const char* API_URL = "http://192.168.0.10:8081/";

/* SENSOR */

#define DHT_SENSOR

#endif
