/*********************

Example code for the Adafruit RGB Character LCD Shield and Library

This code displays text on the shield, and also reads the buttons on the keypad.
When a button is pressed, the backlight changes color.

**********************/

// include the library code:
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// These #defines make it easy to set the backlight color
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

uint8_t set_temp = 87;
const uint8_t max_set_temp = 100;
const uint8_t min_set_temp = 70;
void setup() {
  // Debugging output
  Serial.begin(9600);
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);

  int time = millis();
  print_set_temp();
  print_current_temp();
  lcd.setBacklight(BLUE);
}

//Stub for temp measurement
uint8_t get_current_temp()
{
  return 57; 
}

void print_current_temp()
{
  lcd.setCursor(0, 1);
  lcd.print("Cur Temp: ");
  lcd.print(get_current_temp());
  lcd.print("F");
}

void print_set_temp()
{

  lcd.setCursor(0,0);
  lcd.print("Set Temp: ");
  lcd.print(set_temp);
  lcd.print("F");
}

//Return false and don't increment if temp at max, otherwise increment and return true
boolean increment_set_temp(void)
{
  const uint8_t new_set_temp = set_temp + 1;
  if(new_set_temp > max_set_temp) {
    return false; 
  }
  else {
    set_temp = new_set_temp;
    return true;
  }
}


//Return false and don't decrement if temp at min, otherwise decrement and return true
boolean decrement_set_temp(void)
{
  const uint8_t new_set_temp = set_temp - 1;
  if(new_set_temp < min_set_temp) {
    return false; 
  }
  else {
    set_temp = new_set_temp;
    return true;
  }
}

uint8_t i=0;
void loop() {
  uint8_t buttons = lcd.readButtons();
  print_current_temp();
  if (buttons) {
    if (buttons & BUTTON_UP) {
      increment_set_temp();
      print_set_temp();
    }
    if (buttons & BUTTON_DOWN) {
      decrement_set_temp();
      print_set_temp();   
    }
  }
}
