#ifndef MENU_H
#define MENU_H

#include <WiFi.h>
#include "esp_wifi.h"
#include <esp_now.h>

#include <vector>
#include <algorithm> // Include for std::clamp

#include <omegaTFT.h>
#include <omegaWireless.h>
#include <omegaNOW.h>
#include <omegaPlant.h>

#include "NotoSansMonoSCB20.h"
#include "NotoSansBold15.h"
#include "mqttManager.h"

volatile int gyroy = 0;
volatile int gyrop = 0;
volatile int gyror = 0;
uint8_t *test;
omegaWireless wirelessManager = omegaWireless("PlantPal");
EspNowManager espNOW;
PlantProfile newProfile; 

omegaPlant myPlant = omegaPlant(newProfile);

extern sensorDataPacket curData;
extern PlantSaveData curSave;
 bool espNowActive= false;

TFT_eSPI tft =TFT_eSPI();  // Create object "tft"
TFT_eSprite menuSprite(&tft);

struct Arc {
    int cx;
    int cy;
    int radius;
    int width;
    int startAngle;
    int endAngle;
    uint16_t color;
    int value;
    char unit;
    int min;
    int max;
};

template<typename T>
T clamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

std::vector<MacAddress> connectedMacAddresses;
/*########################################## Menu Functions ##################################################*/

std::vector<MacAddress> getConnectedMacAddresses() {
    std::vector<MacAddress> macAddresses;
  // Get the number of connected stations
  int numStations = WiFi.softAPgetStationNum();
  Serial.print("Number of connected devices: ");
  Serial.println(numStations);

  // Get the list of connected stations
  wifi_sta_list_t stationList;
  tcpip_adapter_sta_list_t adapterList;
 

  if (esp_wifi_ap_get_sta_list(&stationList) == ESP_OK) {
    if (tcpip_adapter_get_sta_list(&stationList, &adapterList) == ESP_OK) {
      for (int i = 0; i < adapterList.num; i++) {

        tcpip_adapter_sta_info_t station = adapterList.sta[i];
        Serial.print("Device ");
        Serial.print(i + 1);
        Serial.print(": MAC Address: ");
        Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
                      station.mac[0], station.mac[1], station.mac[2],
                      station.mac[3], station.mac[4], station.mac[5]);
        MacAddress macAddress;
        memcpy(macAddress.mac, adapterList.sta[i].mac, sizeof(macAddress.mac)); // Copy MAC address
        macAddresses.push_back(macAddress); // Add MAC address to vector
      }
    }
  }
  return macAddresses;
}

// Function to animate the avatar on the display
void animateAvatar(TFT_eSPI* tft, TFT_eSprite* avatarSprite, uint8_t frame) {
    TFT_eSprite itemSprite = TFT_eSprite(tft);
    itemSprite.createSprite(50, 10);
    itemSprite.pushImage(0, 0, 50, 10, icon_sunglasses);

    if (frame > 3) return; // Exit if the frame is out of the valid range

    uint8_t avatarInt = curData.hum % 3;
    avatarSprite->pushImage(0, 0, 120, 120, plantArray[avatarInt]);
    itemSprite.pushToSprite(avatarSprite, 40 + frame * 4, 30 - frame * 3, 0);
}

// Function to create an arc with given properties
void createArc(TFT_eSprite* arcSprite, Arc arcData) {
    arcSprite->fillScreen(0);
    arcSprite->drawSmoothArc(16, 16, arcData.radius, arcData.width, arcData.startAngle, arcData.endAngle+1, arcData.color, TFT_TRANSPARENT, true);
    arcSprite->drawFastVLine(16, 0, 7, TFT_WHITE);
    arcSprite->setCursor(5, 28);
    arcSprite->setTextColor(arcData.color);
    arcSprite->printf("%d", arcData.value);
    arcSprite->print(arcData.unit);
}

// Function to calculate the positions of the arcs on the screen
void calculate_arc_positions(Arc arcs[], int numArcs) {
    const int displayRadius = 120;
    const int arcRadius = 16;
    const int centerOffset = 30;
    const int arcWidth = 14;
    const uint16_t arcColor = TFT_GOLD;
    const uint8_t startAngle = 180 + 45;
    const uint8_t endAngle = 360 - startAngle;

    float stepAngle = (endAngle - startAngle) / (numArcs - 1);

    for (int i = 0; i < numArcs; ++i) {
        float angle = startAngle + i * stepAngle;
        float radians = (angle - 45) * PI / 180.0;

        int cx = displayRadius + (displayRadius - centerOffset) * cos(radians);
        int cy = displayRadius + (displayRadius - centerOffset) * sin(radians);

        arcs[i] = {cx, cy, arcRadius, arcWidth, 45, 0, 0,0,0,0};
    }
}

