#include <TouchScreen.h>
#include <TFTScreen.h>
#include <Keyboard.h>
#include <Fonts/FreeSans12pt7b.h>
#include <RotaryEncoder.h>
#include "Mouse.h"

#define GREEN 1365
#define ORANGE 64905
#define LIGHT_RED 64076
#define BLUE_GRAY 19151
#define DARK_BLUE_GRAY 12812
#define DARK_BLUE 627
#define BLUE 854
#define DARK_RED 59753
#define LIGHT_BLUE 1599
#define DARK_GRAY 4359
#define ALMOST_WHITE 52825

#define MINPRESSURE 100
#define MAXPRESSURE 1000
#define SWAP(a, b) { uint16_t tmp = a; a = b; b = tmp; }

TFTScreen myScreen;

TouchScreen ts = TouchScreen(6, A1, A2, 7, 300);
TSPoint tp;

TFTButton UndoButton;

uint16_t xpos, ypos;

class clickButton
{
  public:
    clickButton(int pin)
    {
      buttonPin = pin;  
    }
    
    int buttonPin;
    int buttonState;
    int lastButtonState;
    unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 50;    
};

clickButton topButton(1);
clickButton middleButton(3);
clickButton bottomButton(2);
clickButton rotaryButton(0);

RotaryEncoder encoder(A4, A5);

enum PSInputMode { BrushSize, Zoom };
PSInputMode CurrentPSInputMode;

enum MenuState { MainMenu, PSMenu, YouTubeMenu };

void setup(void)
{
    CurrentPSInputMode = BrushSize;
    
    Serial.begin(9600);

    pinMode(12, OUTPUT);
    pinMode(topButton.buttonPin, INPUT);    
    pinMode(middleButton.buttonPin, INPUT);
    pinMode(bottomButton.buttonPin, INPUT);
    
    myScreen.begin(37671);
    myScreen.setRotation(1);

    myScreen.fillScreen(GetColor(0,0,0));

    //Green
    myScreen.fillRect(0, 0, 32, 32, GREEN);

    //Orange
    myScreen.fillRect(32, 0, 32, 32, ORANGE);

    //LightRed
    myScreen.fillRect(64, 0, 32, 32, LIGHT_RED);

    //BlueGray
    myScreen.fillRect(96, 0, 32, 32, BLUE_GRAY);

    //DarkBlueGray
    myScreen.fillRect(128, 0, 32, 32, DARK_BLUE_GRAY);

    //DarkBlue
    myScreen.fillRect(160, 0, 32, 32, DARK_BLUE);

    //Blue
    myScreen.fillRect(192, 0, 32, 32, BLUE);    

    //DarkRed
    myScreen.fillRect(224, 0, 32, 32, DARK_RED);

    //LightBlue
    myScreen.fillRect(256, 0, 32, 32, LIGHT_BLUE);

    //DarkGray
    myScreen.fillRect(288, 0, 32, 32, DARK_GRAY); 

    //AlmostWhite
    myScreen.fillRect(320, 0, 32, 32, ALMOST_WHITE); 

    myScreen.setFont(&FreeSans12pt7b);
    
    UndoButton.InitButton(&myScreen, 16, 16, 150, 80, 
      DARK_RED,           //OUTLINE COLOUR
      ALMOST_WHITE,       //INTERNAL COLOUR
      DARK_BLUE, "Undo"); //FONT COLOUR
      
    UndoButton.DrawButton();
      
    delay(500);
}

