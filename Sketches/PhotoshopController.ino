#include <TouchScreen.h>
#include <TFTScreen.h>
#include <Keyboard.h>
#include <Fonts/FreeSans12pt7b.h>
#include <RotaryEncoder.h>
#include <Mouse.h>

//#define GREEN 1365
//#define ORANGE 64905
//#define LIGHT_RED 64076
//#define BLUE_GRAY 19151
//#define DARK_BLUE_GRAY 12812
#define DARK_BLUE 627
//#define BLUE 854
//#define DARK_RED 59753
#define LIGHT_BLUE 1599
//#define DARK_GRAY 4359
//#define ALMOST_WHITE 52825
#define win10Col 986

#define BLACK 0x0000
#define WHITE 0xFFFF
#define RED 63488
//#define GREEN 0x07E0
//#define YELLOW 0xFFE0
//#define LIME 0x07FF
#define VS_PURPLE 22898

#define MINPRESSURE 20
#define MAXPRESSURE 1000

TFTScreen myScreen;
TouchScreen ts = TouchScreen(6, A1, A2, 7, 300);
TSPoint tp;
uint16_t xpos, ypos;

class clickButton
{
  public:
    clickButton(int pin)
    {
        buttonPin = pin;  
    }
    
    int buttonPin;
    bool buttonState;
    bool lastButtonState;
    unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 50;    
};

//Physical buttons
clickButton topButton(1);
clickButton middleButton(3);
clickButton bottomButton(2);
clickButton rotaryButton(0);

//PS Menu Buttons
TFTButton MainMenuButton;
TFTButton SwitchModeButton;
TFTButton NewLayerButton;
TFTButton ChangeBrushButton;

//Main Menu Buttons
TFTButton PhotoshopButton;
TFTButton WindowsButton;
TFTButton ChromeButton;
TFTButton YouTubeButton;

RotaryEncoder encoder(A4, A5);

enum PSInputMode { BrushSize, Zoom };
PSInputMode CurrentPSInputMode;

enum MenuState { MainMenu, PSMenu, WindowsMenu, ChromeMenu, YouTubeMenu  };
MenuState CurrentMenuState;

enum BrushState { Brush, Eraser};
BrushState CurrentBrushState;

void setup(void)
{
    CurrentMenuState = MainMenu;
    CurrentPSInputMode = BrushSize;
    
    Serial.begin(9600);

    pinMode(12, OUTPUT); //Buzzer pin
    pinMode(topButton.buttonPin, INPUT);    
    pinMode(middleButton.buttonPin, INPUT);
    pinMode(bottomButton.buttonPin, INPUT);
    
    myScreen.begin(37671);
    myScreen.setRotation(1);

    myScreen.fillScreen(GetColor(0,0,0));
    myScreen. setFont(&FreeSans12pt7b);

    //PS Menu Buttons
    SwitchModeButton.InitButton(&myScreen, 16, 16, 150, 80, 
      WHITE,           //OUTLINE COLOUR
      LIGHT_BLUE,       //INTERNAL COLOUR
      WHITE, "Brush Size"); //FONT COLOUR

    NewLayerButton.InitButton(&myScreen, 182, 16, 150, 80,
      WHITE,
      LIGHT_BLUE,
      WHITE, "New Layer");

    ChangeBrushButton.InitButton(&myScreen, 16, 112, 150, 80,
      WHITE,
      LIGHT_BLUE,
      WHITE, "Brush"); 

    MainMenuButton.InitButton(&myScreen, 16, 198 , 32, 26,
      WHITE,
      LIGHT_BLUE,
      WHITE, "");


    //Main Menu Buttons
    PhotoshopButton.InitButton(&myScreen, 8, 16, 64,64,
      LIGHT_BLUE,
      DARK_BLUE,
      LIGHT_BLUE,
      "Ps");

    WindowsButton.InitButton(&myScreen, 88, 16, 64, 64,
    GetColor(255,255,255),
    win10Col,    
    win10Col,
    "");

    ChromeButton.InitButton(&myScreen, 168, 16, 64,64,
      WHITE,
      GetColor(255, 223, 0),
      WHITE,
      "");

    YouTubeButton.InitButton(&myScreen, 248, 16, 64,64,
      RED,
      WHITE,
      WHITE,
      "");

    DrawMainMenu();
      
    delay(500);
}

