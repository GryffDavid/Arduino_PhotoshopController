#include <TouchScreen.h>
#include <TFTScreen.h>
#include <Keyboard.h>
//#include <Fonts/FreeSans9pt7b.h>
#include <RotaryEncoder.h>
#include <Mouse.h>

#define DARK_BLUE 627
#define LIGHT_BLUE 1599
#define win10Col 986
#define BLACK 0x0000
#define WHITE 0xFFFF
#define RED 63488
#define VS_PURPLE 22898
#define CRM_YEL 65248

#define MINPRESSURE 20
#define MAXPRESSURE 1000

TFTScreen myScreen;
TouchScreen ts = TouchScreen(6, A1, A2, 7, 300);
TSPoint tp;
uint16_t xpos, ypos;

class cBtn
{
  public:
    cBtn(byte pin) { buttonPin = pin; }
    
    byte buttonPin;
    bool buttonState;
    bool lastBtnState;
    unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 50;    
};

//Physical buttons
cBtn topBtn(1);
cBtn middleBtn(3);
cBtn bottomBtn(2);
cBtn rotaryBtn(0);

//PS Menu Btns
TFTButton MainMenuBtn;
TFTButton SwitchModeBtn;
TFTButton NewLayerBtn;
TFTButton ChangeBrushBtn;
TFTButton DeselectBtn;
TFTButton TransformBtn;
TFTButton MarqueeBtn;

//Main Menu Btns
TFTButton PhotoshopBtn;
TFTButton WindowsBtn;
TFTButton ChromeBtn;
TFTButton YouTubeBtn;

TFTButton MMBtns[5];

RotaryEncoder encoder(A4, A5);

bool CurrentPSInputMode; //True = Zoom, False = BrushSize
bool CurrentBrushState; //True = Brush, False = Eraser

enum MenuState { MainMenu, PSMenu, WindowsMenu, ChromeMenu, YouTubeMenu  };
MenuState CurrentMenuState;

void setup(void)
{
    CurrentMenuState = MainMenu;
    CurrentPSInputMode = true;
    
    Serial.begin(9600);

    pinMode(12, OUTPUT); //Buzzer pin
    pinMode(topBtn.buttonPin, INPUT);    
    pinMode(middleBtn.buttonPin, INPUT);
    pinMode(bottomBtn.buttonPin, INPUT);
    
    myScreen.begin(37671);
    myScreen.setRotation(1);
    myScreen.fillScreen(BLACK);
    //myScreen.setFont(&FreeSans9pt7b);
    myScreen.setTextSize(1);
    myScreen.setTextColor(WHITE);
    
    MainMenuBtn.InitButton(&myScreen, 16, 198 , 32, 26, WHITE, LIGHT_BLUE, WHITE, "");  
    
    //PS Menu Btns
    SwitchModeBtn.InitButton(&myScreen, 16, 16, 150, 75, WHITE, LIGHT_BLUE, WHITE, "Brush Size");
    ChangeBrushBtn.InitButton(&myScreen, 16, 107, 150, 75, WHITE, LIGHT_BLUE, WHITE, "Brush");
    NewLayerBtn.InitButton(&myScreen, 182, 16, 93, 75, WHITE, LIGHT_BLUE, WHITE, "New Layer");
    DeselectBtn.InitButton(&myScreen, 182, 107, 93, 75, WHITE, LIGHT_BLUE, WHITE, "Deselect");
    TransformBtn.InitButton(&myScreen, 291, 16, 75, 80, WHITE, LIGHT_BLUE, WHITE, "Transform");
    MarqueeBtn.InitButton(&myScreen, 291, 107, 75, 80, WHITE, LIGHT_BLUE, WHITE, "Transform");
    
    //Adjust hue (Ctrl-U)
    //Marquee tool
    //Transform selection (Ctrl-T)
    
        
    //Main Menu Btns
    PhotoshopBtn.InitButton(&myScreen, 8, 16, 64, 64, LIGHT_BLUE, DARK_BLUE, LIGHT_BLUE, "Ps");
    WindowsBtn.InitButton(&myScreen, 88, 16, 64, 64, WHITE, win10Col, win10Col, "");
    ChromeBtn.InitButton(&myScreen, 168, 16, 64, 64, WHITE, CRM_YEL, WHITE, "");
    YouTubeBtn.InitButton(&myScreen, 248, 16, 64, 64, RED, WHITE, WHITE, "");

    MMBtns[0] = PhotoshopBtn;
    MMBtns[1] = WindowsBtn;
    MMBtns[2] = ChromeBtn;
    MMBtns[3] = YouTubeBtn;
    
    DrawMainMenu();      
    delay(500);
}

