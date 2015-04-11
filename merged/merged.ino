#include <OneWire.h>
#include <DallasTemperature.h>
#include <Servo.h> 
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>
#include <EEPROM.h>

#include <OneWire.h>
#include <DallasTemperature.h>

Servo myservo;

// Data wire is plugged into pin 3 on the Arduino
#define ONE_WIRE_BUS 3

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Assign the addresses of your 1-Wire temp sensors.
// See the tutorial on how to obtain these addresses:
// http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html

DeviceAddress water_thermometer = {   0x28, 0xFF, 0xD3, 0x63, 0x54, 0x14, 0x00, 0x72 };

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

float set_temp = 87.0;
float current_temp = 50.0;
const float max_set_temp = 100.0;
const float min_set_temp = 70.0;


////Stub for temp measurement
//uint8_t get_current_temp(void)
//{
//  float temp = get_temp_fahrenheit();
//  return 57;
//}

void print_current_temp()
{
  lcd.setCursor(0, 1);
  lcd.print("Cur Temp: ");
  lcd.print(current_temp);
  lcd.print("F");
}

void print_set_temp()
{
  lcd.setCursor(0,0);
  lcd.print("Set Temp: ");
  lcd.print(set_temp);
  lcd.print("F");
}

float get_temp_fahrenheit(DeviceAddress deviceAddress)
{
  sensors.requestTemperatures();
  float tempC = sensors.getTempC(deviceAddress);

  if (tempC == -127.00) {
    Serial.print("Error getting temperature");
    return 0;
  }
  const float fahrenheit_temp = DallasTemperature::toFahrenheit(tempC);

  return fahrenheit_temp;
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

void setup()
{
  // start serial port
  Serial.begin(9600);
  // Start up the library
  sensors.begin();
  // set the resolution to 10 bit (good enough?)
  sensors.setResolution(water_thermometer, 10);
  
  myservo.attach(8);

  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);

  int time = millis();
  print_set_temp();
  print_current_temp();
  lcd.setBacklight(BLUE);  
}


void loop()
{
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

  current_temp = get_temp_fahrenheit(water_thermometer);
  if(current_temp > set_temp) {
    myservo.write(20);
  }
  else {
    myservo.write(160);
  }
  
}