// Function to draw the home screen, including updating various sprites and animations
bool drawHomeScreen(TFT_eSPI* tft, TFT_eSprite* mainSprite) {
    static int frameCounter = 0;
    static bool spritesInitialized = false;
    static uint8_t animationIndex = 0;
    static TFT_eSprite txtSprite = TFT_eSprite(tft);
    static TFT_eSprite avatarSprite = TFT_eSprite(tft);
    static TFT_eSprite arcSprite = TFT_eSprite(tft);
    static TFT_eSprite iconSprite = TFT_eSprite(tft);

    if (!spritesInitialized) {
        txtSprite.createSprite(180, 35);
        txtSprite.loadFont(NotoSansBold15);

        arcSprite.createSprite(34, 44);
        arcSprite.loadFont(NotoSansBold15);

        iconSprite.createSprite(40, 40);
        avatarSprite.createSprite(120, 120, 1);
        spritesInitialized = true;
    }

    frameCounter = frameCounter % 100;
    if (frameCounter < 20) animationIndex = 0;
    else if (frameCounter < 21) animationIndex = 1;
    else if (frameCounter < 23) animationIndex = 2;
    else if (frameCounter < 25) animationIndex = 1;
    else animationIndex = 0;

    uint16_t offset = 50;
    uint16_t startAngl = 45;
    uint16_t endAngl = 315;

    Arc valArcs[4];
    calculate_arc_positions(valArcs, 4);

    uint8_t tempc = clamp<uint8_t>(curData.tempc,curProfile.tempc - curProfile.range_temp,curProfile.tempc + curProfile.range_temp);
    uint8_t hum =   clamp<uint8_t>(curData.hum, (curProfile.hum - curProfile.range_hum),(curProfile.hum + curProfile.range_hum));
    uint8_t light = clamp<uint8_t>(curData.light, (curProfile.light - curProfile.range_light),(curProfile.light + curProfile.range_light));
    uint8_t moist = clamp<uint8_t>(curData.moist,(curProfile.soil_moisture - curProfile.range_soil_moisture), (curProfile.soil_moisture + curProfile.range_soil_moisture));

uint16_t tempc_angle = clamp<uint16_t>(map(tempc, (curProfile.tempc - curProfile.range_temp), (curProfile.tempc + curProfile.range_temp), startAngl, endAngl), startAngl, endAngl);
uint16_t hum_angle = clamp<uint16_t>(map(hum, (curProfile.hum - curProfile.range_hum), (curProfile.hum + curProfile.range_hum), startAngl, endAngl), startAngl, endAngl);
uint16_t light_angle = clamp<uint16_t>(map(light, (curProfile.light - curProfile.range_light), (curProfile.light + curProfile.range_light), startAngl, endAngl), startAngl, endAngl);
uint16_t soil_angle = clamp<uint16_t>(map(moist, (curProfile.soil_moisture - curProfile.range_soil_moisture), (curProfile.soil_moisture + curProfile.range_soil_moisture), startAngl, endAngl), startAngl, endAngl);


    valArcs[0].endAngle = light_angle;
    valArcs[1].endAngle = tempc_angle;
    valArcs[2].endAngle = hum_angle;
    valArcs[3].endAngle = soil_angle;


    valArcs[0].value = curData.light;
    valArcs[1].value = curData.tempc;
    valArcs[2].value = curData.hum;
    valArcs[3].value = curData.moist;

    valArcs[0].color = COLOR_YELLOW;
    valArcs[1].color = COLOR_RED;
    valArcs[2].color = COLOR_PURPLE;
    valArcs[3].color = COLOR_BROWN;

    mainSprite->setSwapBytes(1);
    mainSprite->fillScreen(0);

    txtSprite.fillScreen(TFT_BLACK);
    txtSprite.setCursor(0, 0);
    txtSprite.setTextColor(TFT_WHITE);

    uint8_t level = myPlant.calculateLevel(curData.xp);
    uint8_t xp_progress = abs(curData.xp - (level - 1) * 2);

    txtSprite.setTextColor(0x3F29);
    txtSprite.printf("Mood\n  %d%%", curData.mood);
    txtSprite.setSwapBytes(1);
    txtSprite.setTextColor(TFT_WHITE);
    txtSprite.pushToSprite(mainSprite, 155, 180);

    txtSprite.fillScreen(0);
    txtSprite.setTextColor(TFT_WHITE);
    txtSprite.setCursor(0, 0);
    txtSprite.setTextColor(TFT_SILVER);
    txtSprite.printf("Level\n    %d\n", level);
    txtSprite.pushToSprite(mainSprite, 180, 90);

    txtSprite.fillScreen(0);
    txtSprite.setTextColor(TFT_GOLD);
    txtSprite.setCursor(0, 0);
    txtSprite.printf(" %dXP", xp_progress);
    txtSprite.pushToSprite(mainSprite, 180, 125);

    uint16_t mood_angle = map(curData.mood, -1, 101, 360 - offset, 180 + offset);
    uint16_t xp_angle = map(xp_progress, 0, level * 2, 360 - offset, 180 + offset);

    mood_angle = clamp<uint16_t>(mood_angle, 180 + offset, (uint16_t)(360 - offset));
    xp_angle = clamp<uint16_t>(xp_angle, 180 + offset, (uint16_t)(360 - offset));

    mainSprite->drawSmoothArc(120, 120, 118, 110, mood_angle-1, 360 - offset, 0x3F29, TFT_TRANSPARENT, true);
    mainSprite->drawSmoothArc(120, 120, 106, 102, xp_angle-1, 360 - offset, TFT_GOLD, TFT_TRANSPARENT, true);
    mainSprite->drawFastHLine(190, 120, 40, TFT_WHITE);

    int iPlant = (frameCounter / (100 / plantArrLen)) % plantArrLen;
    mainSprite->setCursor(80, 10);
    char msb = (char)((curData.id >> 8) & 0xFF);
    char lsb = (char)(curData.id & 0xFF);
    mainSprite->printf("Plant %c%d", msb, lsb);

    animateAvatar(tft, &avatarSprite, animationIndex);
    avatarSprite.pushToSprite(mainSprite, 60, 60, TFT_BLACK);

    for (int i = 0; i < 4; i++) {
        createArc(&arcSprite, valArcs[i]);
        arcSprite.pushToSprite(mainSprite, valArcs[i].cx - 16, valArcs[i].cy - 16);
    }

    if (frameCounter < 50 || frameCounter > 75) {
        mainSprite->fillCircle(120, 160, 5, TFT_SILVER);
        mainSprite->fillCircle(120, 160, 4, COLOR_BROWN);

        mainSprite->fillCircle(80, 145, 5, TFT_SILVER);
        mainSprite->fillCircle(80, 145, 4, COLOR_PURPLE);

        mainSprite->fillCircle(65, 105, 5, TFT_SILVER);
        mainSprite->fillCircle(65, 105, 4, COLOR_RED);

        mainSprite->fillCircle(85, 65, 5, TFT_SILVER);
        mainSprite->fillCircle(85, 65, 4, COLOR_YELLOW);
    } else {
        if (curData.emotion > 0) {
            mainSprite->fillCircle(80, 86, 2, TFT_WHITE);
            if (frameCounter > 55) mainSprite->fillCircle(70, 80, 5, TFT_WHITE);
            if (frameCounter > 58) mainSprite->fillCircle(40, 70, 22, TFT_WHITE);
            if (frameCounter > 62) {
              if(curData.emotion == TOO_DARK) 
                iconSprite.pushImage(0, 0, 40, 40, icon_light);
              if(curData.emotion == TOO_COLD) 
                iconSprite.pushImage(0, 0, 40, 40, icon_eye);


              iconSprite.pushToSprite(mainSprite, 20, 50, TFT_BLACK);
            }
        }
    }

    mainSprite->pushSprite(0, 0);
    frameCounter++;

    return false; // Stay in the menu
}

