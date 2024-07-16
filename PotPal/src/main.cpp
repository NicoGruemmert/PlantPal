/**
 * @file main.cpp
 * @brief Main file for the PotPal Sensor System
 * 
 * This file contains the setup and loop functions for the Plant Monitoring System,
 * which reads sensor data, updates plant state, and publishes data to an MQTT server.
 * 
 * @custom_headers
 *  - omegaPlant.h
 * 
 * @external_headers
 *  - Arduino.h
 *  - WiFi.h
 *  - PubSubClient.h
 *  - Adafruit_VEML6075.h
 *  - Adafruit_Sensor.h
 *  - Adafruit_BME280.h
 *  - ArduinoJson.h
 *  - Wire.h
 *  - BH1750.h
 * 
 * @authors
 *  - Nico Grümmert
 *  
 * 
 * @date 2024-07-14
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_VEML6075.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <BH1750.h>

/** Modeling of a Plant using a C++ Class
 *  Created by Group 1
 */
#include <omegaPlant.h>

/** WiFi and MQTT setup */
#define LED_PIN 15
#define MQTT_PASSWORD "49937025"
#define WIFI_PASSWORD "49937025"
#define SSID "OMEGA"

const char *ssid = "OMEGA";
const char *password = WIFI_PASSWORD;
const char *mqtt_server = "192.168.2.5";
const char *mqtt_user = "containership";
const char *mqtt_pass = MQTT_PASSWORD;
const char *sensor_topic = "plantpal/sensor";
const char *alive_topic = "plantpal/alive";
const char *state_topic = "plantpal/state";
const char *friendly_name = "PotPal8A";

WiFiClient espClient;
PubSubClient client(espClient);

#define PUBLISH_INTERVAL 5000 // Publish interval in milliseconds

/** I2C Pins */
#define I2C_SDA 21
#define I2C_SCL 22

/** Sensor activation */
#define USE_DUMMY

#ifndef USE_DUMMY
  #define USE_VEML6075
  #define USE_BME280
  #define USE_BH1750
  #define USE_SOIL_MOISTURE
#endif

#ifdef USE_VEML6075
Adafruit_VEML6075 uv = Adafruit_VEML6075();
#endif

#ifdef USE_BME280
Adafruit_BME280 bme; // use I2C interface
#endif

#ifdef USE_BH1750
BH1750 lightMeter;
#endif

#ifdef USE_SOIL_MOISTURE
#define SOIL_MOISTURE_PIN 34
#endif

PlantProfile profile;
omegaPlant myPlant(profile);

unsigned long lastPublishTime = 0;

/**
 * @brief Connect to WiFi
 */
void setup_wifi() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.begin(ssid, password);
  uint16_t counter = 0;
  while (WiFi.status() != WL_CONNECTED && counter < 5000) {
    counter++;
    if (counter % 10 == 0) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Blink LED while connecting
    }
    delay(1);
  }
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_PIN, HIGH); // Turn LED on when connected
  } else {
    digitalWrite(LED_PIN, LOW); // Turn LED off if connection failed
  }
}

/**
 * @brief Reconnect to MQTT server if disconnected
 */
void reconnect() {
  while (!client.connected()) {
    digitalWrite(LED_PIN, HIGH); // Turn LED on while attempting to reconnect
    delay(100);
    digitalWrite(LED_PIN, LOW); // Blink LED while reconnecting
    delay(100);
    if (client.connect(friendly_name, mqtt_user, mqtt_pass)) {
      digitalWrite(LED_PIN, HIGH); // Turn LED on when connected
      client.publish(alive_topic, friendly_name);
    } else {
      delay(5000); // Wait 5 seconds before retrying
    }
  }
}

/**
 * @brief Get sensor data and store it in theData
 * @param theData Pointer to sensorData struct to store the readings
 */
void getSensorData(sensorData *theData) {
  #ifdef USE_VEML6075
  theData->lightIntensity = uv.readUVA();
  #endif

  #ifdef USE_BME280
  bme.takeForcedMeasurement();
  theData->temperature = bme.readTemperature();
  theData->humidity = bme.readHumidity();
  #endif

  #ifdef USE_BH1750
  theData->lightIntensity = lightMeter.readLightLevel();
  #endif

  #ifdef USE_SOIL_MOISTURE
  theData->moisture = analogRead(SOIL_MOISTURE_PIN);
  #endif

  #ifdef USE_DUMMY
    theData->moisture = 50 + random(-25,25);
    theData->temperature = 20 + random(-10,10);
    theData->humidity = 50 + random(-25,25);
    theData->lightIntensity = 50 + random(-25,25);
  #endif
}

/**
 * @brief Publish sensor data to MQTT server
 * @param newData The sensor data to be published
 */
void publishSensorData(sensorData newData) {
  StaticJsonDocument<256> doc;
  static PlantSaveData currentPlant;

  static uint8_t lastLevel = 0;

  plantState state = myPlant.getMeasurement(newData, &currentPlant);

  doc["id"] = currentPlant.plantID;
  doc["light"] = state.curData.lightIntensity;
  doc["tempc"] = state.curData.temperature;
  doc["hum"] = state.curData.humidity;
  doc["soilm"] = state.curData.moisture;

  doc["mood"] = state.curMood;
  doc["xp"] = state.curXP;
  doc["level"] = currentPlant.savedLvL;
  doc["rcmnd"] = state.curEmotion;

  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);
  client.publish(sensor_topic, jsonBuffer);

  if (lastLevel != currentPlant.savedLvL) {
    char jBuffer[256];
    StaticJsonDocument<256> lvlupMSG;

    lastLevel = currentPlant.savedLvL;
   
    lvlupMSG["id"] = currentPlant.plantID;
    lvlupMSG["level"] = currentPlant.savedLvL;
    lvlupMSG["xp"] = currentPlant.savedExp;
    lvlupMSG["items"] = currentPlant.unlockedItems;

    serializeJson(lvlupMSG, jBuffer);
    client.publish(state_topic, jBuffer);
  }
}

/**
 * @brief Setup function to initialize sensors and WiFi connection
 */
void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  delay(1000);

  Wire.begin(I2C_SDA, I2C_SCL);

  #ifdef USE_VEML6075
  if (!uv.begin()) {
    while (1); // Halt if VEML6075 is not found
  }
  uv.setHighDynamic(true);
  uv.setCoefficients(2.22, 1.33, 2.95, 1.74, 0.001461, 0.002591);
  #endif

  #ifdef USE_BME280
  if (!bme.begin(0x76, &Wire)) {
    while (1) { // Halt if BME280 is not found
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      delay(50);
    }
  }
  #endif

  #ifdef USE_BH1750
  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire)) {
    while (1); // Halt if BH1750 is not found
  }
  #endif

  setup_wifi();
  client.setKeepAlive(60);
  client.setServer(mqtt_server, 1883);
}

/**
 * @brief Main loop function to handle MQTT connection and publish sensor data
 */
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - lastPublishTime > PUBLISH_INTERVAL) {
    sensorData currentData;
    getSensorData(&currentData);
    publishSensorData(currentData);
    lastPublishTime = millis();
  }
  delay(100);
}
