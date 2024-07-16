#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <FreeSans9pt7b.h>
#include <Fonts/Picopixel.h>
#include <Fonts/SquareBold6pt7b.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define ICON_SIZE 16


#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32


const int centerX = SCREEN_WIDTH / 4;
const int centerY = SCREEN_HEIGHT / 2;

const unsigned char bmp_times_circle [] PROGMEM = {
	0x00, 0x00, 0x07, 0xe0, 0x18, 0x18, 0x30, 0x04, 0x20, 0x04, 0x46, 0x62, 0x43, 0xc2, 0x41, 0x82, 
	0x43, 0xc2, 0x46, 0x62, 0x64, 0x26, 0x20, 0x04, 0x18, 0x18, 0x0f, 0xf0, 0x03, 0xc0, 0x00, 0x00
};

const unsigned char bmp_tag [] PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x3e, 0x00, 0x1e, 0x00, 0x1e, 0x00, 0x36, 
	0x3c, 0x62, 0x3c, 0xc0, 0x43, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};



enum menuType
{
    EMPTY,
    SUBMENU,
    EXIT,
    VALUE,
    FUNCTION,
    FUNCTION_MENU
};

enum inputType
{
    PRESS,
    LONG_PRESS,
    DOUBLE_PRESS,
    UP,
    DOWN,
};



class omegaMenu
{
    public:
     static Adafruit_SSD1306* display;

private:



    menuType type;
   
    const char * name;
    omegaMenu * subMenus;

    omegaMenu * lastMenu = nullptr;
    omegaMenu * currentMenu;
    
    uint8_t subMenuCount;
    const unsigned char * icon;
    uint8_t selectedItem = 0;

    uint8_t dataValue = 0;

    void (*externalFunction)(void) = nullptr;
    omegaMenu * (*extMenu)(void) = nullptr;
    
public:
    omegaMenu(menuType myType, const char * myName, const unsigned char* myIcon) : 
    sensorInitCallback(nullptr),
    type(myType),
    name(myName), 
    icon(myIcon){}
    
    omegaMenu(menuType myType, const char * myName, const unsigned char* myIcon ):
    type(myType),
    name(myName), 
    icon(myIcon){}

    omegaMenu(menuType myType, const char * myName, const unsigned char* myIcon, uint8_t & myData) :
    sensorInitCallback(nullptr), 
        type(myType),
        name(myName), 
        icon(myIcon),
        dataValue(myData){}

    omegaMenu(menuType myType, char * myName,const unsigned char* myIcon, omegaMenu* myMenus, uint8_t myCount) : 
        sensorInitCallback(nullptr),
        type(myType),
        name(myName),
        icon(myIcon), 
        subMenus(myMenus), 
        subMenuCount(myCount){}
    


    bool forceDraw = true;
    
    void setCurrentMenu(omegaMenu* mycurrentMenu)
    {
        if(lastMenu!=nullptr)lastMenu->setCurrentMenu(mycurrentMenu);
        currentMenu = mycurrentMenu;
        
    }
    void setLastMenu(omegaMenu* mylastMenu){lastMenu =mylastMenu;}
    
    omegaMenu * getCurrentMenu(){return currentMenu;}
    omegaMenu * getSubMenus(){return subMenus;}

void triggerSensorInit() {
        if (sensorInitCallback) {
            sensorInitCallback();
            exitMenu();
        }
    }

    void activateMenu()
    {
        currentMenu = this;
    }

    void selectMenu()
    {   
            if (this->type == SUBMENU)
            {
            if(subMenus[selectedItem].type == EXIT)
            {   
                exitMenu();
                return;
            }

            if(subMenus[selectedItem].type !=0)
            {
                setCurrentMenu(&subMenus[selectedItem]);
                subMenus[selectedItem].setLastMenu(this);
                subMenus[selectedItem].forceDraw = true;
                return;
            }
            }

         if(this->type == VALUE || this->type == FUNCTION) // Value wurde gesetzt
        {
            exitMenu();
            return;
        }
        



    }

    void exitMenu()
    {

        if(lastMenu!= nullptr){
            lastMenu->setCurrentMenu(lastMenu);
            lastMenu->forceDraw =true;
            selectedItem =0;
            lastMenu=nullptr;
            return;
        }
    }

    void runMenu()
    {
        switch (type)
        {
        case SUBMENU:
        
            drawSubMenu();
            break;
        case VALUE:
            if (&this->dataValue != nullptr)
            {
                forceDraw =true;
                //display->clearDisplay();
                lastMenu->drawSubMenu();
                drawValueMenu();
            }
            
            break;
        case FUNCTION_MENU
        subMenus =
        drawSubMenu();

        break;
        default:
            break;
        }

        display->display();
    }