bool myFunc(TFT_eSPI* tft,TFT_eSprite * mainSprite)
{
   
  uint16_t angly = map(gyroy,-18000, 18000,41,319 );
  uint16_t anglp = map(gyrop,-18000, 18000,41,319 );
  uint16_t anglr = map(gyror,-18000, 18000,41,319 );
  
  Serial.print(angly/100);

  mainSprite->fillScreen(TFT_BLACK);
  mainSprite->loadFont(NotoSansMonoSCB20);

  mainSprite->setCursor(80,100);
  mainSprite->setTextColor(TFT_GOLD);
  mainSprite->print("Yaw: ");
  mainSprite->print(gyroy/100);

  mainSprite->setCursor(80,120);
  mainSprite->setTextColor(TFT_SILVER);
  mainSprite->print("Pitch: ");
  mainSprite->print(gyrop/100);
  
  mainSprite->setCursor(80,140);
  mainSprite->setTextColor(TFT_VIOLET);
  mainSprite->print("Roll: ");
  mainSprite->print(gyror/100);
  

  mainSprite->drawSmoothArc(120,120,118,114,40,angly,TFT_GOLD,TFT_TRANSPARENT,true);
  mainSprite->drawSmoothArc(120,120,110,106,40,anglp,TFT_SILVER,TFT_TRANSPARENT,true);
  mainSprite->drawSmoothArc(120,120,102,98,40,anglr,TFT_VIOLET,TFT_TRANSPARENT,true);
  
  mainSprite->pushSprite(0,0);
  return 0; // Bleibe im Menu
}

