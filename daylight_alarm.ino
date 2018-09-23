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
*/

#include <SPI.h>
#include <SparkFunDS3234RTC.h>
#include <Wire.h> //display
#include <JC_Button.h>

#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

#include <assert.h>
//#include <string>
 
  // Or you can use the rtc.setTime(s, m, h, day, date, month, year)
  // function to explicitly set the time:
  // e.g. 7:32:16 | Monday October 31, 2016:
  //rtc.setTime(16, 32, 7, 2, 31, 10, 16);  // Uncomment to manually set time

//RTC
// Configurable Pin Definitions //
#define DS13074_CS_PIN 10 // DeadOn RTC Chip-select pin
//OLED
#define I2C_ADDRESS 0x3C
SSD1306AsciiAvrI2c oled;

// Input pin assignments
const byte
  BUTTON1_PIN(4),              // connect a button switch from this pin to ground
  BUTTON2_PIN(5),              // connect a button switch from this pin to ground
  BUTTON3_PIN(6),              // connect a button switch from this pin to ground
  BUTTON4_PIN(7);              // connect a button switch from this pin to ground-not connected

Button myBtn1(BUTTON1_PIN), myBtn2(BUTTON2_PIN), myBtn3(BUTTON3_PIN);       // define the button

// Output pin assignments
const byte
  LED_PIN1(8),                // Output 1
  LED_PIN2(9);                // Output 2

// detecting button repeats
const unsigned long
  REPEAT_FIRST(500),          // ms required before repeating on long press
  REPEAT_INCR(100),           // repeat interval for long press
  REPEAT_INCR_FAST(20),
  FAST_THRESHOLD(2000);
static bool
  printoled = LOW,            // flag to update screen
  SPEED = LOW,                // for speeding up the counter after the button is pressed for a long time
  led1State = LOW,            // a variable that keeps the current LED status
  led2State = LOW;            // a variable that keeps the current LED status


// set alarm values - probably dont need seconds, month or year setting week day would be nice
// Set to 20:30 Sunday September 2, 2018:
static signed char           // type int8_t, range -128 to 127
  alarm_hour = 20,
  alarm_minute = 30,
  alarm_day = 1,               // day: Sunday=1, Monday=2, ..., Saturday=7
  alarm_date = 2,
  alarm_month = 9,
  alarm_year = 18;
static bool Alarm_on_or_off = LOW;

// set time values (used when setting the clock)
// Set to 00:00 Sunday September 2, 2018:
static signed char           // type int8_t, range -128 to 127
  set_time_hour = 0,
  set_time_minute = 0,
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
enum clock_modes {SET_DAY=1, SET_DATE, SET_MONTH, SET_YEAR, SET_HOUR, SET_MINUTE};
const unsigned char num_clock_modes = 6;
static unsigned char clock_mode = 1;

// modes for the alarm
enum alarm_modes {DAILY=1, WEEKDAYS, ONCE};
const unsigned char num_alarm_modes = 3;
static unsigned char alarm_mode = 1;

// text to display on the oled screen
//static char lineone[18];
//static char linefour[18];
static String lineone;
static String linefour;
const String blankline = "                  ";

void setup()
{
  // Use the serial monitor to view output
  Serial.begin(9600);

  // Call rtc.begin([cs]) to initialize the library
  // The chip-select pin should be sent as the only parameter
  rtc.begin(DS13074_CS_PIN);
  rtc.autoTime();              // set time

  // Update time/date values, so we can set alarms
  //  rtc.update();
  Wire.begin();
  Wire.setClock(400000L);

  // Start the oled display
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.clear();

  // Start buttons
  myBtn1.begin();              // initialize the button object
  myBtn2.begin();              // initialize the button object
  myBtn3.begin();              // initialize the button object

  // Start Outputs
  pinMode(LED_PIN1, OUTPUT);   // set the LED pin as an output
  pinMode(LED_PIN2, OUTPUT);   // set the LED pin as an output
}

