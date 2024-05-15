#include <hidcomposite.h>
#include <usbhub.h>
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>
#include <Keyboard.h>
#include <Mouse.h>

#include <avr/pgmspace.h>
#include <Usb.h>
#include <usbhub.h>
#include <avr/pgmspace.h>
#include <hidboot.h>
#include <hiduniversal.h>
#include <Keyboard.h>
#include <Mouse.h>


USB Usb;
HIDUniversal               HidMouse(&Usb);


struct {
    uint8_t buttons;
    int8_t x;
    int8_t y;
    int8_t wheel;
} mouseReport;

const byte buffSize = 32;
char inputSeveral[buffSize];

byte maxChars;

int inputInt = 0;
float inputFloat = 0.0;
char inputCsvString[12];
int x;
int y;
int dx;
int dy;
int dxn;
int dyn;
int index = 0;
int num_size = 0;
int jump = 5;

String myString;  
int j = 0; 
int c = 0; 
int e = 0;
int arr[2]; 
int arrv[8]; 

void MoveMouseToXY(long x, long y) {
  long max = max(abs(x), abs(y));
  int count = (int) (max / 127);
  signed char stepX = x / (count + 1);
  signed char stepY = y / (count + 1);
  for (int i = 0; i < count; i++) {
    Mouse.begin();
    Mouse.move(stepX, stepY);
    Mouse.end();
  }
  signed char resX = x - (stepX * count);
  signed char resY = y - (stepY * count);
  if (resX != 0 || resY != 0) {
    Mouse.begin();
    Mouse.move(resX, resY);
    Mouse.end();
  }
}


class MouseRptParser : public MouseReportParser {
protected:
    virtual void OnMouseMove        (MOUSEINFO* mi);
    virtual void OnLeftButtonUp     (MOUSEINFO* mi);
    virtual void OnLeftButtonDown   (MOUSEINFO* mi);
    virtual void OnRightButtonUp    (MOUSEINFO* mi);
    virtual void OnRightButtonDown  (MOUSEINFO* mi);
    virtual void OnMiddleButtonUp   (MOUSEINFO* mi);
    virtual void OnMiddleButtonDown (MOUSEINFO* mi);
    virtual void OnWheelMove        (MOUSEINFO *mi);
    virtual void OnX1ButtonUp       (MOUSEINFO *mi);
    virtual void OnX1ButtonDown     (MOUSEINFO *mi);
    virtual void OnX2ButtonUp       (MOUSEINFO *mi);
    virtual void OnX2ButtonDown     (MOUSEINFO *mi);
};
void MouseRptParser::OnMouseMove(MOUSEINFO* mi)        {
  MoveMouseToXY(mi->dX, mi->dY);
};
void MouseRptParser::OnLeftButtonUp(MOUSEINFO* mi)     {
  Mouse.begin();
  Mouse.release(MOUSE_LEFT);
  Mouse.end();
};
void MouseRptParser::OnLeftButtonDown(MOUSEINFO* mi)   {
  Mouse.begin();
  Mouse.press(MOUSE_LEFT);
  Mouse.end();
};
void MouseRptParser::OnRightButtonUp(MOUSEINFO* mi)    {
  Mouse.begin();
  Mouse.release(MOUSE_RIGHT);
  Mouse.end();
};
void MouseRptParser::OnRightButtonDown(MOUSEINFO* mi)  {
  Mouse.begin();
  Mouse.press(MOUSE_RIGHT);
  Mouse.end();
};
void MouseRptParser::OnMiddleButtonUp(MOUSEINFO* mi)   {
  Mouse.begin();
  Mouse.release(MOUSE_MIDDLE);
  Mouse.end();
};
void MouseRptParser::OnMiddleButtonDown(MOUSEINFO* mi) {
  Mouse.begin();
  Mouse.press(MOUSE_MIDDLE);
  Mouse.end();
};
void MouseRptParser::OnWheelMove(MOUSEINFO *mi)        {
  Mouse.begin();
  Mouse.move(0, 0, mi->dZ);
  Mouse.end();
};
void MouseRptParser::OnX1ButtonUp(MOUSEINFO *mi)       {
  Keyboard.release('o');
};
void MouseRptParser::OnX1ButtonDown(MOUSEINFO *mi)     {
  Keyboard.press('o');
};
void MouseRptParser::OnX2ButtonUp(MOUSEINFO *mi)       {
  Keyboard.release('p');
};
void MouseRptParser::OnX2ButtonDown(MOUSEINFO *mi)     {
  Keyboard.press('p');
};

