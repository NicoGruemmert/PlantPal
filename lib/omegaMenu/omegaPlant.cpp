#include "omegaPlant.h"

omegaPlant::omegaPlant(PlantProfile extPlantProfile) {
    myCurrentState.savedProfile = extPlantProfile;
}

omegaPlant::omegaPlant(PlantSaveData extSaveData) : myCurrentState(extSaveData) {}

omegaPlant::omegaPlant(uint8_t index) {
    PlantSaveData data;
    LoadFromMemory(index, &data); // Stores the read memory
}

omegaPlant::~omegaPlant() {}

plantState omegaPlant::getMeasurement(sensorData newData, PlantSaveData* saveData) {
    plantState curState;
    uint8_t rewardedXP = 0;
    curState.curData = newData;

    static int measureCounter = 0;
    measureCounter++;

    curState.curMood = calculateMood(newData);
    updateMoodHistory(curState.curMood);

    if (curState.curMood < 90) 
        curState.curEmotion = getRCMD(newData);
    else 
        curState.curEmotion = HAPPY;

    curState.curXP = gainXP(0); // Get current XP

    if (measureCounter % MEASUREMENT_PER_XP_GAIN == 0) {
        uint8_t rewardedXP = calculateXP();
        curState.curXP = gainXP(rewardedXP);
        uint8_t currentLevel = calculateLevel(curState.curXP);

        // On LevelUP
        if (isLevelUp()) {
            // Unlock stuff
            myCurrentState.unlockedItems |= ULCK_SUNGLASSES; // Unlock Sunglasses

            if (saveData != nullptr) *saveData = myCurrentState; // Return unlockables / new Savestate for publish
        }
    }

    return curState;
}

uint8_t omegaPlant::calculateMood(sensorData currentData) {
    uint8_t mood = 100; // Start with 100% mood
    PlantProfile myProfile = myCurrentState.savedProfile; // Access the current plant profile

    uint8_t range_temp = myProfile.range_temp;
    uint8_t range_hum = myProfile.range_hum;
    uint32_t range_light = myProfile.range_light;
    uint8_t range_soil_moisture = myProfile.range_soil_moisture;

    // Check Temperature
    if (currentData.temperature < myProfile.tempc - range_temp || currentData.temperature > myProfile.tempc + range_temp) {
        mood -= 25;
    } else if (currentData.temperature < myProfile.tempc) {
        mood -= (myProfile.tempc - currentData.temperature) * 25 / range_temp;
    } else if (currentData.temperature > myProfile.tempc) {
        mood -= (currentData.temperature - myProfile.tempc) * 25 / range_temp;
    }

    // Check Humidity
    if (currentData.humidity < myProfile.hum - range_hum || currentData.humidity > myProfile.hum + range_hum) {
        mood -= 25;
    } else if (currentData.humidity < myProfile.hum) {
        mood -= (myProfile.hum - currentData.humidity) * 25 / range_hum;
    } else if (currentData.humidity > myProfile.hum) {
        mood -= (currentData.humidity - myProfile.hum) * 25 / range_hum;
    }

    // Check Light
    if (currentData.lightIntensity < myProfile.light - range_light || currentData.lightIntensity > myProfile.light + range_light) {
        mood -= 25;
    } else if (currentData.lightIntensity < myProfile.light) {
        mood -= (myProfile.light - currentData.lightIntensity) * 25 / range_light;
    } else if (currentData.lightIntensity > myProfile.light) {
        mood -= (currentData.lightIntensity - myProfile.light) * 25 / range_light;
    }

    // Check Soil Moisture
    if (currentData.moisture < myProfile.soil_moisture - range_soil_moisture || currentData.moisture > myProfile.soil_moisture + range_soil_moisture) {
        mood -= 25;
    } else if (currentData.moisture < myProfile.soil_moisture) {
        mood -= (myProfile.soil_moisture - currentData.moisture) * 25 / range_soil_moisture;
    } else if (currentData.moisture > myProfile.soil_moisture) {
        mood -= (currentData.moisture - myProfile.soil_moisture) * 25 / range_soil_moisture;
    }

    // Ensure mood is not less than 0
    if (mood < 0) {
        mood = 0;
    }

    return mood; // Mood in percentage (0-100%)
}

