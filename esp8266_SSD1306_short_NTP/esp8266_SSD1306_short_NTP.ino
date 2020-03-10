/**
    Required libraries:
      - Adafruit BME280 Library
      - Adafruit Unified Sensor
      - PubSubClient
**/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerif9pt7b.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


#define MQTT_TOPIC_HUMIDITY "home/schuppen/humidity"
#define MQTT_TOPIC_TEMPERATURE "home/schuppen/temperature"
#define MQTT_TOPIC_PRESSURE "home/schuppen/pressure"
#define MQTT_TOPIC_STATE "home/schuppen/status"
#define MQTT_PUBLISH_DELAY 60000
#define MQTT_CLIENT_ID "sensor-schuppen"

#define BME280_ADDRESS 0x76

const char *WIFI_SSID = "Two Girls - One Router";
const char *WIFI_PASSWORD = "94809730825940089603";
WiFiUDP ntpUDP;

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
char daysOfTheWeek[7][12] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};
char hours[24][60] = {"00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23"};
char minutes[60][60] = {"00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "50", "51", "52", "53", "54", "55", "56", "57", "58", "59"};

const char *MQTT_SERVER = "192.168.2.252";
const char *MQTT_USER = "mqttuser"; // NULL for no authentication
const char *MQTT_PASSWORD = "mqttpassword"; // NULL for no authentication

float humidity;
float temperature;
float pressure;
long lastMsgTime = 0;

Adafruit_BME280 bme;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup() {
  Serial.begin(115200);
  while (! Serial);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while (1);
  }

  if (!bme.begin(BME280_ADDRESS)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring or BME-280 address!");
    while (1);
  }

  //display.setFont(&FreeSerif9pt7b);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Use force mode so that the sensor returns to sleep mode when the measurement is finished
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1, // temperature
                  Adafruit_BME280::SAMPLING_NONE, // pressure
                  Adafruit_BME280::SAMPLING_X1, // humidity
                  Adafruit_BME280::FILTER_OFF);

  setupWifi();
  mqttClient.setServer(MQTT_SERVER, 1883);
  timeClient.begin();
}

void loop() {
  timeClient.update();
  if (!mqttClient.connected()) {
    mqttReconnect();
  }
  mqttClient.loop();

  long now = millis();
  if (now - lastMsgTime > MQTT_PUBLISH_DELAY) {
    lastMsgTime = now;

    // Reading BME280 sensor data
    bme.takeForcedMeasurement(); // has no effect in normal mode
    humidity = bme.readHumidity();
    temperature = bme.readTemperature();
    pressure = bme.readPressure();
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("BME280 reading issues");
      return;
    }
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(2);
    //    display.println("Schuppen");
    //   display.println(timeClient.getFormattedTime());
    Serial.print(daysOfTheWeek[timeClient.getDay()]);
    Serial.print(", ");
    Serial.println(timeClient.getFormattedTime());
    Serial.print(hours[timeClient.getHours()]);
    Serial.print(":");
    Serial.println(minutes[timeClient.getMinutes()]);
    Serial.print(timeClient.getHours());
    Serial.print(":");
    Serial.println(timeClient.getMinutes());

    //Uhrzeit Anzeigen
    display.print(" ");
    display.print(daysOfTheWeek[timeClient.getDay()]);
    display.print(", ");
    // display.print("  ");
    display.print(hours[timeClient.getHours()]);
    // display.print(timeClient.getHours());
    display.print(":");
    //  display.println(timeClient.getMinutes());
    display.println(minutes[timeClient.getMinutes()]);
    //display.print("T:");
    display.setTextSize(3);
    display.print(" ");
    display.print(temperature);
    display.println("C");
    //display.setTextSize(1);
    //display.println("");
    //display.println("");
    //display.print("H:");
    display.print(" ");
    display.setTextSize(3);
    display.print(humidity);
    display.println("%");
    display.display();
    // Publishing sensor data
    mqttPublish(MQTT_TOPIC_TEMPERATURE, temperature);
    mqttPublish(MQTT_TOPIC_HUMIDITY, humidity);
  }
}

void setupWifi() {
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqttReconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_STATE, 1, true, "disconnected", false)) {
      Serial.println("connected");

      // Once connected, publish an announcement...
      mqttClient.publish(MQTT_TOPIC_STATE, "connected", true);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqttPublish(char *topic, float payload) {
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(payload);

  mqttClient.publish(topic, String(payload).c_str(), true);
}
