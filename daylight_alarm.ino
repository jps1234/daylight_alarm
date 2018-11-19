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
#include <EEPROM.h>
//#include <assert.h> // something to do with testing

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
Alarm_on_or_off = LOW,        //starting point for alarm state
alarm_is_on = LOW;



// set alarm values - probably dont need seconds, month or year setting week day would be nice
// Set to 20:30 Sunday September 2, 2018:
static signed char           // type int8_t, range -128 to 127
alarm_hour = 00,
alarm_minute = 5,
alarm_day = 1,               // day: Sunday=1, Monday=2, ..., Saturday=7
alarm_date = 2,
alarm_month = 9,
alarm_year = 18,

alarm_off_delay = 5; // how long till the alarm turns its self off should be adjustable

int
set_alarm_hour = alarm_hour,
set_alarm_minute = alarm_minute;

// set time values (used when setting the clock)
// Set to 00:00 Sunday September 2, 2018:
static signed char           // type int8_t, range -128 to 127
set_time_second = 0,
set_time_minute = 0,
set_time_hour = 0,
set_time_day = 1,               // day: Sunday=1, Monday=2, ..., Saturday=7
set_time_date = 2,
set_time_month = 9,
set_time_year = 18;

// days of the week
const String days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const String months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// for every time the second rolls over to do something
static signed char lastSecond = -1;
static unsigned long rpt(REPEAT_FIRST);  // a variable time that is used to drive the repeats for long presses

// states for the menu
enum states_t {HOME = 0, MENU_SET_ALARM, MENU_SET_TIME, SET_ALARM, SAVE_ALARM, SET_TIME, SAVE_TIME};
static states_t STATE = HOME;

// modes for setting the time and alarm time
enum clock_modes {SET_DAY = 1, SET_DATE, SET_MONTH, SET_YEAR, SET_HOUR, SET_MINUTE};
const unsigned char num_clock_modes = 6;
static unsigned char clock_mode = 1;

// modes for the alarm
enum alarm_modes {DAILY = 1, WEEKDAYS, ONCE};
const unsigned char num_alarm_modes = 3;
static unsigned char alarm_mode = 1;

// text to display on the oled screen
static String lineone;
static String linefour;
const String blankline = "                  ";

int
LED1_brightness_turning_on = 0, //brightness of LED when turning on
LED1_brightness_turning_off = 0,//brightness of LED when turning off
LED2_brightness_turning_on = 0, //brightness of LED when turning on
LED2_brightness_turning_off = 0, //brightness of LED when turning off

LED1_alarm_brightness_turning_on = 0, //brightness of LED when turning on
LED1_alarm_brightness_turning_off = 0,//brightness of LED when turning off
LED2_alarm_brightness_turning_on = 0, //brightness of LED when turning on
LED2_alarm_brightness_turning_off = 0, //brightness of LED when turning off

//max_brightness_LED1 = EEPROM.read(10), // out of 100 relates to pwm steps below...
//max_brightness_LED2 = EEPROM.read(11), // out of 100 relates to pwm steps below...
max_brightness_LED1 = 0, // out of 100 relates to pwm steps below...
max_brightness_LED2 = 0, // out of 100 relates to pwm steps below...

PWM_Steps_LED1 = 0, //  PWM_Steps/100 * max_brightness_LED1
PWM_Steps_LED2 = 0, //as percent of PWM Steps
PWM_Steps = 32767; //sent to ICR1 register. less than 32,767 otherwise code wraps around and breaks
//PWM_Steps = 65535; // max value 0xffff or 65535. higher the value the slower the refresh rate

long
LED1_turn_on_time = 4000, // time in ms that light takes to turn on
LED1_turn_off_time = 3000, // time in ms that light takes to turn off
LED2_turn_on_time  = 4000, // time in ms that light takes to turn on
LED2_turn_off_time  = 4000, // time in ms that light takes to turn off

LED1_alarm_turn_on_time = 100000, // time in ms that light takes to turn on
LED1_alarm_turn_off_time = 3000, // time in ms that light takes to turn off
LED2_alarm_turn_on_time  = 8000, // time in ms that light takes to turn on
LED2_alarm_turn_off_time  = 4000; // time in ms that light takes to turn off

static bool
turning_LED1_on = LOW, // records for turning led on from button press or other
turning_LED1_off = LOW,
turning_LED2_on = LOW,
turning_LED2_off = LOW,

turning_alarm_LED1_on = LOW, // records for turning led on from button press or other
turning_alarm_LED1_off = LOW,
turning_alarm_LED2_on = LOW,
turning_alarm_LED2_off = LOW;

// time elapsed since the last time switch was triggered
unsigned long
LED1_millis_at_turn_on = 0L, //time in millis when the button is pressed
LED1_millis_at_turn_off = 0L, //time in millis when the button is released
LED2_millis_at_turn_on = 0L, //time in millis when the button is pressed
LED2_millis_at_turn_off = 0L, //time in millis when the button is released
LED1_time_elapsed = 0L,  // time spent turning on or off
LED2_time_elapsed = 0L;