void loop()
{
    switch (CurrentMenuState)
    {
        case ChromeMenu:
        case WindowsMenu:
        case PSMenu:
        case YouTubeMenu:
        {
            if (UpdateTFTButton(MainMenuButton) == true)
            {
                tone(12, 3500, 5);
                CurrentMenuState = MainMenu;

                DrawMainMenu();
            }
        }
        break;
    }

    switch (CurrentMenuState)
    {      
        case MainMenu:
        {
            if (UpdateTFTButton(PhotoshopButton) == true)
            {
                tone(12, 3500, 5);
                CurrentMenuState = PSMenu;

                DrawPSMenu();
            }

            if (UpdateTFTButton(WindowsButton) == true)
            {
                tone(12, 3500, 5);
                CurrentMenuState = WindowsMenu;
                
                DrawWindowsMenu();
            }

            if (UpdateTFTButton(ChromeButton) == true)
            {
                tone(12, 3500, 5);
                CurrentMenuState = ChromeMenu;
                
                DrawChromeMenu();
            }

            if (UpdateTFTButton(YouTubeButton) == true)
            {
                tone(12, 3500, 5);
                CurrentMenuState = YouTubeMenu;
                
                DrawYouTubeMenu();
            }
        }
        break;

        case ChromeMenu:
        {
          
        }
        break;
    
        case WindowsMenu:
        {
            if (UpdatePhysicalButton(rotaryButton) == true)
            {
                Serial.println("Rotary Button Clicked");
                Remote.mute();
                Remote.clear();
            }
        }
        break;
      
        case PSMenu:
        {
            //Top physical button
            if (digitalRead(middleButton.buttonPin) == 1)
            {
                if (UpdatePhysicalButton(topButton) == true)
                {
                    Keyboard.press(KEY_LEFT_CTRL); 
                    Keyboard.press(KEY_LEFT_ALT);         
                    Keyboard.press('z');
                    Keyboard.releaseAll();
                }
            }
        
            //Middle physical button
            if (digitalRead(middleButton.buttonPin) == 0)
            {
                if (UpdatePhysicalButton(topButton) == true)
                {
                    Keyboard.press(KEY_LEFT_CTRL); 
                    Keyboard.press(KEY_LEFT_SHIFT);         
                    Keyboard.press('z');
                    Keyboard.releaseAll();
                }
                else
                {
                    Keyboard.press(' ');
                }
            }
            else
            {
                Keyboard.release(' ');
            }
        
            //Bottom physical button
            if (digitalRead(bottomButton.buttonPin) == 0)
            {
                Keyboard.press(KEY_LEFT_ALT);
            }
            else   
            {
                Keyboard.release(KEY_LEFT_ALT);
            }            

            //New Layer touch button
            if (UpdateTFTButton(NewLayerButton) == true)
            {
                tone(12, 3500,5);
                Keyboard.press(KEY_LEFT_CTRL); 
                Keyboard.press(KEY_LEFT_SHIFT);         
                Keyboard.press('n');
                Keyboard.press(KEY_RETURN);
                Keyboard.releaseAll();
            }
            
            //Switch Mode touch button
            if (UpdateTFTButton(SwitchModeButton) == true)
            {
                tone(12, 500, 5);
        
                switch(CurrentPSInputMode)
                {
                    case BrushSize:
                    {
                      CurrentPSInputMode = Zoom;
                      SwitchModeButton.ChangeLabel("Zoom");
                    }
                    break;
        
                    case Zoom:
                    {
                      CurrentPSInputMode = BrushSize;
                      SwitchModeButton.ChangeLabel("Brush Size");
                    }
                    break;
                }
                
                SwitchModeButton.DrawButton();      
            }

            //Change brush mode
            if (UpdateTFTButton(ChangeBrushButton) == true)
            {
                switch(CurrentBrushState)
                {
                    case Brush:
                    {
                        tone(12, 500, 10);
                        Keyboard.print('e');
                        Keyboard.release('e');
                        ChangeBrushButton.ChangeLabel("Eraser");
                        CurrentBrushState = Eraser;
                    }
                    break;
        
                    case Eraser:
                    {
                        tone(12, 2500, 5);
                        Keyboard.print('b');
                        Keyboard.release('b');
                        ChangeBrushButton.ChangeLabel("Brush");
                        CurrentBrushState = Brush;
                    }
                    break;
                }
                
                ChangeBrushButton.DrawButton();      
            }
        }
        break;
    }

    //Rotary encoder
    static int pos = 0;
    encoder.tick();
    int newPos = encoder.getPosition();

    if (pos != newPos) 
    {
      if (CurrentMenuState == PSMenu)
      {
          switch(CurrentPSInputMode)
          {
              case BrushSize:
              {                 
                  if (pos > newPos) { Keyboard.print("["); }
                  if (pos < newPos) { Keyboard.print("]"); }
              }
              break;
      
              case Zoom:
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
              }
              break;
          }
      }

      if (CurrentMenuState == WindowsMenu)
      {
          if (pos > newPos)
          {
              Remote.decrease();
              Remote.clear(); 
          }

          if (pos < newPos)
          {
              Remote.increase();
              Remote.clear(); 
          }
      }
        
      pos = newPos;
    }

    //Update touch screen
    tp = ts.getPoint();
    SetPins(); //This is necessary for the touch screen to work properly

    if (tp.z > MINPRESSURE && 
        tp.z < MAXPRESSURE) 
    {
        xpos = map(tp.y, 950, 180, 0, myScreen.width());
        ypos = map(tp.x, 170, 880, 0, myScreen.height());

        switch (CurrentMenuState)
        {
            case MainMenu:
            {
                PhotoshopButton.CheckButton(xpos, ypos);
                WindowsButton.CheckButton(xpos, ypos);
                ChromeButton.CheckButton(xpos, ypos);
                YouTubeButton.CheckButton(xpos, ypos);
            }
            break;
          
            case PSMenu:
            {
                SwitchModeButton.CheckButton(xpos, ypos);
                NewLayerButton.CheckButton(xpos, ypos);
                ChangeBrushButton.CheckButton(xpos, ypos);                
            }
            break;

            case WindowsMenu:
            {
              
            }
            break;

            case ChromeMenu:
            {
              
            }
            break;
        }

        switch (CurrentMenuState)
        {
            case PSMenu:
            case WindowsMenu:
            case ChromeMenu:
            case YouTubeMenu:
            {
                MainMenuButton.CheckButton(xpos, ypos);
            }
            break;
        }
    }
    else
    {
        switch (CurrentMenuState)
        {
            case PSMenu:
            case WindowsMenu:
            case ChromeMenu:
            case YouTubeMenu:
            {
                MainMenuButton.CheckButton(900, 900);
            }
            break;
        }
      
        switch (CurrentMenuState)
        {
            case MainMenu:
            {
                PhotoshopButton.CheckButton(900, 900);
                WindowsButton.CheckButton(900, 900);
                ChromeButton.CheckButton(900, 900);
                YouTubeButton.CheckButton(900, 900);
            }
            break;
          
            case PSMenu:
            {
                SwitchModeButton.CheckButton(900, 900);
                NewLayerButton.CheckButton(900, 900); 
                ChangeBrushButton.CheckButton(900, 900);             
            }
            break;

            case WindowsMenu:
            {
              
            }
            break;

            case ChromeMenu:
            {
                
            }
            break;
        }
    }
}

