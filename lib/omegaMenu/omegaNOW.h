#include <WiFi.h>
#include "esp_wifi.h"
#include <esp_now.h>
#include <vector>

class EspNowManager {
public:
    EspNowManager() {}

    void begin() {
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        if (esp_now_init() != ESP_OK) {
            ESP.restart();
        }
        esp_now_register_recv_cb(receiveCallback);
        esp_now_register_send_cb(sentCallback);
    }

    void sendMessage(const String &message) {
        uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        esp_now_peer_info_t peerInfo = {};
        memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
        if (!esp_now_is_peer_exist(broadcastAddress)) {
            esp_now_add_peer(&peerInfo);
        }
        esp_now_send(broadcastAddress, (const uint8_t *)message.c_str(), message.length());
    }

private:
    static void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen) {
                

        
        char buffer[ESP_NOW_MAX_DATA_LEN + 1];
        int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
        strncpy(buffer, (const char *)data, msgLen);
        buffer[msgLen] = 0;
        onMessageReceived(buffer);
    }

    static void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status) {
        onMessageSent(status == ESP_NOW_SEND_SUCCESS);
    }

    static void onMessageReceived(const char *message) {
        // Handle the received message
        digitalWrite(15,1);
        delay(50);
        digitalWrite(15,0);
    }

    static void onMessageSent(bool success) {
        // Handle the send status
    }
};

