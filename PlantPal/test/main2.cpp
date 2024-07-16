#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "omegaMotion.h"
#include "omegaMenu.h"

#include <freertos/FreeRTOS.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

omegaMotion* myMPU;

uint8_t menudata = 1;
uint8_t menudata2 = 1;

uint8_t gyroy = 0;
uint8_t gyrop = 0;
uint8_t gyror = 0;

void intitGyro();


omegaMenu submenus2 [4] = {
  omegaMenu(EMPTY,"Sub21", bmp_tag),
  omegaMenu(EMPTY,"Sub22",bmp_tag),
  omegaMenu(EMPTY,"Sub23", nullptr),
  omegaMenu(EXIT,"Exit", bmp_times_circle),
};


omegaMenu submenus1 [6] = {
  omegaMenu(SUBMENU,"Sub11", bmp_tag,submenus2,4),
  omegaMenu(FUNCTION,"Calibrate Gyro",bmp_tag,intitGyro),
  omegaMenu(VALUE,"Yaw", nullptr,gyroy),
  omegaMenu(VALUE,"Pitch", nullptr,gyrop),
  omegaMenu(VALUE,"Roll", nullptr,gyror),
  omegaMenu(EXIT,"Exit", bmp_times_circle),
};

omegaMenu submenus [3] = {
  omegaMenu(SUBMENU,"Gyro", bmp_tag,submenus1,6),
  omegaMenu(VALUE,"Sub2",bmp_tag,menudata),
  omegaMenu(VALUE,"Sub3", nullptr,menudata2)
};


omegaMenu* myMenu = new omegaMenu(SUBMENU,"Main Menu",nullptr,submenus,3);


#define PIN_SW D6
#define PIN_DT D1
#define PIN_CLK D3

volatile int swCount = 0;
volatile int encoderPos = 0;
volatile int lastCLKstate = 0;

void updateEncoder();
//omegaMotion myMPU;
//Quaternion *myq = new Quaternion();

void updateSW();


void dpTask(void * pvParameters)
{
    omegaMenu::display =&display;
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
    for(;;); // Don't proceed, loop forever
    
    display.setTextSize(1);      // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);     // Start at top-left corner
    display.cp437(true);      
    display.clearDisplay();
    display.display();

    myMenu->activateMenu();

    while (1)
    {
      myMenu->getCurrentMenu()->runMenu();
      vTaskDelay(1);
    }
    

}


void inputTask(void * paParameters)
{
  static int lastswcount;
  static int lastenc;
  while (1)
  {
    if (lastswcount<swCount)
    {
      myMenu->getCurrentMenu()->selectMenu();
      lastswcount = swCount;
      vTaskDelay(50);
    }

    if (lastenc<encoderPos)
    {
      myMenu->getCurrentMenu()->navUp();
      lastenc =encoderPos;
      vTaskDelay(20);
    }
        
    if (lastenc> encoderPos)
    {
      myMenu->getCurrentMenu()->navDown();
      lastenc = encoderPos;
      vTaskDelay(20);
      
    }
    
  }
}


void gyroTask(void * pvParameters)
{


  while (1)
  {

    if(myMPU)
    {
    
    float yprdeg[3];
    myMPU->getData(yprdeg,nullptr);
    gyroy = static_cast<int>(yprdeg[0] * 100) % 100;
    gyrop = static_cast<int>(yprdeg[1] * 100) % 100;
    gyror = static_cast<int>(yprdeg[2] * 100) % 100;
    }
    else{Serial.println("GYRO FAIL");
    vTaskDelay(10000);
    }
    
    
  /* code */
  }



}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);

 
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally

  pinMode(PIN_SW, INPUT_PULLUP); // Rotary Encoder Schalter Pin
  pinMode(PIN_DT, INPUT_PULLUP); // Rotary Encoder DT Pin
  pinMode(PIN_CLK, INPUT_PULLUP); // Rotary Encoder CLK Pin
  
  attachInterrupt(digitalPinToInterrupt(PIN_CLK), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_SW), updateSW, CHANGE);

  xTaskCreate(
        dpTask,      // Function name of the task
        "Display_Task",   // Name of the task (e.g. for debugging)
        2048,        // Stack size (bytes)
        NULL,        // Parameter to pass
        1,           // Task priority
        NULL         // Task handle
  );
  
  
  xTaskCreate(
        inputTask,      // Function name of the task
        "Input_Task",   // Name of the task (e.g. for debugging)
        2048,        // Stack size (bytes)
        NULL,        // Parameter to pass
        1,           // Task priority
        NULL         // Task handle
  );

  xTaskCreate(
        gyroTask,      // Function name of the task
        "Input_Task",   // Name of the task (e.g. for debugging)
        8192,        // Stack size (bytes)
        NULL,        // Parameter to pass
        2,           // Task priority
        NULL         // Task handle
  );
  
  
}

void loop() 
{}

void updateSW()
{
  bool SWstate = digitalRead(PIN_SW);
  if(SWstate)

  swCount++;
}


void updateEncoder() {
  int currentCLKstate = digitalRead(PIN_CLK);
  int DTstate = digitalRead(PIN_DT);

  if (currentCLKstate != lastCLKstate && currentCLKstate == HIGH) {
    if (DTstate != currentCLKstate) {
      encoderPos++;
    } else {
      encoderPos--;

    }
    // Hier können Sie weitere Bedingungen hinzufügen, falls gewünscht
  }

  lastCLKstate = currentCLKstate;
}


void intitGyro()
{
  static bool calibrated;
vTaskSuspendAll();
  myMPU = new omegaMotion();
  myMPU->initialize();
  calibrated= true;
  xTaskResumeAll();

  
}

