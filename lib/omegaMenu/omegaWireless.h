#ifndef OMEGAWIRELESS_H
#define OMEGAWIRELESS_H

#include <WiFi.h>
#include "esp_wifi.h"
#include <esp_now.h>
#include <vector>

#define MYSSID "PlantPal-S2"
#define PWD "78847884"

struct MacAddress
{
  uint8_t mac[6];
};

enum omegaWirelessSate
{
  W_STARTING,
  W_IDLE,
  W_AP,
  W_ESPNOW,
  W_SLEEP

};
typedef struct struct_message
{
  uint8_t id;
} struct_message;

/*######################### CallBacks ####################################*/

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  if (status == ESP_NOW_SEND_SUCCESS)
  {
    Serial.println("Data sent successfully");
  }
  else
  {
    Serial.println("Data failed to send");
  }
}

class omegaWireless
{
private:
  omegaWirelessSate currentState;
  omegaWirelessSate prevState;
  const uint8_t *myMacAdd;
  char *friendlyName;
  std::vector<MacAddress> activeClients;
  std::vector<MacAddress> lastClients;

  uint8_t activeClientCount = 0;
  /* data */
public:
  void startAP();
  void stopAP();
  void connect(const char *, const char *);

  std::vector<MacAddress> getConnectedMacAddresses();
  std::vector<MacAddress> getLastDevices();

  void initESPNOW(bool);
  void sendESPNOW(const uint8_t *);
  void stopESPNOW();
  omegaWireless(){};
  omegaWireless(char *);
  ~omegaWireless();

  static void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen)
  {

    char buffer[ESP_NOW_MAX_DATA_LEN + 1];
    int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
    strncpy(buffer, (const char *)data, msgLen);
    buffer[msgLen] = 0;
    onMessageReceived(buffer);
  }

  static void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status)
  {
    onMessageSent(status == ESP_NOW_SEND_SUCCESS);
  }

  static void onMessageReceived(const char *message)
  {
    if(message[0]%5 == 0 ){
    // Handle the received message
    digitalWrite(15, 1);
    delay(50);
    digitalWrite(15, 0);
    }
  }

  static void onMessageSent(bool success)
  {
    // Handle the send status
  }
};

omegaWireless::omegaWireless(char *fName) : friendlyName(fName) {}

omegaWireless::~omegaWireless()
{
}

void omegaWireless::connect(const char *ssid, const char *password)
{

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while ((WiFi.status() != WL_CONNECTED))
  {
    delay(100);
    digitalWrite(15, !digitalRead(15));
  }
}

void omegaWireless::startAP()
{
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(MYSSID, PWD);

  prevState = currentState;
  currentState = W_AP;
}

void omegaWireless::stopAP()
{
  lastClients = getConnectedMacAddresses();
  WiFi.softAPdisconnect(true);
  WiFi.disconnect();

  prevState = currentState;
  currentState = W_IDLE;
}

std::vector<MacAddress> omegaWireless::getConnectedMacAddresses()
{
  std::vector<MacAddress> macAddresses;
  // Get the number of connected stations
  int numStations = WiFi.softAPgetStationNum();
  Serial.print("Number of connected devices: ");
  Serial.println(numStations);

  // Get the list of connected stations
  wifi_sta_list_t stationList;
  tcpip_adapter_sta_list_t adapterList;

  if (esp_wifi_ap_get_sta_list(&stationList) == ESP_OK)
  {
    if (tcpip_adapter_get_sta_list(&stationList, &adapterList) == ESP_OK)
    {
      for (int i = 0; i < adapterList.num; i++)
      {

        tcpip_adapter_sta_info_t station = adapterList.sta[i];
        Serial.print("Device ");
        Serial.print(i + 1);
        Serial.print(": MAC Address: ");
        Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
                      station.mac[0], station.mac[1], station.mac[2],
                      station.mac[3], station.mac[4], station.mac[5]);
        MacAddress macAddress;
        memcpy(macAddress.mac, adapterList.sta[i].mac, sizeof(macAddress.mac)); // Copy MAC address
        macAddresses.push_back(macAddress);                                     // Add MAC address to vector
      }
    }
  }
  return macAddresses;
}

std::vector<MacAddress> omegaWireless::getLastDevices()
{
  return lastClients;
}

void omegaWireless::initESPNOW(bool addClients)
{
  if (currentState != W_ESPNOW)
  {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

    if (esp_now_init() != ESP_OK)
    {
      Serial.println("Error initializing ESP-NOW");
    }

    esp_now_register_recv_cb(receiveCallback);
    esp_now_register_send_cb(sentCallback);
    if (addClients)
    {
      esp_now_del_peer(lastClients[0].mac);

      esp_now_peer_info_t peerInfo;
      memset(&peerInfo, 0, sizeof(peerInfo));
      peerInfo.channel = 0;     // Set the channel (0 for auto)
      peerInfo.encrypt = false; // No encryption
      // Replace this MAC address with the sender's MAC address
      Serial.println("Adding: ");
      Serial.println(lastClients[0].mac[5]);
      memcpy(peerInfo.peer_addr, lastClients[0].mac, 6);

      // Add peer
      if (esp_now_add_peer(&peerInfo) != ESP_OK)
      {
        Serial.println("Failed to add peer");
      }
    }
  }
  prevState = currentState;
  currentState = W_ESPNOW;
}

void omegaWireless::sendESPNOW(const uint8_t *rxMac)
{
  /*

    Serial.printf("Sending Data:\n%02X:%02X:%02X:%02X:%02X:%02X\n",
                        activeClients[0].mac[0],
                        activeClients[0].mac[1],
                        activeClients[0].mac[2],
                        activeClients[0].mac[3],
                        activeClients[0].mac[4],
                        activeClients[0].mac[5]);

         */

  static struct_message msg;
  msg.id++;
  esp_err_t result = esp_now_send(rxMac, (uint8_t *)&msg, sizeof(msg));

  if (result == ESP_OK)
  {
    Serial.println("Sending data...");
  }
  else
  {
    Serial.println("Failed to send data");
  }
}
#endif