void loop()
{
    static int pos = 0;
    {
        int reading = digitalRead(topButton.buttonPin);
        if (reading != topButton.lastButtonState) 
        {
          topButton.lastDebounceTime = millis();
        }
        
        if ((millis() - topButton.lastDebounceTime) > topButton.debounceDelay) 
        {
          if (reading != topButton.buttonState) 
          {
            topButton.buttonState = reading;
      
            if (topButton.buttonState == LOW) 
            {
              Keyboard.press(KEY_LEFT_CTRL); 
              Keyboard.press(KEY_LEFT_ALT);         
              Keyboard.press('z');
              Keyboard.releaseAll();
            }
          }
        }
        
        topButton.lastButtonState = reading;
    }

    if (digitalRead(middleButton.buttonPin) == 0)
    {
        Keyboard.press(' ');
    }
    else
    {
        Keyboard.release(' ');
    }
 
    

//    {
//        int reading = digitalRead(middleButton.buttonPin);
//        if (reading != middleButton.lastButtonState) 
//        {
//          middleButton.lastDebounceTime = millis();
//        }
//        
//        if ((millis() - middleButton.lastDebounceTime) > middleButton.debounceDelay) 
//        {
//          if (reading != middleButton.buttonState) 
//          {
//            middleButton.buttonState = reading;
//      
//            if (middleButton.buttonState == LOW) 
//            {
//              Keyboard.press(KEY_LEFT_CTRL); 
//              Keyboard.press(KEY_LEFT_ALT);         
//              Keyboard.press('z');
//              Keyboard.releaseAll();
//            }
//          }
//        }
//        
//        middleButton.lastButtonState = reading;
//    }

//    {
//        int reading = digitalRead(rotaryButton.buttonPin);
//        if (reading != rotaryButton.lastButtonState) 
//        {
//          rotaryButton.lastDebounceTime = millis();
//        }
//        
//        if ((millis() - rotaryButton.lastDebounceTime) > rotaryButton.debounceDelay) 
//        {
//          if (reading != rotaryButton.buttonState) 
//          {
//            rotaryButton.buttonState = reading;
//      
//            if (rotaryButton.buttonState == LOW) 
//            {
//              Keyboard.press(KEY_LEFT_CTRL); 
//              Keyboard.press(KEY_LEFT_SHIFT);         
//              Keyboard.press('n');
//              Keyboard.press(KEY_RETURN);
//              Keyboard.releaseAll();
//            }
//          }
//        }
//        
//        rotaryButton.lastButtonState = reading;
//    }

    {
        int reading = UndoButton.pressed;
        if (reading != UndoButton.lastButtonState) 
        {
          UndoButton.lastDebounceTime = millis();
        }
        
        if ((millis() - UndoButton.lastDebounceTime) > UndoButton.debounceDelay) 
        {
          if (reading != UndoButton.buttonState) 
          {
            UndoButton.buttonState = reading;
      
            if (UndoButton.buttonState == LOW) 
            {
              tone(12, 3500,5);
              
              switch(CurrentPSInputMode)
              {
                  case BrushSize:
                  {
                    CurrentPSInputMode = Zoom;
                    UndoButton.ChangeLabel("Zoom");
                  }
                  break;

                  case Zoom:
                  {
                    CurrentPSInputMode = BrushSize;
                    UndoButton.ChangeLabel("Brush Size");
                  }
                  break;
              }
              
              UndoButton.DrawButton();
            }
          }
        }
        
        UndoButton.lastButtonState = reading;
    }
    
    encoder.tick();
    int newPos = encoder.getPosition();

    switch(CurrentPSInputMode)
    {
        case BrushSize:
        {
            if (pos != newPos) 
            {      
              if (pos > newPos) { Keyboard.print("["); }
              if (pos < newPos) { Keyboard.print("]"); }
              
              Serial.write("Hello");
              
              pos = newPos;
            }
        }
        break;

        case Zoom:
        {
          if (pos != newPos) 
            {      
              if (pos > newPos) 
              { 
                Keyboard.press(KEY_LEFT_ALT);
                delay(16);
                Mouse.move(0,0,-1);
                delay(16);
                Keyboard.release(KEY_LEFT_ALT);
              }
              
              if (pos < newPos) 
              { 
                Keyboard.press(KEY_LEFT_ALT);
                delay(16);
                Mouse.move(0,0,1);
                delay(16);
                Keyboard.release(KEY_LEFT_ALT);
              }
              
              pos = newPos;
            }             
        }
        break;
    }

    if (digitalRead(bottomButton.buttonPin) == 0)
    {
        Keyboard.press(KEY_LEFT_ALT);
    }
    else   
    {
        Keyboard.release(KEY_LEFT_ALT);
    }

    tp = ts.getPoint();
    SetPins(); //This is necessary for the touch screen to work properly

    if (tp.z > MINPRESSURE && 
        tp.z < MAXPRESSURE) 
    {
        xpos = map(tp.y, 950, 180, 0, myScreen.width());
        ypos = map(tp.x, 170, 880, 0, myScreen.height());

        UndoButton.CheckButton(xpos, ypos);
    }
    else
    {
        UndoButton.CheckButton(900, 900);
    }
}

void SetPins()
{
    pinMode(A2, OUTPUT);
    pinMode(A1, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
}

uint16_t GetColor(uint8_t r, uint8_t g, uint8_t b) 
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

