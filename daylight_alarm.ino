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
  alarmhour = 20,
  alarmminute = 30,
  alarmsecond = 00,
  alarmday = 1,               // day: Sunday=1, Monday=2, ..., Saturday=7
  alarmdate = 2,
  alarmmonth = 9,
  alarmyear = 18;

// for every time the second rolls over to do something
static signed char lastSecond = -1;
static unsigned long rpt(REPEAT_FIRST);  // a variable time that is used to drive the repeats for long presses

// states for the state machine
enum states_t {WAIT, MENU_SET_ALARM, MENU_SET_TIME, MENU_SET_EXIT, SET_ALARM, SET_TIME};   // states for the state machine
//enum states_t {WAIT, INCR, DECR}; // states for the state machine old working
static states_t STATE;              // current state machine state

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

  //static char lineone[18];
  //static char linefour[18];
  static String lineone;
  static String linefour;
  
  static bool Alarm_on_or_off = LOW;
  
  myBtn1.read();                // read the buttons
  myBtn2.read();
  myBtn3.read();

  // Call rtc.update() to update all rtc.seconds(), rtc.minutes(),
  // etc. return functions.
  rtc.update();

  if (rtc.second() != lastSecond) // If the second has changed
  {
    lastSecond = rtc.second(); // Update lastSecond value
    printoled = !printoled;
    Serial.print ("state = ");
    Serial.print (STATE);
    Serial.print (" ");
    printTime(); // Print the new time to serial
    Serial.println (lineone);
    Serial.println (linefour);
  }
  //   if (Alarm_on_or_off == HIGH)
  //      strcpy(lineone, "Alarm : ON   ");
  //   else if (Alarm_on_or_off == LOW)
  //      strcpy(lineone, "Alarm : OFF  ");

  if (printoled == HIGH)      // something has changed and its time to update screen
  {
    printoled = !printoled;
    //oled.setInvertMode (false);
    oled.setFont(X11fixed7x14);
    //oled.set1X();
    //oled.setCursor(0, 0);
    oled.home();              // Set the cursor position to (0, 0).
    //oled.print ("state = ");
    //oled.println (STATE);

    //line One
    switch (STATE)
    {
      case WAIT:
        lineone = "Home     ";
        break;

      case MENU_SET_ALARM:
      case SET_ALARM:
        lineone = "Set Alarm";
        break;

      case MENU_SET_TIME:
      case SET_TIME:
        lineone = "Set Time ";
        break;

      case MENU_SET_EXIT:
        lineone = "Exit     ";
        break;        
    }
    assert (lineone.length() == 9);            // strings overwrite one another on the display, so should be the same length
    
    //line four
    switch (STATE)
    {    
      case WAIT:
        linefour = "Menu    On     Off";
        break;
        
      case MENU_SET_ALARM:
      case MENU_SET_TIME:
      case MENU_SET_EXIT:
        linefour = "Enter   Prev  Next";
        break;
        
      case SET_ALARM:
      case SET_TIME:
        linefour = "Exit   Plus  Minus";
        break;
    }
    assert (linefour.length() == 18);            // strings overwrite one another on the display, so should be the same length

    //Serial.println (lineone);
    // print to LCD
    oled.println(lineone);
    OLEDprintTime ();
    OLEDprintAlarm(Alarm_on_or_off);
    oled.println(linefour);

  }

  switch (STATE)
  {
    case WAIT:                           // case 0 wait for a button event
      // strcpy(linefour, "Menu");
      if (myBtn1.wasPressed())
        STATE = MENU_SET_ALARM,
        printoled = !printoled;
      else if (myBtn2.wasPressed())
        Alarm_on_or_off = HIGH,
        printoled = !printoled;
      else if (myBtn3.wasPressed())
        Alarm_on_or_off = LOW,
        printoled = !printoled;      // something changed tell the screen to update
      break;

    case MENU_SET_ALARM:                //case 1
      // strcpy(lineone, "Set Alarm");
      if (myBtn1.wasPressed())
        STATE = SET_ALARM,
        printoled = !printoled;
      else if (myBtn2.wasPressed())
        STATE = MENU_SET_EXIT,
        printoled = !printoled;
      else if (myBtn3.wasPressed())
        STATE = MENU_SET_TIME,
        printoled = !printoled;
      break;

    case MENU_SET_TIME:               // case 2
      // using alarm code here for now
      if (myBtn1.wasPressed())
        STATE = SET_TIME,
        printoled = !printoled;
      else if (myBtn2.wasPressed())
        STATE = MENU_SET_ALARM,
        printoled = !printoled;
      else if (myBtn3.wasPressed())
        STATE = MENU_SET_EXIT,
        printoled = !printoled;
      break;

    case MENU_SET_EXIT:             // case 3
      // using alarm code here for now
      if (myBtn1.wasPressed())
        STATE = WAIT,
        printoled = !printoled;
      else if (myBtn2.wasPressed())
        STATE = MENU_SET_TIME,
        printoled = !printoled;
      else if (myBtn3.wasPressed())
        STATE = MENU_SET_ALARM,
        printoled = !printoled;
      break;


    case SET_ALARM:             // case 4: Exit  Plus  Minus
      if (myBtn1.wasPressed())
        STATE = WAIT,
        printoled = !printoled;
      else if (myBtn2.wasPressed())
        incrementTime();
      else if (myBtn3.wasPressed())
        decrementTime();
      else if (myBtn2.wasReleased())       // reset the long press interval
        rpt = REPEAT_FIRST,
        SPEED = LOW;
      else if (myBtn3.wasReleased())
        rpt = REPEAT_FIRST,
        SPEED = LOW;

      else if (myBtn2.pressedFor(rpt))     // check for long press
      {
        printoled = !printoled;
        if (SPEED == HIGH) {
          rpt += REPEAT_INCR_FAST;
        }
        else {
          rpt += REPEAT_INCR;          // increment the long press interval
        }
        incrementTime();
        if (rpt > FAST_THRESHOLD) {
          SPEED = HIGH;
        }
      }

      else if (myBtn3.pressedFor(rpt))
      {
        printoled = !printoled;
        if (SPEED == HIGH) {
          rpt += REPEAT_INCR_FAST;     // increment the long press interval
        }
        else {
          rpt += REPEAT_INCR;
        }
        decrementTime();
        if (rpt > FAST_THRESHOLD) {
          SPEED = HIGH;
        }
      }
      break;

    case SET_TIME:             // case 5
      if (myBtn1.wasPressed()) {
        STATE = WAIT,
        printoled = !printoled;
      }
      break;
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

void stateMachine()
{
  
}

void incrementTime()
{
  alarmsecond++;          // increment the counter
  if (alarmsecond > 59)   // but not more than the specified maximum
    alarmminute++,
    alarmsecond = 0;
}

void decrementTime()
{
  alarmsecond--;         // decrement the counter
  if (alarmsecond < 0)   // but not less than the specified minimum
    alarmminute--,
    alarmsecond = 59;
}

void OLEDprintTime()
{

  oled.print("Time  : ");
  oled.print(String(rtc.hour()) + ":"); // Print hour
  if (rtc.minute() < 10)
    oled.print('0'); // Print leading '0' for minute
  oled.print(String(rtc.minute()) + ":"); // Print minute

  if (rtc.second() < 10)
    oled.print('0'); // Print leading '0' for second
  // oled.setInvertMode (true);
  oled.println(String(rtc.second())); // Print second
}

void OLEDprintAlarm(bool Alarm_on_or_off)
{
  //oled.set1X();  // font size
  oled.print("Alarm : ");
  if (Alarm_on_or_off == HIGH | STATE == SET_ALARM)
  {
    oled.print(String(alarmhour) + ":"); // Print hour
    if (alarmminute < 10)
      oled.print('0'); // Print leading '0' for second
    oled.print(String(alarmminute) + ":"); // Print min
    if (alarmsecond < 10)
      oled.print('0'); // Print leading '0' for second
    oled.println(String(alarmsecond)); // Print sec
  }
  else {
    oled.println("Off        ");
  }
}

void LCDprintButtonState()
{
  //  oled.print(bool(led1State) + ":"); //

}

void printTime() //Over serial
{
  Serial.print ("time = ");
  Serial.print(String(rtc.hour()) + ":"); // Print hour
  if (rtc.minute() < 10)
    Serial.print('0'); // Print leading '0' for minute
  Serial.print(String(rtc.minute()) + ":"); // Print minute
  if (rtc.second() < 10)
    Serial.print('0'); // Print leading '0' for second
  Serial.print(String(rtc.second())); // Print second

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