void loop()
{
  
  myBtn1.read();                // read the buttons
  myBtn2.read();
  myBtn3.read();

  // menu state machine
  if (myBtn1.wasPressed()) {          // BUTTON 1: MENU/BACK: move between different menu screens
    switch (STATE) {
      case HOME:
        STATE = MENU_SET_ALARM;
        break;
      case MENU_SET_ALARM:
      case MENU_SET_TIME:
      case SAVE_TIME:
      case SAVE_ALARM:
        STATE = HOME;
        break;
      case SET_ALARM:
        STATE = MENU_SET_ALARM;
        break;
      case SET_TIME:
        STATE = MENU_SET_TIME;
        break;
      }
    printoled = HIGH;
  } 
  else if (myBtn2.wasPressed()) {    // BUTTON 2: SCROLL: scroll through lists/options
    switch (STATE)
    {
      case HOME:
        break;

      case MENU_SET_ALARM:
        STATE = MENU_SET_TIME;
        break;
      case MENU_SET_TIME:
        STATE = MENU_SET_ALARM;
        break;
      
      case SET_ALARM:
        switch (clock_mode) {
          case SET_HOUR:
            alarm_hour = incrementHour(alarm_hour);
            break;
          case SET_MINUTE:
            alarm_minute = incrementMinute(alarm_minute);
            break;
        }
        
      case SET_TIME:
        switch (clock_mode) {
          case SET_DAY:
            set_time_day = incrementDay(set_time_day);
            break;
          case SET_DATE:
            set_time_date = incrementDate(set_time_date);
            break;
          case SET_MONTH:
            set_time_month = incrementMonth(set_time_month);
            break;
          case SET_HOUR:
            set_time_hour = incrementHour(set_time_hour);
            break;
          case SET_MINUTE:
            set_time_minute = incrementMinute(set_time_minute);
            break;
        }
 
      case SAVE_TIME:
      case SAVE_ALARM:
        break;
    }
    printoled = HIGH;
  } 
  else if (myBtn3.wasPressed()) {           // BUTTON 3: ENTER: toggle alarm or select item
    switch (STATE)
    {
      case HOME:
        Alarm_on_or_off = !Alarm_on_or_off; // toggle the alarm on and off
        break;
        
      case MENU_SET_ALARM:
        STATE = SET_ALARM;                  // go into set alarm and start with clock_mode = SET_HOUR (skip the date)
        clock_mode = SET_HOUR;
        break;
      case SET_ALARM:
        if (clock_mode < num_clock_modes)   // scroll through the clock_modes, then go to save
          clock_mode++;
        else
          STATE = SAVE_ALARM;
        break;
        
      case MENU_SET_TIME:                   // go into set time and start with clock_mode = SET_DAY
        STATE = SET_TIME;
        clock_mode = 1;
        set_time_hour = rtc.hour();         // grab the current time as a starting point
        set_time_minute = rtc.minute();
        set_time_day = rtc.day();
        set_time_date = rtc.date();
        set_time_month = rtc.month();
        set_time_year = rtc.year();
        break;
      case SET_TIME:
        if (clock_mode < num_clock_modes)   // scroll through the clock_modes, then go to save
          clock_mode++;
        else
          STATE = SAVE_TIME;
        break;
      
      case SAVE_ALARM:                      // return to the home screen
      case SAVE_TIME:
        STATE = HOME;
        break;
    }
    printoled = HIGH;
  }

  // Call rtc.update() to update all rtc.seconds(), rtc.minutes(),
  // etc. return functions.
  rtc.update();

  if (rtc.second() != lastSecond) // If the second has changed
  {
    lastSecond = rtc.second(); // Update lastSecond value
    printoled = HIGH;
    Serial.print ("state = ");
    Serial.print (STATE);
    Serial.print (" ");
    printTime(); // Print the new time to serial
  }

  if (printoled == HIGH)      // something has changed and its time to update screen
  {
    printoled = LOW;
    oled.setFont(X11fixed7x14);
    //oled.set1X();
    oled.home();              // Set the cursor position to (0, 0).

    switch (STATE)
    {
      case HOME:
        OLEDprintDateTime ();
        if (Alarm_on_or_off)
          OLEDprintAlarm(false, false);
        else
          oled.println("Alarm : Off       ");
        oled.println("Menu         Alarm");
        break;

      case MENU_SET_ALARM:
        oled.setInvertMode (true);
        lineone = "   Set alarm      ";
        oled.println(lineone);
        oled.setInvertMode (false);
        oled.println("   Set time       ");
        oled.println(blankline);
        oled.println("Back  Scroll Enter");
        break;

      case MENU_SET_TIME:
        lineone = "   Set alarm      ";
        oled.println(lineone);
        oled.setInvertMode (true);
        oled.println("   Set time       ");
        oled.setInvertMode (false);
        oled.println(blankline);
        oled.println("Back  Scroll Enter");
        break;
        
      case SET_ALARM:
        oled.println(blankline);
        OLEDprintAlarm(true, false);
        oled.println("              Save");
        oled.println("Back  Scroll Enter");
        break;

      case SAVE_ALARM:
        oled.println(blankline);
        OLEDprintAlarm(false, false);
        oled.setInvertMode (true);
        oled.println("              Save");
        oled.setInvertMode (false);
        oled.println("Back         Enter");
        break;

      case SET_TIME:
        OLEDprintSetTime(clock_mode);
        oled.println("              Save");
        oled.println("Back  Scroll Enter");
        break;

      case SAVE_TIME:
        OLEDprintSetTime(clock_mode);
        oled.setInvertMode (true);
        oled.println("              Save");
        oled.setInvertMode (false);
        oled.println("Back         Enter");
        break;
    }

  }

}
//End of loop
/*
  void OLEDlineOne()
  {

  //   oled.print ("state = ");
  //   oled.print (STATE);
  //  oled.println(lineone);
   //oled.println(" : ");
  }
  //void OLEDlinefour()
  {//
  //  oled.println(linefour);
  }
*/