void omegaPlant::updateMoodHistory(uint8_t newMood) {
    static uint8_t moodHistorySize;
    if (moodHistorySize < MEASUREMENT_PER_XP_GAIN) {
        moodHistory[moodHistorySize++] = newMood;
    } else {
        // Shift the history array to the left and add the new mood at the end
        for (uint8_t i = 1; i < MEASUREMENT_PER_XP_GAIN; ++i) {
            moodHistory[i - 1] = moodHistory[i];
        }
        moodHistory[MEASUREMENT_PER_XP_GAIN - 1] = newMood;
    }
}

uint8_t omegaPlant::calculateLevel(uint8_t exp) {
    uint8_t level = 1;
    uint32_t exp_needed = 2;

    while (exp >= exp_needed) {
        exp -= exp_needed;
        level++;
        exp_needed = 2 * level;
    }

    return level;
}

uint8_t omegaPlant::getRCMD(sensorData newData) {
    // Access the current plant profile
    PlantProfile myProfile = myCurrentState.savedProfile;

    // Initialize the deviation values
    int8_t tempDeviation = newData.temperature - myProfile.tempc;
    int8_t humDeviation = newData.humidity - myProfile.hum;
    int8_t moistureDeviation = newData.moisture - myProfile.soil_moisture;
    int8_t lightDeviation = newData.lightIntensity - myProfile.light;

    int8_t maxDeviation = std::max({abs(tempDeviation), abs(humDeviation), abs(moistureDeviation), abs(lightDeviation)});
    uint8_t recommendation = HAPPY;

    if (maxDeviation == abs(tempDeviation)) {
        recommendation = (tempDeviation > 0) ? TOO_HOT : TOO_COLD;
    } else if (maxDeviation == abs(humDeviation)) {
        recommendation = (humDeviation > 0) ? TOO_HUMID : TOO_DRY;
    } else if (maxDeviation == abs(moistureDeviation)) {
        recommendation = (moistureDeviation > 0) ? TOO_MOIST : TOO_ARID;
    } else if (maxDeviation == abs(lightDeviation)) {
        recommendation = (lightDeviation > 0) ? TOO_SUNNY : TOO_DARK;
    }

    return recommendation;
}

uint8_t omegaPlant::calculateXP() {
    uint8_t baseXP = 3;
    uint16_t accMood = 0;

    for (size_t i = 0; i < MEASUREMENT_PER_XP_GAIN; i++) {
        accMood += moodHistory[i];
    }

    return baseXP * (accMood / MEASUREMENT_PER_XP_GAIN) / 100;
}

uint8_t omegaPlant::gainXP(uint8_t gainedXP) {
    myCurrentState.savedExp += gainedXP;
    return myCurrentState.savedExp;
}

bool omegaPlant::isLevelUp() {
    static uint8_t level = 0;
    uint8_t newLevel = calculateLevel(myCurrentState.savedExp);

    if (newLevel > level) {
        level = newLevel;
        return true;
    } else {
        level = newLevel;
        return false;
    }
}

bool omegaPlant::saveMyState(uint8_t index) {
    PlantSaveData mydata = generateSaveData(this);
    PlantSaveData *data = &mydata;

    if (!data) return false;

    if (index < 0 || index >= MAX_PLANTS) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        return err;
    }

    char key[16];
    snprintf(key, sizeof(key), "plant%d", index);
    err = nvs_set_blob(nvs_handle, key, data, sizeof(PlantSaveData));
    if (err != ESP_OK) {
        nvs_close(nvs_handle);
        return err;
    }

    err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    return err;
}

bool omegaPlant::LoadFromMemory(uint8_t index, PlantSaveData* data) {
    if (index < 0 || index >= MAX_PLANTS || data == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        return err;
    }

    char key[16];
    snprintf(key, sizeof(key), "plant%d", index);
    size_t required_size = sizeof(PlantSaveData);
    err = nvs_get_blob(nvs_handle, key, data, &required_size);
    nvs_close(nvs_handle);
    return err;
}

PlantSaveData omegaPlant::generateSaveData(omegaPlant* plant) {
    return plant->myCurrentState;
}

void omegaPlant::getState(uint8_t& mood, uint8_t& xp, uint8_t& emotion, uint8_t& lvl) {
    mood = myCurrentState.savedProfile.hum;
    xp = myCurrentState.savedExp;
    emotion = myCurrentState.savedProfile.light;
    lvl = myCurrentState.savedLvL;
}