void setup()
{
  delay(1000);
  // Use the serial monitor to view output
  Serial.begin(9600);

  // Call rtc.begin([cs]) to initialize the library
  // The chip-select pin should be sent as the only parameter
  rtc.begin(DS13074_CS_PIN);
  //rtc.autoTime();//set time don't run all the time it starts or it resets the time



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

  // check if the alarm on or LED on switch is pressed at start-up
  myBtn5.read(); // to enable the alarm
  if (myBtn5.isPressed()) //start up check of switch
    Alarm_on_or_off = HIGH,
    Serial.println ("button 5 pressed at start-up");

  myBtn6.read(); // turns LED on as light
  if (myBtn6.isPressed())
    LED1_millis_at_turn_on = millis(),
    turning_LED1_on = HIGH,
    turning_LED1_off = LOW,
    LED2_millis_at_turn_on = millis(),
    turning_LED2_on = HIGH,
    turning_LED2_off = LOW,
    Serial.println ("button 6 pressed at start-up");

  // convert maximum brightness from percent to integer
  // if menu is enabled to set brightness value move this

  // read values from eeprom and check if first turn on
  /* uncooment the following to set EEPROM for the first time
  EEPROM locations used
   EEPROM.write(0, 0); // daylight saving flag check what month to start on
  1 Alarm hour
  2 Alarm Minute

  EEPROM.write(10, 100);10 LED1 max brightness out of 100
  EEPROM.write(11, 100);11 LED2 max brightness out of 100

*/
  alarm_hour = EEPROM.read(1);
  set_alarm_hour = alarm_hour;
  alarm_minute = EEPROM.read(2);
  set_alarm_minute = alarm_minute;

max_brightness_LED1 = EEPROM.read(10); // out of 100 relates to pwm steps below...
max_brightness_LED2 = EEPROM.read(11); // out of 100 relates to pwm steps below...

  
} // end of setup
void loop()
{

  // read the buttons
  myBtn1.read(); //Keypad left button
  myBtn2.read(); //Keypad middle button
  myBtn3.read(); //Keypad right button
  myBtn4.read(); // detect if display is closed and turn off
  myBtn5.read(); // to enable the alarm
  myBtn6.read(); // turns LED on as light


  if (myBtn4.wasPressed())
    oled.ssd1306WriteCmd(SSD1306_DISPLAYOFF),
                         Serial.println ("button 4 pressed");
  else if (myBtn4.wasReleased())
    oled.ssd1306WriteCmd(SSD1306_DISPLAYON),
                         Serial.println ("button 4 released");

  //

  if (myBtn5.wasPressed())
    Alarm_on_or_off = HIGH,
    printoled = !printoled,
    Serial.println ("button 5 pressed");

  else if (myBtn5.wasReleased())
    printoled = !printoled,
    Alarm_on_or_off = LOW,

    LED1_millis_at_turn_off = millis(),
    turning_alarm_LED1_on = LOW,
    turning_alarm_LED1_off = HIGH,
    LED2_millis_at_turn_off = millis(),
    turning_alarm_LED2_on = LOW,
    turning_alarm_LED2_off = HIGH,
    /*
        LED1_millis_at_turn_off = millis(),
        turning_LED1_on = LOW,
        turning_LED1_off = HIGH,
        LED2_millis_at_turn_off = millis(),
        turning_LED2_on = LOW,
        turning_LED2_off = HIGH,
    */

    print_all_data (),
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
    turning_LED1_on = LOW,
    turning_LED1_off = HIGH,
    LED2_millis_at_turn_off = millis(),
    turning_LED2_on = LOW,
    turning_LED2_off = HIGH,


    Serial.println ("button 6 released");



  // Call rtc.update() to update all rtc.seconds(), rtc.minutes(),
  // etc. return functions.
  rtc.update();

  if (rtc.second() != lastSecond) // If the second has changed
  {
    lastSecond = rtc.second(); // Update lastSecond value
    printoled = HIGH;
    /*Serial.print ("state = ");
      Serial.print (STATE);
      Serial.print (" ");
    */
    print_all_data ();
    printTime(); // Print the new time to serial
  }


  Update_Display(); // runs every second or if display flag is updated
  Menu_State_Machine (); // for scrolling through the menu
  check_alarm_time (); // is it time to turn on the alarm
  check_alarm_time_turn_off (); // is it time to turn off the alarm
  Switch_LEDS_on_or_off(); // uses button 6
  Switch_LEDS_alarm_on_or_off(); // uses button 5 and check_alarm_time_turn_off
  daylight_saving ();

} //End of loop