void loop()
{
    //Handle Main Menu Btn on submenus
    switch (CurrentMenuState)
    {
        case ChromeMenu:
        case WindowsMenu:
        case PSMenu:
        case YouTubeMenu:
        {
            if (UpdateTFTBtn(MainMenuBtn) == true)
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
            for (byte i = 0; i < 5; i++)
            {
                if (UpdateTFTBtn(MMBtns[i]) == true)
                {
                    tone(12, 3500, 5);

                    switch (i)
                    {
                      case 0:
                        CurrentMenuState = PSMenu;
                        DrawPSMenu();
                      break;

                      case 1:
                        CurrentMenuState = WindowsMenu;
                        DrawWindowsMenu();
                      break;

                      case 2:
                        CurrentMenuState = ChromeMenu;
                        DrawChromeMenu();
                      break;

                      case 3:
                        CurrentMenuState = YouTubeMenu;
                        DrawYouTubeMenu();
                      break;
                    }
                }
            }
        }
        break;
        
        case ChromeMenu:
        {
          
        }
        break;
    
        case WindowsMenu:
        {
            if (UpdatePhysicalBtn(rotaryBtn) == true)
            {
                Remote.mute();
                Remote.clear();
            }
        }
        break;
      
        case PSMenu:
        {
            //Top physical button
            if (digitalRead(middleBtn.buttonPin) == 1)
            {
                if (UpdatePhysicalBtn(topBtn) == true)
                {
                    Keyboard.press(KEY_LEFT_CTRL); 
                    Keyboard.press(KEY_LEFT_ALT);         
                    Keyboard.press('z');
                    Keyboard.releaseAll();
                }
            }
        
            //Middle physical button
            if (digitalRead(middleBtn.buttonPin) == 0)
            {
                if (UpdatePhysicalBtn(topBtn) == true)
                {
                    Keyboard.press(KEY_LEFT_CTRL); 
                    Keyboard.press(KEY_LEFT_SHIFT);         
                    Keyboard.press('z');
                    Keyboard.releaseAll();
                } else { Keyboard.press(' '); }
            }
            else
            {
                Keyboard.release(' ');
            }
        
            //Bottom physical button
            if (digitalRead(bottomBtn.buttonPin) == 0) { Keyboard.press(KEY_LEFT_ALT); } else { Keyboard.release(KEY_LEFT_ALT); }            

            if (UpdateTFTBtn(DeselectBtn) == true)
            {
                tone(12, 3500,5);
                Keyboard.press(KEY_LEFT_CTRL);        
                Keyboard.press('d');
                Keyboard.releaseAll();
            }

            //New Layer touch button
            if (UpdateTFTBtn(NewLayerBtn) == true)
            {
                tone(12, 3500,5);
                Keyboard.press(KEY_LEFT_CTRL); 
                Keyboard.press(KEY_LEFT_SHIFT);         
                Keyboard.press('n');
                Keyboard.press(KEY_RETURN);
                Keyboard.releaseAll();
            }
            
            //Switch Mode touch button
            if (UpdateTFTBtn(SwitchModeBtn) == true)
            {
                tone(12, 500, 5);
        
                switch(CurrentPSInputMode)
                {
                    case true:                    
                      CurrentPSInputMode = false;
                      SwitchModeBtn.ChangeLabel("Zoom");                    
                    break;
        
                    case false:                    
                      CurrentPSInputMode = true;
                      SwitchModeBtn.ChangeLabel("Brush Size");                    
                    break;
                }
                
                SwitchModeBtn.DrawButton();      
            }

            //Change brush mode
            if (UpdateTFTBtn(ChangeBrushBtn) == true)
            {
                switch(CurrentBrushState)
                {
                    case true:                    
                      tone(12, 500, 10);
                      Keyboard.print('e');
                      Keyboard.release('e');
                      ChangeBrushBtn.ChangeLabel("Eraser");
                      CurrentBrushState = false;                    
                    break;
        
                    case false:                    
                      tone(12, 2500, 5);
                      Keyboard.print('b');
                      Keyboard.release('b');
                      ChangeBrushBtn.ChangeLabel("Brush");
                      CurrentBrushState = true;                    
                    break;
                }
                
                ChangeBrushBtn.DrawButton();      
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
      switch (CurrentMenuState)
      {
        case PSMenu:
          switch(CurrentPSInputMode)
          {
              case true:
                if (pos > newPos) { Keyboard.print("["); }
                if (pos < newPos) { Keyboard.print("]"); }
              break;
      
              case false:              
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
              break;
          }
        break;

        case WindowsMenu:
          if (pos > newPos) { Remote.decrease(); Remote.clear(); }
          if (pos < newPos) { Remote.increase(); Remote.clear(); }
        break;
      }
              
      pos = newPos;
    }

    //Update touch screen
    tp = ts.getPoint();
    SetPins(); //This is necessary for the touch screen to work properly

    if (tp.z > MINPRESSURE && tp.z < MAXPRESSURE)
    {
        xpos = map(tp.y, 950, 180, 0, myScreen.width());
        ypos = map(tp.x, 170, 880, 0, myScreen.height());

        switch (CurrentMenuState)
        {
            case MainMenu:            
              for (byte i = 0; i < 5; i++)
              {
                  MMBtns[i].CheckButton(xpos, ypos);
              }            
            break;
          
            case PSMenu:
                SwitchModeBtn.CheckButton(xpos, ypos);
                NewLayerBtn.CheckButton(xpos, ypos);
                ChangeBrushBtn.CheckButton(xpos, ypos);
                DeselectBtn.CheckButton(xpos, ypos);
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
                MainMenuBtn.CheckButton(xpos, ypos);
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
                MainMenuBtn.CheckButton(-1, -1);
            }
            break;
        }
      
        switch (CurrentMenuState)
        {
            case MainMenu:            
              for (byte i = 0; i < 5; i++)
              {
                  MMBtns[i].CheckButton(-1,-1);
              }            
            break;
          
            case PSMenu:
            {
                SwitchModeBtn.CheckButton(-1, -1);
                NewLayerBtn.CheckButton(-1, -1);
                ChangeBrushBtn.CheckButton(-1, -1);
                DeselectBtn.CheckButton(-1, -1);
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

bool UpdateTFTBtn(TFTButton &tftBtn)
{
    int reading = tftBtn.pressed;
    bool result = false;
    
    if (reading != tftBtn.lastButtonState) { tftBtn.lastDebounceTime = millis(); }
    
    if ((millis() - tftBtn.lastDebounceTime) > tftBtn.debounceDelay) 
    {
      if (reading != tftBtn.buttonState) 
      {
        tftBtn.buttonState = reading;  
        if (tftBtn.buttonState == LOW) { result = true; }
      }
    }
    
    tftBtn.lastButtonState = reading;
    return result;
}

bool UpdatePhysicalBtn(cBtn &button)
{
    int reading = digitalRead(button.buttonPin);
    bool result = false;
    
    if (reading != button.lastBtnState) { button.lastDebounceTime = millis(); }
    
    if ((millis() - button.lastDebounceTime) > button.debounceDelay) 
    {
      if (reading != button.buttonState) 
      {
        button.buttonState = reading;  
        if (button.buttonState == LOW) { result = true; }
      }
    }
        
    button.lastBtnState = reading;
    return result;
}

void SetPins()
{
    pinMode(A2, OUTPUT);
    pinMode(A1, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
}

void DrawMainMenu()
{
    myScreen.fillScreen(BLACK);
  
    WindowsBtn.DrawButton();
    PhotoshopBtn.DrawButton();
    ChromeBtn.DrawButton();
    YouTubeBtn.DrawButton();
  
    DrawWindowsLogo(16, 88);
    DrawChromeLogo(168, 16);
    DrawYouTubeLogo(246, 16);
    DrawVSLogo(328, 16);
}

void DrawPSMenu()
{
    myScreen.fillScreen(BLACK);
    NewLayerBtn.DrawButton();
    SwitchModeBtn.DrawButton();
    ChangeBrushBtn.DrawButton();
    DeselectBtn.DrawButton();
    TransformBtn.DrawButton();
    MarqueeBtn.DrawButton();
    
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

void DrawHamburger(int x, byte y)
{
    myScreen.fillRoundRect(x, y, 32, 6, 2, WHITE);
    myScreen.fillRoundRect(x, y + 10, 32, 6, 2, WHITE);
    myScreen.fillRoundRect(x, y + 20, 32, 6, 2, WHITE);
}

void DrawWindowsLogo(int x, byte y)
{
        //Top left rectangle
      myScreen.fillTriangle(y + 10, x + 15, y + 10, x + 30, y + 28, x + 13, WHITE);
      myScreen.fillTriangle(y + 28, x + 13, y + 10, x + 30, y + 28, x + 30, WHITE);

      //Top right rectangle
      myScreen.fillTriangle(y + 31, x + 12, y + 31, x + 30, y + 54, x + 9, WHITE);      
      myScreen.fillTriangle(y + 54, x + 9, y + 54, x + 30, y + 31, x + 30, WHITE);

      //Bottom left rectangle
      myScreen.fillTriangle(y + 10, x + 33, y + 10, x + 48, y + 28, x + 33, WHITE);      
      myScreen.fillTriangle(y + 28, x + 33, y + 10, x + 48, y + 28, x + 51, WHITE);

      //Bottom right rectangle
      myScreen.fillTriangle(y + 31, x + 33, y + 54, x + 33, y + 31, x + 51, WHITE);      
      myScreen.fillTriangle(y + 54, x + 33, y + 31, x + 51, y + 54, x + 54, WHITE);
}

void DrawChromeLogo(int x, byte y)
{
    myScreen.fillCircle(x + 32, y + 32, 23, WHITE);
    myScreen.drawCircle(x + 32, y + 32, 10, BLACK);
    myScreen.drawCircle(x + 32, y + 32, 24, BLACK);

    myScreen.drawLine(x + 13, y + 19, x + 23, y + 35, BLACK);
    myScreen.drawLine(x + 31, y + 22, x + 53, y + 22, BLACK);
    myScreen.drawLine(x + 39, y + 39, x + 31, y + 55, BLACK);
}

void DrawYouTubeLogo(int x, byte y)
{
    myScreen.fillRoundRect(x + 10, y + 17, 48, 32, 3, RED);
    myScreen.fillTriangle(x + 27, y + 25, x + 27, y + 38, x +  39, y + 32, WHITE);
}

void DrawVSLogo(int x, byte y)
{
    myScreen.drawRect(x, y, 64, 64, VS_PURPLE);
    myScreen.fillRect(x + 1, y + 1, 62, 62, WHITE);
    myScreen.fillRect(x + 10, y + 11, 43, 42, VS_PURPLE);    
    myScreen.fillTriangle(x + 2, y + 2, x + 11, y + 22, x + 41, y + 11, WHITE);
    myScreen.fillTriangle(x + 10, y + 43, x + 10, y + 53, x + 41, y + 53, WHITE);
    myScreen.fillTriangle(x + 16, y + 20, x + 25, y + 28, x + 42, y + 11, WHITE);
    myScreen.fillTriangle(x + 16, y + 28, x + 16, y + 37, x + 21, y + 32, WHITE);
    myScreen.fillTriangle(x + 32, y + 32, x + 42, y + 25, x + 42, y + 41, WHITE);
    myScreen.fillTriangle(x + 25, y + 37, x + 16, y + 45, x + 42, y + 53, WHITE);
    myScreen.fillTriangle(x + 44, y + 11, x + 53, y + 11, x + 53, y + 16, WHITE);
    myScreen.fillTriangle(x + 53, y + 49, x + 53, y + 53, x + 45, y + 53, WHITE);
}