    void drawValueMenu()
    {
        //display->clearDisplay();
        //display->setCursor(100,16);
        //display->println(dataValue);
    
    }

    const unsigned char *getIcon()
    {
        return icon;
    }
    
    inline void navUp()
    {
        switch (type)
        {
        case SUBMENU:
            if(selectedItem<subMenuCount-1)
            selectedItem++;
            break;
        case VALUE:
            if(&dataValue != nullptr) dataValue++;

        
        default:
            break;
        }

    }
    
    inline void navDown()
    {
        switch (type)
        {
        case SUBMENU:
            if(selectedItem>0)
            selectedItem--;
            break;
        case VALUE:
            if(&dataValue != nullptr) dataValue--;
        default:
            break;
        }

    
    }
    
    const char* getName(){return name;}

    void drawSubMenu()
    {
        static uint8_t lastSelectedItem;
        //if (lastSelectedItem == selectedItem && !forceDraw) return;
        
        forceDraw=false;
        display->clearDisplay();

        //Obere Ecke
        display->writePixel(1, 0, SSD1306_WHITE);
        display->writePixel(1, 1, SSD1306_WHITE);
        display->writePixel(1, 2, SSD1306_WHITE);
        display->writePixel(2, 0, SSD1306_WHITE);
        display->writePixel(3, 0, SSD1306_WHITE);
        display->writePixel(4, 0, SSD1306_WHITE);

        //Untere Ecke
        display->writePixel(1, SCREEN_HEIGHT-1, SSD1306_WHITE);
        display->writePixel(1, SCREEN_WIDTH-2, SSD1306_WHITE);
        display->writePixel(1, SCREEN_HEIGHT-3, SSD1306_WHITE);
        display->writePixel(2, SCREEN_HEIGHT-1, SSD1306_WHITE);
        display->writePixel(3, SCREEN_HEIGHT-1, SSD1306_WHITE);
        display->writePixel(4, SCREEN_HEIGHT-1, SSD1306_WHITE);

        //Scroller
        int rectY = 4+((SCREEN_HEIGHT-8) / subMenuCount) * selectedItem;
        display->fillRoundRect(0,rectY,3,6,1,SSD1306_WHITE);

        //Rechte Menüleiste
        for (size_t i = 3; i <= SCREEN_HEIGHT-4; i += 4)
        {
            // Zwei Pixel einschalten
            display->writePixel(1, i, SSD1306_BLACK);
            display->writePixel(1, i + 1, SSD1306_BLACK);

            // Zwei Pixel ausschalten
            display->writePixel(1, i + 2, SSD1306_WHITE);
            display->writePixel(1, i + 3, SSD1306_WHITE);
        }

        if(selectedItem != 0) //Falls nicht das erste Element, Zeige vorheriges
        {
            display->setFont();
            display->setCursor(22, 0);
            display->print(subMenus[selectedItem-1].getName());

            if (subMenus[selectedItem-1].type == VALUE)
            {
                display->setCursor(22+64, 0);
                display->print(subMenus[selectedItem-1].dataValue);


            }
        }

        if(selectedItem != subMenuCount-1)// Kein unteres Element
        {   
            display->setFont();
            display->setCursor(22,  SCREEN_HEIGHT-7); 
            display->println(subMenus[selectedItem+1].getName());
            
            if (subMenus[selectedItem+1].type == VALUE)
            {
                display->setCursor(22+64,  SCREEN_HEIGHT-7);
                display->print(subMenus[selectedItem+1].dataValue);
            }

        } 

        //Mittleres Item
        if(selectedItem<subMenuCount)
        {
            if(subMenus[selectedItem].getIcon()!= nullptr)
            display->drawBitmap(5, 8, subMenus[selectedItem].getIcon(), ICON_SIZE, ICON_SIZE, SSD1306_WHITE);
            
            display->setCursor(ICON_SIZE + 8, 13);
            display->setFont(&SquareBold6pt7b);
            display->print(subMenus[selectedItem].getName());

            if(subMenus[selectedItem].type == VALUE)
            {
                 display->setCursor(ICON_SIZE + 8 +64 , 19);
                 display->print(subMenus[selectedItem].dataValue);

            }
            //display->drawRoundRect(ICON_SIZE+8,9,80,14,5,SSD1306_WHITE); //Optional
        }

        lastSelectedItem = selectedItem;        
    };

    ~omegaMenu();
};

omegaMenu::~omegaMenu()
{
}


Adafruit_SSD1306* omegaMenu::display = nullptr;
