/*
*****************************************************************************
  Sources
  DS3234_RTC_Demo.ino
  https://github.com/sparkfun/SparkFun_DS3234_RTC_Arduino_Library
  Monochrome OLEDs based on SSD1306 drivers
  https://github.com/greiman/SSD1306Ascii
  Arduino Button Library
  https://github.com/JChristensen/JC_Button
****************************************************************************
  github working branch changes reset
*/

#include <SPI.h>
#include <SparkFunDS3234RTC.h>
#include <Wire.h> //display
#include <JC_Button.h>

#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

//OLED
#define I2C_ADDRESS 0x3C // spi port of ss1306 display
SSD1306AsciiAvrI2c oled;

// Pin assignments in order around arduino micro
// pin 0 - N/C TXD1 only
// pin 1 - N/C RXD1 only
// pin 2 - SDA for OLED
// pin 3 - SCL for OLED
// pin 4 - top input
const byte BUTTON5_PIN(4);
// pin 5 - N/C (pwm)
// pin 6 - N/C (pwm)
// pin 7 - top input
const byte BUTTON6_PIN(7);
// pin 8 -
#define DS13074_CS_PIN 8 // DeadOn RTC Chip-select pin
// pin 9 - PWM Control for LED driver is this warm or cool?
const byte   LED_PIN1 = 9;
// pin 10 - PWM Control for LED driver is this warm or cool?
const byte   LED_PIN2 = 10;
// pin 16 - MOSI RTC
// pin 14 - MISO RTC
// pin 15 - SCK RTC
// pins A0, A1, A2, A3  - Analogue pins for buttons
// connect a button from these pins to ground (internal pull-ups enabled)
const byte BUTTON1_PIN(A0), BUTTON2_PIN(A1), BUTTON3_PIN(A2), BUTTON4_PIN(A3);

// define the buttons 5 & 6 have longer bounce as toggle switch
Button myBtn1(BUTTON1_PIN), myBtn2(BUTTON2_PIN), myBtn3(BUTTON3_PIN), myBtn4(BUTTON4_PIN), myBtn5(BUTTON5_PIN, 200), myBtn6(BUTTON6_PIN, 200);

const unsigned long REPEAT_FIRST(500),          // ms required before repeating on long press
      REPEAT_INCR(100),           // repeat interval for long press
      REPEAT_INCR_FAST(20),
      FAST_THRESHOLD(2000);

static bool
printoled = LOW,            // flag to update screen
SPEED1 = LOW,                // for speeding up the counter after the button is pressed for a long time
Alarm_on_or_off = LOW;        //starting point for alarm state


// set alarm values - probably dont need seconds, month or year setting week day would be nice
int alarmhour = 20;
int alarmminute = 30;
int alarmsecond = 00;
// Set to Sunday September 2, 2018:
int alarmday = 1; // Sunday=1, Monday=2, ..., Saturday=7.
int alarmdate = 2;
int alarmmonth = 9;
int alarmyear = 18;

static int8_t lastSecond = -1;   // for every time the second rolls over to do something
static unsigned long rpt(REPEAT_FIRST);              // a variable time that is used to drive the repeats for long presses
enum states_t {WAIT, MENU_SET_ALARM, MENU_SET_TIME, MENU_SET_EXIT, SET_ALARM, SET_TIME, INCR, DECR};   // states for the state machine
static states_t STATE;              // current state machine state

int
LED1_brightness_turning_on = 0, //brightness of LED when turning on
LED1_brightness_turning_off = 0,//brightness of LED when turning off
LED2_brightness_turning_on = 0, //brightness of LED when turning on
LED2_brightness_turning_off = 0, //brightness of LED when turning off

max_brightness_LED1 = 100, // out of 100 relates to pwm steps below...
max_brightness_LED2 = 100, // out of 100 relates to pwm steps below...
PWM_Steps_LED1 = 0, //  PWM_Steps/100 * max_brightness_LED1
PWM_Steps_LED2 = 0, //as percent of PWM Steps
PWM_Steps = 32767; //sent to ICR1 register. less than 32,767 otherwise code wraps around and breaks
//PWM_Steps = 65535; // max value 0xffff or 65535. higher the value the slower the refresh rate
const long
LED1_turn_on_time = 4000, // time in ms that light takes to turn on
LED1_turn_off_time = 3000, // time in ms that light takes to turn off
LED2_turn_on_time  = 4000, // time in ms that light takes to turn on
LED2_turn_off_time  = 4000; // time in ms that light takes to turn off