MouseRptParser Prs;

void setup()
{
    Mouse.begin();
    Serial.begin(115200);
    Serial.setTimeout(1);
    Serial.println("Start");

    if (Usb.Init() == -1)
        Serial.println("OSC did not start.");
        delay(50);

    if (!HidMouse.SetReportParser(0, &Prs))
        ErrorMessage<uint8_t>(PSTR("SetReportParser"), 1);
}

void loop()
{
    if (Serial.available())
    {

        String data = Serial.readString();
        if (data == "shoot")
        {
            Mouse.click();
        }

        else if (data.substring(0, 6) == "silent")
        {
            data.remove(0, 6);
            index = 0;
            num_size = data.indexOf(":", index);
            dx = data.substring(index, num_size).toInt();
            data.remove(0, num_size + 1);
            dy = data.toInt();
            dxn = dx * -1;
            dyn = dy * -1;

            if (dx > 0)
            {
                while (dx > 127)
                {
                    dx -= 127;
                    Mouse.move(127, 0);
                }
                Mouse.move(dx, 0);
            }
            else if (dx < 0)
            {
                while (dx < -127)
                {
                    dx += 127;
                    Mouse.move(-127, 0);
                }
                Mouse.move(dx, 0);
            }
            if (dy >= 0)
            {
                while (dy > 127)
                {
                    dy -= 127;
                    Mouse.move(0, 127);
                }
                Mouse.move(0, dy);
            }
            else if (dy <= 0)
            {
                while (dy < -127)
                {
                    dy += 127;
                    Mouse.move(0, -127);
                }
                Mouse.move(0, dy);
            }
            Mouse.click();
            if (dxn > 0)
            {
                while (dxn > 127)
                {
                    dxn -= 127;
                    Mouse.move(127, 0);
                }
                Mouse.move(dxn, 0);
            }
            else if (dxn < 0)
            {
                while (dxn < -127)
                {
                    dxn += 127;
                    Mouse.move(-127, 0);
                }
                Mouse.move(dxn, 0);
            }
            if (dyn > 0)
            {
                while (dyn > 127)
                {
                    dyn -= 127;
                    Mouse.move(0, 127);
                }
                Mouse.move(0, dyn);
            }
            else if (dyn < 0)
            {
                while (dyn < -127)
                {
                    dyn += 127;
                    Mouse.move(0, -127);
                }
                Mouse.move(0, dyn);
            }
        }

        else
        {
            index = 0;
            num_size = data.indexOf(":", index);
            dx = data.substring(index, num_size).toInt();
            data.remove(0, num_size + 1);
            dy = data.toInt();
            // Serial.println(dx+":"+dy);
            if (dx > 0)
            {
                while (dx > jump)
                {
                    dx -= jump;
                    Mouse.move(jump, 0);
                }
                Mouse.move(dx, 0);
            }
            else if (dx < 0)
            {
                while (dx < -jump)
                {
                    dx += jump;
                    Mouse.move(-jump, 0);
                }
                Mouse.move(dx, 0);
            }
            if (dy >= 0)
            {
                while (dy > jump)
                {
                    dy -= jump;
                    Mouse.move(0, jump);
                }
                Mouse.move(0, dy);
            }
            else if (dy <= 0)
            {
                while (dy < -jump)
                {
                    dy += jump;
                    Mouse.move(0, -jump);
                }
                Mouse.move(0, dy);
            }
        
        }
    }
    else{
        Usb.Task();
        
    }
    
}