bool startAP(TFT_eSPI* tft,TFT_eSprite * mainSprite)
{
 wirelessManager.startAP();
  return true;
}

bool getMAC(TFT_eSPI* tft,TFT_eSprite * mainSprite)
{
  wirelessManager.getConnectedMacAddresses();
  return true;
}

bool stopAP(TFT_eSPI* tft,TFT_eSprite * mainSprite)
{
  wirelessManager.stopAP();
  return true;
}

void sendData()
{
//wirelessManager.sendESPNOW();


}

bool restartESP(TFT_eSPI* tft,TFT_eSprite * mainSprite)
{
  TFT_eSprite txtSprite = TFT_eSprite(tft);//120x120
  
  txtSprite.createSprite(180,120);
  mainSprite->fillScreen(TFT_BLACK);
  

  txtSprite.fillScreen(TFT_BLACK);
  txtSprite.setTextColor(TFT_RED);
  txtSprite.setCursor(0,0);
  txtSprite.print("Restarting System...");
  txtSprite.pushToSprite(mainSprite,60,100);
  mainSprite->pushSprite(0,0);



  ESP.restart();
  return 0;

}



/*########################################## Menu Structure ##################################################*/
char* macToString(const uint8_t* mac) {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return macStr;
}


std::vector<omegaTFT> macMenu(void)
{
  Serial.println("MACMENU");

  std::vector<omegaTFT> items;
  items.push_back(omegaTFT(EXIT, "Back",icon_cross));

  std::vector<MacAddress> macList = wirelessManager.getConnectedMacAddresses();
  uint8_t i = 0; 
      for (const MacAddress&mac : macList) {
        items.push_back(omegaTFT(VALUE,"Device:",icon_numeric,macList[i].mac[5]));
        i++;

    }
    
 return items;

}




omegaTFT plantParams[]{
omegaTFT(VALUE,"Temperature", curProfile.tempc),
omegaTFT(VALUE,"Humidity", curProfile.hum),
omegaTFT(VALUE,"Soil",curProfile.soil_moisture),
omegaTFT(VALUE,"Light", curProfile.light),

omegaTFT(VALUE,"Temp Range", curProfile.range_temp),
omegaTFT(VALUE,"Hum Range", curProfile.range_hum),
omegaTFT(VALUE,"Soil Range",curProfile.range_soil_moisture),
omegaTFT(VALUE,"Light Range", curProfile.range_light),

omegaTFT(EXIT,"Back")

};

omegaTFT plantMenu[]{
omegaTFT(SUBMENU,"Plant 1", icon_potted_plant,plantParams,sizeof(plantParams)/sizeof(omegaTFT)),
omegaTFT(SUBMENU,"Plant 2", icon_potted_plant,plantParams,sizeof(plantParams)/sizeof(omegaTFT)),
omegaTFT(EXIT,"Back")

};


omegaTFT wifiMenu[]{
omegaTFT(FUNCTION,"Start AP", icon_satellite_antenna,startAP),
omegaTFT(FUNCTION,"Stop AP", icon_satellite_antenna,stopAP),
omegaTFT(MENU_FUNCTION,"Get MAC",icon_satellite_antenna,macMenu),
omegaTFT(EXIT,"Back")


};

omegaTFT settingsMenu [] = {
  omegaTFT(SUBMENU,"Plant Profiles", icon_potted_plant,plantMenu,sizeof(plantMenu)/sizeof(omegaTFT)),
  omegaTFT(SUBMENU,"WiFi", icon_wifi,wifiMenu,sizeof(wifiMenu)/sizeof(omegaTFT)),
  omegaTFT(EMPTY,"BLE", icon_bluetooth),
  omegaTFT(EXIT,"Back")
};

omegaTFT submenus [] = {
  omegaTFT(FUNCTION, "Home Screen",icon_potted_plant,drawHomeScreen),
  omegaTFT(SUBMENU, "Settings",icon_gear,settingsMenu, sizeof(settingsMenu)/sizeof(omegaTFT)),
  omegaTFT(FUNCTION,"Restart Device",icon_cross,restartESP)
};

omegaTFT myMenu = omegaTFT(SUBMENU,"Main Menu",nullptr,submenus,sizeof(submenus)/sizeof(omegaTFT));

#endif