bool UpdateTFTButton(TFTButton &tftButton)
{
    bool result = false;
    int reading = tftButton.pressed;
    
    if (reading != tftButton.lastButtonState) 
    {
        tftButton.lastDebounceTime = millis();
    }
    
    if ((millis() - tftButton.lastDebounceTime) > tftButton.debounceDelay) 
    {
      if (reading != tftButton.buttonState) 
      {
        tftButton.buttonState = reading;
  
        if (tftButton.buttonState == LOW) 
        {
            result = true;
        }
      }
    }
    
    tftButton.lastButtonState = reading;
    return result;
}

bool UpdatePhysicalButton(clickButton &button)
{
    int reading = digitalRead(button.buttonPin);
    bool result = false;
    
    if (reading != button.lastButtonState) 
    {
      button.lastDebounceTime = millis();
    }
    
    if ((millis() - button.lastDebounceTime) > button.debounceDelay) 
    {
      if (reading != button.buttonState) 
      {
        button.buttonState = reading;
  
        if (button.buttonState == LOW) 
        {
            result = true;
        }
      }
    }
        
    button.lastButtonState = reading;
    return result;
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

void DrawMainMenu()
{
    myScreen.fillScreen(BLACK);
  
    WindowsButton.DrawButton();
    PhotoshopButton.DrawButton();
    ChromeButton.DrawButton();
    YouTubeButton.DrawButton();
  
    DrawWindowsLogo(16, 88);
    DrawChromeLogo(168, 16);
    DrawYouTubeLogo(246, 16);
    DrawVSLogo(328, 16);
}

void DrawPSMenu()
{
    myScreen.fillScreen(BLACK);
    NewLayerButton.DrawButton();
    SwitchModeButton.DrawButton();
    ChangeBrushButton.DrawButton();
    
    DrawHamburger(16, 198);
}

void DrawWindowsMenu()
{
    myScreen.fillScreen(BLACK);
    
    DrawHamburger(16, 198);
}

void DrawChromeMenu()
{
    myScreen.fillScreen(BLACK);
    
    DrawHamburger(16, 198);
}

void DrawYouTubeMenu()
{
    myScreen.fillScreen(BLACK);

    DrawHamburger(16, 198);
}

void DrawHamburger(int x, int y)
{
    myScreen.fillRoundRect(x, y, 32, 6, 2, WHITE);
    myScreen.fillRoundRect(x, y + 10, 32, 6, 2, WHITE);
    myScreen.fillRoundRect(x, y + 20, 32, 6, 2, WHITE);
}

void DrawWindowsLogo(int x, int y)
{
        //Top left rectangle
      myScreen.fillTriangle(y + 10, x + 15, y + 10, x + 30, y + 28, x + 13, GetColor(255,255,255));
      myScreen.fillTriangle(y + 28, x + 13, y + 10, x + 30, y + 28, x + 30, GetColor(255,255,255));

      //Top right rectangle
      myScreen.fillTriangle(
      y+ 31, 
      x+ 12,
       
      y+ 31, 
      x+ 30,
      
      y+ 54, 
      x+ 9, 
      GetColor(255,255,255));
      
      myScreen.fillTriangle(
      y+ 54, 
      x+ 9,
       
      y+ 54, 
      x+ 30,
       
      y+ 31, 
      x+ 30, 
      GetColor(255,255,255));

      //Bottom left rectangle
      myScreen.fillTriangle(
      y+ 10, 
      x+ 33,
       
      y+ 10, 
      x+ 48,
      
      y+ 28, 
      x+ 33, 
      GetColor(255,255,255));
      
      myScreen.fillTriangle(
      y+ 28, 
      x+ 33,
       
      y+ 10, 
      x+ 48,
       
      y+ 28, 
      x+ 51, 
      GetColor(255,255,255));

      //Bottom right rectangle
      myScreen.fillTriangle(
      y+ 31, 
      x+ 33,
       
      y+ 54, 
      x+ 33,
      
      y+ 31, 
      x+ 51, 
      GetColor(255,255,255));
      
      myScreen.fillTriangle(
      y+ 54, 
      x+ 33,
       
      y+ 31, 
      x+ 51,
       
      y+ 54, 
      x+ 54, 
      GetColor(255,255,255));
}

void DrawChromeLogo(int x, int y)
{
    myScreen.fillCircle(x+32, y+32, 23, WHITE);
    myScreen.drawCircle(x+32, y+32, 10, BLACK);
    myScreen.drawCircle(x+32, y+32, 24, BLACK);

    myScreen.drawLine(x+13, y+19, x+23, y+35, BLACK);
    myScreen.drawLine(x+31, y+22, x+53, y+22, BLACK);
    myScreen.drawLine(x+39, y+39, x+31, y+55, BLACK);
}

void DrawYouTubeLogo(int x, int y)
{
    myScreen.fillRoundRect(x + 10, y + 17, 48, 32, 3, RED);
    myScreen.fillTriangle(
      x + 27, y + 25, 
      x + 27, y + 38, 
      x +  39, y + 32, 
      WHITE);
}

void DrawVSLogo(int x, int y)
{
    myScreen.drawRect(x, y, 64, 64, VS_PURPLE);
    myScreen.fillRect(x+1, y+1, 62, 62, WHITE);
    myScreen.fillRect(x+10, y+11, 43, 42, VS_PURPLE);    
    
    myScreen.fillTriangle(
    x+2, y+2, 
    x + 11, y + 22, 
    x + 41, y + 11,     
    WHITE);
    
    myScreen.fillTriangle(
    x + 10, y + 43, 
    x + 10, y + 53, 
    x + 41, y + 53,     
    WHITE);
    
    myScreen.fillTriangle(
    x + 16, y + 20, 
    x + 25, y + 28, 
    x + 42, y + 11,     
    WHITE);
    
    myScreen.fillTriangle(
    x + 16, y + 28, 
    x + 16, y + 37, 
    x + 21, y + 32,     
    WHITE);

    myScreen.fillTriangle(
    x + 32, y + 32, 
    x + 42, y + 25, 
    x + 42, y + 41,     
    WHITE);

    myScreen.fillTriangle(
    x + 25, y + 37, 
    x + 16, y + 45, 
    x + 42, y + 53,     
    WHITE);

    myScreen.fillTriangle(
    x + 44, y + 11, 
    x + 53, y + 11, 
    x + 53, y + 16,     
    WHITE);

    myScreen.fillTriangle(
    x + 53, y + 49, 
    x + 53, y + 53, 
    x + 45, y + 53,     
    WHITE);
}

