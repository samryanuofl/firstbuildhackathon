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
DeviceAddress water_thermometer = {0x28, 0xFF, 0xD3, 0x63, 0x54, 0x14, 0x00, 0x72};

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
const float min_set_temp = 50.0;
const float temp_dead_band = 3.00;

//Directions for SERVO direction
const uint8_t HOT = 0x01;
const uint8_t COLD = 0x02;

const int HOT_INCREMENT_SIZE = 10;
const int COLD_INCREMENT_SIZE = -10;
const unsigned int SERVO_MAX_POSITION = 170;
const unsigned int SERVO_MIN_POSITION = 10;
const uint8_t SERVO_CENTER_POSITION = 90;

uint8_t servo_position = 80;

//These two pins used to select user profile
const int user_select_pin_1 = 12;
const int user_select_pin_2 = 13;
//Select the user number
uint8_t user = 0;
//Start address for user data in EEPROM
const unsigned int temperature_eeprom_start = 0;

uint8_t initial_user_number = 0;

uint8_t get_user_number_from_gpio(void)
{
  uint8_t user_select_1_val = 0;
  uint8_t user_select_2_val = 0;
  if (digitalRead(user_select_pin_1)) {
    user_select_1_val = 1;
  }
  if (digitalRead(user_select_pin_2)) {
    user_select_2_val = 1;
  }
  const uint8_t user_number = (user_select_2_val * 2) + user_select_1_val;
  return user_number;
}

void print_current_temp()
{
  lcd.setCursor(0, 1);
  lcd.print("Cur Temp: ");
  lcd.print(current_temp);
  lcd.print("F");
}

void print_set_temp()
{
  lcd.setCursor(0, 0);
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
  if (new_set_temp > max_set_temp) {
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
  if (new_set_temp < min_set_temp) {
    return false;
  }
  else {
    set_temp = new_set_temp;
    return true;
  }
}

uint8_t get_saved_temp(uint8_t user_number)
{
  const uint8_t saved_temp_eeprom_address = temperature_eeprom_start  + user_number;
  const uint8_t saved_set_temp = EEPROM.read(saved_temp_eeprom_address);
  return saved_set_temp;
}

void store_temp(uint8_t user_number, uint8_t value)
{
  const uint8_t saved_temp_eeprom_address = temperature_eeprom_start  + user_number;
  EEPROM.write(saved_temp_eeprom_address, value);
}

void adjust_servo(uint8_t water_temp_direction)
{
  if (water_temp_direction == HOT) {
    const uint8_t new_servo_position = servo_position + HOT_INCREMENT_SIZE;
    if ((new_servo_position < SERVO_MAX_POSITION) && (new_servo_position > SERVO_MIN_POSITION)) {
      servo_position = new_servo_position;
    }
  }
  else {
    const uint8_t new_servo_position = servo_position + COLD_INCREMENT_SIZE;
    if ((new_servo_position < SERVO_MAX_POSITION) && (new_servo_position > SERVO_MIN_POSITION)) {
      servo_position = new_servo_position;
    }
  }
}

uint8_t get_servo_from_temp(uint8_t temp_fahrenheit)
{
  #define num_points 12
  const uint8_t servo_positions[num_points] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120};
  const uint8_t temperatures[num_points] = {60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115};

  int i;
  uint8_t new_servo_position = 0;
  for (i = 0; i < sizeof(servo_positions); i++) {
    if (temp_fahrenheit > temperatures[i]) {
      new_servo_position = servo_positions[i];
    }
  }
  return new_servo_position;
}

void setup()
{
  //Pin setup
  pinMode(user_select_pin_1, INPUT);
  pinMode(user_select_pin_2, INPUT);

  // start serial port
  Serial.begin(9600);
  // Start up the library
  sensors.begin();
  // set the resolution to 10 bit (good enough?)
  sensors.setResolution(water_thermometer, 10);

  myservo.attach(8);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  //Read back saved "set_temp" value from EEPROM
  initial_user_number = get_user_number_from_gpio();
  const uint8_t saved_temp_eeprom_address = temperature_eeprom_start  + initial_user_number;
  const uint8_t saved_set_temp = EEPROM.read(saved_temp_eeprom_address);
  //if saved val is too high, set it to max
  if (saved_set_temp > max_set_temp) {
    EEPROM.write(saved_temp_eeprom_address, (uint8_t)max_set_temp);
  }
  set_temp = float(saved_set_temp);

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
    if (buttons & BUTTON_SELECT) {
      const uint8_t saved_temp_eeprom_address = temperature_eeprom_start  + initial_user_number;
      EEPROM.write(saved_temp_eeprom_address, (uint8_t)set_temp);
    }
  }

  Serial.print("Servo Position = ");
  Serial.println(servo_position);
  servo_position = get_servo_from_temp(set_temp);
  myservo.write(servo_position);
}