static bool
turning_LED1_on = LOW, // records for turning led on from button press or other
turning_LED1_off = LOW,
turning_LED2_on = LOW,
turning_LED2_off = LOW;

// time elapsed since the last time switch was triggered
unsigned long
LED1_millis_at_turn_on = 0L, //time in millis when the button is pressed
LED1_millis_at_turn_off = 0L, //time in millis when the button is released
LED2_millis_at_turn_on = 0L, //time in millis when the button is pressed
LED2_millis_at_turn_off = 0L, //time in millis when the button is released
LED1_time_elapsed = 0L,  //
LED2_time_elapsed = 0L;


void setup()
{
  delay(1000);
  // Use the serial monitor to view output
  Serial.begin(9600);

  // Call rtc.begin([cs]) to initialize the library
  // The chip-select pin should be sent as the only parameter
  rtc.begin(DS13074_CS_PIN);
  rtc.autoTime();//set time


  //  rtc.update();   // Update time/date values, so we can set alarms
  Wire.begin();
  Wire.setClock(400000L);

  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.clear();

  // Start buttons
  myBtn1.begin();              // initialize the button object
  myBtn2.begin();
  myBtn3.begin();
  myBtn4.begin();
  myBtn5.begin();
  myBtn6.begin();

  setupPWM16();
  // Start PWM Outputs
  pinMode(LED_PIN1, OUTPUT);   // set the LED pin as an output
  pinMode(LED_PIN2, OUTPUT);   // set the LED pin as an output
  analogWrite16(LED_PIN1, 0);  // should start it as off
  analogWrite16(LED_PIN2, 0);

} // end of setup


void loop()
{
  // read the buttons
  myBtn1.read();
  myBtn2.read();
  myBtn3.read();
  myBtn4.read();
  myBtn5.read();
  myBtn6.read();

  // using is pressed to account for when it is turned on maybe a flag would be better
  if (myBtn4.wasPressed())
    oled.ssd1306WriteCmd(SSD1306_DISPLAYOFF),
                         Serial.println ("button 4 pressed");
  else if (myBtn4.wasReleased())
    oled.ssd1306WriteCmd(SSD1306_DISPLAYON),
                         Serial.println ("button 4 released");

  if (myBtn5.wasPressed())
    Alarm_on_or_off = HIGH,
    printoled = !printoled,
    Serial.println ("button 5 pressed");
  else if (myBtn5.wasReleased())
    printoled = !printoled,
    Alarm_on_or_off = LOW,
    Serial.println ("button 5 released");

  if (myBtn6.wasPressed())
    LED1_millis_at_turn_on = millis(),
    turning_LED1_on = HIGH,
    turning_LED1_off = LOW,
    LED2_millis_at_turn_on = millis(),
    turning_LED2_on = HIGH,
    turning_LED2_off = LOW,

    Serial.println ("button 6 pressed");

  if (myBtn6.wasReleased())
    LED1_millis_at_turn_off = millis(),
    turning_LED1_off = HIGH,
    turning_LED1_on = LOW,
    LED2_millis_at_turn_off = millis(),
    turning_LED2_off = HIGH,
    turning_LED2_on = LOW,

    Serial.println ("button 6 released");

  Switch_LEDS_on_or_off();
  Update_Display();
  State_machine();

  rtc.update();
  if (rtc.second() != lastSecond) // If the second has changed
  {
    lastSecond = rtc.second(); // Update lastSecond value
    printoled = !printoled;
    Serial.print ("state = ");
    Serial.print (STATE);
    Serial.print (" ");
    printTime(); // Print the new time to serial
  }

}
//End of loop

void setupPWM16() {
  TCCR1A = bit (WGM11) | bit (COM1B1) | bit (COM1A1); // fast PWM, TOP = ICR1, Enable OCR1A and OCR1B as outputs clear on compare
  TCCR1B = bit (WGM13) | bit (WGM12) | bit (CS10);   // fast PWM,  CS10 no prescaler p.s. WGM11, WGM12, WGM12 means TOP = ICR1
  ICR1 = PWM_Steps;       /* TOP counter value */
}

void analogWrite16(uint8_t pin, uint16_t val) //
{
  switch (pin) {
    case  LED_PIN1: OCR1A = val; break;
    case  LED_PIN2: OCR1B = val; break;

  }
  Serial.print ("OCR1A = ");
  Serial.print (OCR1A);
  Serial.print (" : LED1_brightness_turning_on = ");
  Serial.print (LED1_brightness_turning_on);
  Serial.print (" : LED1_brightness_turning_off = ");
  Serial.println (LED1_brightness_turning_off);
}


