#ifndef OMEGA_BUTTON_H
#define OMEGA_BUTTON_H

#include <Arduino.h>

enum buttonEvent
{
NOTHING,
CLICK,
DBL_CLICK,
HOLD,
LONG_HOLD
};

class omegaButton
{ 
  public :
    const int debounce = 20;          // ms debounce period to prevent flickering when pressing or releasing the button
    const int DCgap = 250;            // max ms between clicks for a double click event
    const int holdTime = 300;        // ms hold period: how long to wait for press+hold event
    const int longHoldTime = 1500;

private:
    buttonEvent currentEvent;
    int myPin;
    bool buttonVal = true;   // value read from button
    bool buttonLast = true;  // buffered value of the button's previous state
    bool DCwaiting = false;  // whether we're waiting for a double click (down)
    bool DConUp = false;     // whether to register a double click on next release, or whether to wait and click
    bool singleOK = true;    // whether it's OK to do a single click
    long downTime = -1;         // time the button was pressed down
    long upTime = -1;           // time the button was released
    bool ignoreUp = false;   // whether to ignore the button release because the click+hold was triggered
    bool waitForUp = false;        // when held, whether to wait for the up event
    bool holdEventPast = false;    // whether or not the hold event happened already
    bool longHoldEventPast = false;// whether or not the long hold event happened already
public: 

omegaButton(int buttonPin) :myPin(buttonPin){};
buttonEvent getCurrentEvent()
{
    buttonEvent curEvent = currentEvent;
    currentEvent = NOTHING;
    return curEvent;
};

inline buttonEvent handleInput()
{

    buttonVal = digitalRead(myPin);
    buttonEvent event = NOTHING;


  // Button pressed down
  if (buttonVal == LOW && buttonLast == HIGH && (millis() - upTime) > debounce)
  {
    downTime = millis();
    ignoreUp = false;
    waitForUp = false;
    singleOK = true;
    holdEventPast = false;
    longHoldEventPast = false;
    if ((millis() - upTime) < DCgap && DConUp == false && DCwaiting == true)  DConUp = true;
    else  DConUp = false;
    DCwaiting = false;
  }
  // Button released
  else if (buttonVal == HIGH && buttonLast == LOW && (millis() - downTime) > debounce)
  {
    upTime = millis();
    if (!ignoreUp)
    {
      
      if (DConUp == false) DCwaiting = true;
      else
      {
        event = DBL_CLICK;
        DConUp = false;
        DCwaiting = false;
        singleOK = false;
        ignoreUp = false;
      }
    }
  }
  // Test for normal click event: DCgap expired
  if ( buttonVal == HIGH && (millis() - upTime) >= DCgap && DCwaiting == true && DConUp == false && singleOK == true && event != 2)
  {
    event = CLICK;
    DCwaiting = false;
  }
  // Test for hold
  if (buttonVal == LOW && (millis() - downTime) >= holdTime) {
    // Trigger "normal" hold
    if (!holdEventPast)
    {
      event = HOLD;
      waitForUp = true;
      ignoreUp = true;
      DConUp = false;
      DCwaiting = false;
      holdEventPast = true;
    }
    // Trigger "long" hold
    if ((millis() - downTime) >= longHoldTime)
    {
      if (!longHoldEventPast)
      {
        event = LONG_HOLD;
        longHoldEventPast = true;
      }
    }
  }
  buttonLast = buttonVal;
  return event;

}



};




#endif