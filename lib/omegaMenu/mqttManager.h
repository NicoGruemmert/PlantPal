#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> // Include ArduinoJson library

#define LED_PIN 15
#define MQTT_PASSWORD "49937025" // Define the MQTT password here

class WiFiMQTTManager {
public:
    WiFiMQTTManager(const char* ssid, const char* password, const char* mqtt_server, const char* mqtt_user, const char* mqtt_pass, const char* sensor_topic, const char* config_topic)
        : ssid(ssid), password(password), mqtt_server(mqtt_server), mqtt_user(mqtt_user), mqtt_pass(mqtt_pass), sensor_topic(sensor_topic), config_topic(config_topic), client(espClient) {
        uint8_t hi = (uint8_t)'P';
        id = (hi << 8) | (counter & 0xFF);
        counter++;
    }

    void setup() {
        setup_wifi();
        client.setServer(mqtt_server, 1883);
        client.setCallback([this](char* topic, byte* payload, unsigned int length) { this->callback(topic, payload, length); });
    }

    void loop() {
        if (!client.connected()) {
            reconnect();
        }
        client.loop();

        // Publish sensor data every 10 seconds
        static unsigned long lastPublishTime = 0;
        if (millis() - lastPublishTime > 10000) {
            publishSensorData();
            lastPublishTime = millis();
        }
    }

private:
    const char* ssid;
    const char* password;
    const char* mqtt_server;
    const char* mqtt_user;
    const char* mqtt_pass;
    const char* sensor_topic;
    const char* config_topic;
    uint16_t id;
    static uint8_t counter;
    WiFiClient espClient;
    PubSubClient client;

    void setup_wifi() {
        delay(10);
        pinMode(LED_PIN, OUTPUT);
        digitalWrite(LED_PIN, LOW);

        // Start connecting to Wi-Fi
        WiFi.begin(ssid, password);

        uint16_t counter = 0;
        while (WiFi.status() != WL_CONNECTED && counter < 5000) {
            counter++;
            if (counter % 10 == 0) {
                digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            }
            delay(1);
        }

        if (WiFi.status() == WL_CONNECTED) {
            digitalWrite(LED_PIN, HIGH);
    
        } else {
            Serial.println("Failed to connect to WiFi");
        }
    }

    void reconnect() {
        // Loop until we're reconnected
        while (!client.connected()) {
            Serial.print("Attempting MQTT connection...");
            // Attempt to connect
            if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
                Serial.println("connected");
                // Once connected, publish an "alive" message
                client.publish("/plantpal/alive", "ESP32 is alive");
                // Subscribe to the configuration topic
                client.subscribe(config_topic);
            } else {
                Serial.print("failed, rc=");
                Serial.print(client.state());
                Serial.println(" try again in 5 seconds");
                // Wait 5 seconds before retrying
                delay(5000);
            }
        }
    }

    void publishSensorData() {
        StaticJsonDocument<256> doc;
        // Assuming you have functions to get the actual sensor values
        float temperature = 25.6;  // Replace with actual function
        float humidity = 60.2;     // Replace with actual function
        int soilMoisture = 70;     // Replace with actual function
        doc["id"] = id;
        doc["temperature"] = round(temperature);
        doc["humidity"] = round(humidity);
        doc["soilMoisture"] = soilMoisture;
        doc["freeHeap"] = esp_get_free_heap_size();

        char jsonBuffer[256];
        serializeJson(doc, jsonBuffer);

        client.publish(sensor_topic, jsonBuffer);
    }

    void callback(char* topic, byte* payload, unsigned int length) {
        Serial.print("Message arrived [");
        Serial.print(topic);
        Serial.print("] ");
        for (unsigned int i = 0; i < length; i++) {
            Serial.print((char)payload[i]);
        }
        Serial.println();
        // Handle the configuration message if needed
    }
};

uint8_t WiFiMQTTManager::counter = 1;

const char* ssid = "OMEGA";  // Replace with your Wi-Fi SSID
const char* password = "49937025"; // Replace with your Wi-Fi password
const char* mqtt_server = "192.168.2.5";
const char* mqtt_user = "containership";
const char* mqtt_pass = MQTT_PASSWORD;
const char* sensor_topic = "/plantpal/sensors";
const char* config_topic = "/plantpal/config";


