/**
 * @file omegaPlant.h
 * @brief Header file for the omegaPlant class
 * 
 * This file contains the definitions and declarations for the omegaPlant class,
 * which is responsible for managing the plant state, sensor data, and interactions.
 * 
 * @author
 *  - Nico Grümmert
 *  
 * 
 * @date 2024-07-14
 */


#ifndef OMEGAPLANT_H
#define OMEGAPLANT_H

#include <Arduino.h>
#include <nvs.h>
#include <nvs_flash.h>

/** Settings */
#define MEASUREMENT_PER_XP_GAIN 24
#define MEASUREMENT_INERVAL_SEC 20
#define MAX_PLANTS 5
/** End Settings */

/**
 * @enum RCMD
 * @brief Recommendations based on sensor data
 */
enum RCMD {
    HAPPY = 0,
    TOO_HOT,
    TOO_COLD,
    TOO_HUMID,
    TOO_DRY,
    TOO_SUNNY,
    TOO_DARK,
    TOO_MOIST,
    TOO_ARID,
};

/**
 * @enum Unlockables
 * @brief Unlockable items for the plant
 */
enum Unlockables {
    ULCK_NONE = 0x1FFF,
    // Items:
    ULCK_SUNGLASSES = 1 << 0,
    ULCK_GLASSES = 1 << 1,
    ULCK_BALOON = 1 << 2,
    ULCK_BEARD = 1 << 3,
    ULCK_TIE = 1 << 4,
    ULCK_TIE2 = 1 << 5,
    ULCK_CROWN = 1 << 6,
    ULCK_HAT = 1 << 7,
    // Avatars:
    ULCK_DEFAULTPLANT = 1 << 8,
    ULCK_CACTUS1 = 1 << 9,
    ULCK_VASE1 = 1 << 10,
    // Backgrounds:
    ULCK_BG1 = 1 << 14,
    ULCK_BG2 = 1 << 15,
    ULCK_ALL = 0xFFFF
};

/**
 * @struct PlantProfile
 * @brief Profile settings for the plant
 */
struct PlantProfile {
    char name[16] = {'P', 'l', 'a', 'n', 't'};
    uint8_t tempc = 22;
    uint8_t hum = 60;
    uint8_t soil_moisture = 50;
    uint8_t light = 50;
    uint8_t range_temp = 5;
    uint8_t range_hum = 30;
    uint8_t range_light = 50;
    uint8_t range_soil_moisture = 50;
};

/**
 * @struct sensorData
 * @brief Data collected from sensors
 */
struct sensorData {
    uint8_t temperature;
    uint8_t humidity;
    uint8_t moisture;
    uint8_t lightIntensity;
};

/**
 * @struct plantState
 * @brief Current state of the plant
 */
struct plantState {
    uint8_t plantID;
    uint8_t curMood;
    uint8_t curXP;
    uint8_t curEmotion;
    sensorData curData;
};

/**
 * @struct PlantSaveData
 * @brief Data structure to save the plant state
 */
struct PlantSaveData {
    uint8_t plantID = 0;
    PlantProfile savedProfile;
    uint16_t savedExp = 0;
    uint16_t savedLvL = 0;
    uint16_t unlockedItems = 0;
    uint16_t unlockedBg = 0;
    uint16_t unlockedAvatar = 0;
};

/**
 * @class omegaPlant
 * @brief Class for managing the plant state and interactions
 */
class omegaPlant {
private:
    PlantSaveData myCurrentState;
    sensorData lastData[MEASUREMENT_PER_XP_GAIN]; // Save the last day's data
    uint8_t moodHistory[MEASUREMENT_PER_XP_GAIN];
    sensorData currentData;
    uint8_t plantID; // Memory location ID for managing save states
    uint8_t currentLevel;
    uint8_t myExp;

public:
    /** Constructor with plant profile */
    omegaPlant(PlantProfile extPlantProfile);

    /** Constructor with saved data */
    omegaPlant(PlantSaveData extSaveData);

    /** Constructor with index to load from memory */
    omegaPlant(uint8_t index);

    /** Destructor */
    ~omegaPlant();

    /** 
     * @brief Get measurement and update plant state
     * @param newData New sensor data
     * @param saveData Pointer to save data structure
     * @return Updated plant state
     */
    plantState getMeasurement(sensorData newData, PlantSaveData* saveData = nullptr);

    /**
     * @brief Calculate the mood based on sensor data
     * @param currentData Current sensor data
     * @return Mood percentage (0-100%)
     */
    uint8_t calculateMood(sensorData currentData);

    /**
     * @brief Update the mood history
     * @param newMood New mood value to add to history
     */
    void updateMoodHistory(uint8_t newMood);

    /**
     * @brief Calculate the level based on experience
     * @param exp Current experience points
     * @return Calculated level
     */
    uint8_t calculateLevel(uint8_t exp);

    /**
     * @brief Get recommendation based on sensor data
     * @param newData New sensor data
     * @return Recommendation value
     */
    uint8_t getRCMD(sensorData newData);

    /**
     * @brief Calculate experience points to be awarded
     * @return Calculated experience points
     */
    uint8_t calculateXP();

    /**
     * @brief Gain experience points
     * @param gainedXP Experience points to add
     * @return Total experience points after addition
     */
    uint8_t gainXP(uint8_t gainedXP);

    /**
     * @brief Check if the plant levels up
     * @return True if the plant levels up, false otherwise
     */
    bool isLevelUp();

    /**
     * @brief Save the current state to memory
     * @param index Memory index to save to
     * @return True if save is successful, false otherwise
     */
    bool saveMyState(uint8_t index);

    /**
     * @brief Load the state from memory
     * @param index Memory index to load from
     * @param data Pointer to save data structure
     * @return True if load is successful, false otherwise
     */
    bool LoadFromMemory(uint8_t index, PlantSaveData* data);

    /**
     * @brief Generate save data for the current instance
     * @param plant Pointer to omegaPlant instance
     * @return Generated save data
     */
    PlantSaveData generateSaveData(omegaPlant* plant);

    /**
     * @brief Get the current state
     * @param mood Reference to mood value
     * @param xp Reference to experience points value
     * @param emotion Reference to emotion value
     * @param lvl Reference to level value
     */
    void getState(uint8_t& mood, uint8_t& xp, uint8_t& emotion, uint8_t& lvl);
};

#endif // OMEGAPLANT_H