// functions for setting the date and time

signed char incrementToMax(signed char counter, signed char counter_min, signed char counter_max)
{
  counter++;          // increment the counter
  if (counter > counter_max)   // but not more than the specified maximum
    counter = counter_min;
  return counter;
}

signed char incrementAlarmMode(signed char alarm_mode)
{
  return incrementToMax(alarm_mode, 1, num_alarm_modes);
}

signed char incrementClockMode(signed char clock_mode)
{
  return incrementToMax(clock_mode, 1, num_clock_modes);
}

signed char incrementDay(signed char day_counter)
{
  return incrementToMax(day_counter, 1, 7);
}

signed char incrementDate(signed char date_counter)
{
  return incrementToMax(date_counter, 1, 31);
}

signed char incrementMonth(signed char month_counter)
{
  return incrementToMax(month_counter, 1, 12);
}

signed char incrementHour(signed char hour_counter)
{
  return incrementToMax(hour_counter, 0, 23);
}

signed char incrementMinute(signed char minute_counter)
{
  return incrementToMax(minute_counter, 0, 59);
}


// functions for printing the date and time to the oled

String timeToString(signed char time) {
  if (time < 10)
    return "0" + String(time);                // Print leading '0' when hour/minute/second less than 10
  else
    return String(time);
}

void OLEDprintDateTime()
{
  oled.print("  ");                           // Print date
  oled.print(days[rtc.day()-1] + " ");        // remember that index of days starts at 0 (rather than 1)
  oled.print(String(rtc.date()) + " "); 
  oled.print(months[rtc.month()-1] + " ");    // similarly index of months starts at 0 (rather than 1)
  oled.println(String(rtc.year()) + "   "); 
  
  oled.print("     ");                        // Print time
  oled.print(timeToString(rtc.hour()) + ":");       
  oled.print(timeToString(rtc.minute()) + ":");     
  oled.println(timeToString(rtc.second()) + "     "); 
}

void OLEDprintSetTime(unsigned char clock_mode)
{
  oled.print("  ");                             // Print day
  oled.setInvertMode (clock_mode == SET_DAY);
  oled.print(days[set_time_day-1] + " ");          // remember that index of days starts at 0 (rather than 1)
  oled.setInvertMode (false);

  oled.setInvertMode (clock_mode == SET_DATE);
  oled.print(String(set_time_date) + " ");         // Print date
  oled.setInvertMode (false);

  oled.setInvertMode (clock_mode == SET_MONTH);
  oled.print(months[set_time_month-1] + " ");      // similarly index of months starts at 0 (rather than 1)
  oled.setInvertMode (false);

  oled.setInvertMode (clock_mode == SET_YEAR);
  oled.println(String(set_time_year) + "   ");
  oled.setInvertMode (false);

  oled.print("     ");
  oled.setInvertMode (clock_mode == SET_HOUR);  // Print hour      
  oled.print(timeToString(set_time_hour)); 
  oled.setInvertMode (false);
  oled.print(":");
  
  oled.setInvertMode (clock_mode == SET_MINUTE);  // Print min
  oled.print(timeToString(set_time_minute)); 
  oled.setInvertMode (false);
  oled.println("     ");
}

void OLEDprintAlarm(bool highlightHour, bool highlightMin)
{
  oled.print("Alarm : ");
  
  oled.setInvertMode (highlightHour);         // Print hour
  oled.print(timeToString(alarm_hour)); 
  oled.setInvertMode (false);
  
  oled.print(":");
  
  oled.setInvertMode (highlightMin);          // Print min
  oled.print(timeToString(alarm_minute)); 
  oled.setInvertMode (false);
  oled.println("     ");
}

void LCDprintButtonState()
{
  //  oled.print(bool(led1State) + ":"); //

}

void printTime() //Over serial
{
  Serial.print ("time = ");
  Serial.print(String(rtc.hour()) + ":");
  Serial.print(timeToString(rtc.minute()) + ":");
  Serial.print(timeToString(rtc.second()));

  if (rtc.is12Hour()) // If we're in 12-hour mode
  {
    // Use rtc.pm() to read the AM/PM state of the hour
    if (rtc.pm()) Serial.print(" PM"); // Returns true if PM
    else Serial.print(" AM");
  }

  Serial.print(" | ");

  // Few options for printing the day, pick one:
  Serial.print(rtc.dayStr()); // Print day string
  //Serial.print(rtc.dayC()); // Print day character
  //Serial.print(rtc.day()); // Print day integer (1-7, Sun-Sat)
  Serial.print(" - ");

  Serial.print(String(rtc.date()) + "/" +    // (or) print date
               String(rtc.month()) + "/"); // Print month

  Serial.println(String(rtc.year()));        // Print year
}