void daylight_saving () {
  int temp_time_second = rtc.second ();
  int temp_time_minute = rtc.minute();
  int temp_time_hour = rtc.hour();
  int temp_time_day = rtc.day(); // Sunday=1, Monday=2, ..., Saturday=7.
  int temp_time_date = rtc.date();
  int temp_time_month = rtc.month();
  int temp_time_year = rtc.year();

  if (temp_time_day == 1 && temp_time_month == 10 && temp_time_date >= 25 && temp_time_hour == 3 && EEPROM.read(0) == 1)
  {
    //get the current time but set hour back to 2 am
    temp_time_hour = 2; //rtc.hour();
    rtc.setTime(temp_time_second, temp_time_minute, temp_time_hour, temp_time_day, temp_time_date, temp_time_month, temp_time_year); //added 2019-10-19 this may not work check rtc.second?
    EEPROM.write(0, 0);
    Serial.print ("daylight oct met ");
  }

  if (temp_time_day == 1 && temp_time_month == 3 && temp_time_date >= 25 && temp_time_hour == 2 && EEPROM.read(0) == 0)

  {
    temp_time_hour = 3; //rtc.hour()
    rtc.setTime(temp_time_second, temp_time_minute, temp_time_hour, temp_time_day, temp_time_date, temp_time_month, temp_time_year); //added 2019-10-19 this may not work check rtc.second?
    EEPROM.write(0, 1);
  }
}

void print_all_data () {
  /*
    Serial.print ("EEPROM max_brightness_LED1 % : ");
  Serial.print (EEPROM.read(10));
  Serial.print (" | max_brightness_LED1 % : ");
  Serial.println (max_brightness_LED1);
  //  Serial.println (String(rtc.month()));
  //  Serial.println (String(rtc.day()));
  //  Serial.println (rtc.date());
  //  Serial.print ("EEPROM 0 DST flag: ")
  //  Serial.println (EEPROM.read(0));
  Serial.print ("EEPROM 1 Alarm Hour: ");
  Serial.print (EEPROM.read(1));
  Serial.print (" alarm_hour : ");
  Serial.print (alarm_hour);
  Serial.print (" | EEPROM 2  Alarm Minute: ");
  Serial.print (EEPROM.read(2));
  Serial.print (" alarm_minute : ");
  Serial.println (alarm_minute);

  Serial.print ("OCR1A = ");
  Serial.print (OCR1A);
  Serial.print (" : LED1_brightness_turning_on = ");
  Serial.print (LED1_brightness_turning_on);
  Serial.print (" : LED1_brightness_turning_off = ");
  Serial.print (LED1_brightness_turning_off);
  Serial.print (" : LED1_alarm_brightness_turning_on = ");
  Serial.print (LED1_alarm_brightness_turning_on);
  Serial.print (" : LED1_alarm_brightness_turning_off = ");
  Serial.println (LED1_alarm_brightness_turning_off);

  Serial.print ("OCR1B = ");
  Serial.print (OCR1B);
  Serial.print (" : LED2_brightness_turning_on = ");
  Serial.print (LED2_brightness_turning_on);
  Serial.print (" : LED2_brightness_turning_off = ");
  Serial.print (LED2_brightness_turning_off);
  Serial.print (" : LED2_alarm_brightness_turning_on = ");
  Serial.print (LED2_alarm_brightness_turning_on);
  Serial.print (" : LED2_alarm_brightness_turning_off = ");
  Serial.println (LED2_alarm_brightness_turning_off);
*/
}


void check_alarm_time () {
  if (alarm_is_on == LOW && Alarm_on_or_off == HIGH)
  {
    if (rtc.hour() == alarm_hour && rtc.minute() == alarm_minute)
    {
      alarm_is_on = HIGH,
      LED1_millis_at_turn_on = millis(),
      turning_alarm_LED1_on = HIGH,
      turning_alarm_LED1_off = LOW,
      LED2_millis_at_turn_on = millis(),
      turning_alarm_LED2_on = HIGH,
      turning_alarm_LED2_off = LOW;

      Serial.println ("alarm met leds should be on");

    }
  }
}


void check_alarm_time_turn_off () {
  // if (alarm_is_on == HIGH && Alarm_on_or_off == HIGH)
  if (alarm_is_on == HIGH) // in case the the alarm switch is  tured  off
  {
    if (rtc.hour() == alarm_hour && rtc.minute() == (alarm_minute + alarm_off_delay))
    {
      alarm_is_on = LOW,
      LED1_millis_at_turn_off = millis(),
      turning_alarm_LED1_on = LOW,
      turning_alarm_LED1_off = HIGH,
      LED2_millis_at_turn_off = millis(),
      turning_alarm_LED2_on = LOW,
      turning_alarm_LED2_off = HIGH;

      Serial.println ("alarm time to turn off met leds should be off");

    }
  }
}
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
  /*
    Serial.print ("OCR1A = ");
    Serial.print (OCR1A);
    Serial.print (" : LED1_brightness_turning_on = ");
    Serial.print (LED1_brightness_turning_on);
    Serial.print (" : LED1_brightness_turning_off = ");
    Serial.println (LED1_brightness_turning_off);
  */
}


