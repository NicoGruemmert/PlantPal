
#include <freertos/FreeRTOS.h>
#include <TFT_eSPI.h>
#include <RotaryEncoder.h>

#include <Wire.h>
#include <vector>



TFT_eSPI tft;



TaskHandle_t dpTaskHandle;
TaskHandle_t gyroTaskHandle;
TaskHandle_t inputTaskHandle;
TaskHandle_t calibrationTaskHandle;
TaskHandle_t wifiTaskHandle;


void dpTask(void * params){

    while (1)
    {
            tft.fillScreen(TFT_BLACK);
            vTaskDelay(1000);
            tft.fillScreen(TFT_GREEN);
            Serial.print("HIII");
    }
    
}

void inputTask(void * pv){
    while (1)
    {
        vTaskDelay(1000);
    }
    
}


void setup(){
Serial.begin();
Wire.begin();
delay(100);

while (!Serial)
{
   delay(100);
}


Serial.print("HIII");

    while (1)
    {
      delay(1000);
    }
    

/*
    
    xTaskCreate(
        dpTask,      // Function name of the task
        "Display_Task",   // Name of the task (e.g. for debugging)
        2048,        // Stack size (bytes)
        NULL,        // Parameter to pass
        1,           // Task priority
        &dpTaskHandle         // Task handle
    );
   
    xTaskCreate(
        inputTask,      // Function name of the task
        "Input_Task",   // Name of the task (e.g. for debugging)
        2048,        // Stack size (bytes)
        NULL,        // Parameter to pass
        2,           // Task priority
        &inputTaskHandle         // Task handle
    );
*/


}

void loop(){

}