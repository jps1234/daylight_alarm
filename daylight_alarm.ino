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


//RTC
// Configurable Pin Definitions //
#define DS13074_CS_PIN 10 // DeadOn RTC Chip-select pin
//OLED
#define I2C_ADDRESS 0x3C
SSD1306AsciiAvrI2c oled;

// Input pin assignments
const byte BUTTON1_PIN(A0),              // connect a button switch from this pin to ground
      BUTTON2_PIN(A1),              // connect a button switch from this pin to ground
      BUTTON3_PIN(A2),              // connect a button switch from this pin to ground
      BUTTON4_PIN(A3);              // connect a button switch from this pin to ground-not connected

Button myBtn1(BUTTON1_PIN), myBtn2(BUTTON2_PIN), myBtn3(BUTTON3_PIN);       // define the button

// Output pin assignments
const byte LED_PIN1(8),                // Output 1
      LED_PIN2(9);                // Output 2

// detecting button repeats
const unsigned long REPEAT_FIRST(500),          // ms required before repeating on long press
      REPEAT_INCR(100),           // repeat interval for long press
      REPEAT_INCR_FAST(20),
      FAST_THRESHOLD(2000);
static bool
printoled = LOW,            // flag to update screen
SPEED1 = LOW,                // for speeding up the counter after the button is pressed for a long time
led1State = LOW,            // a variable that keeps the current LED status
led2State = LOW;            // a variable that keeps the current LED status


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
//enum states_t {WAIT, INCR, DECR};       // states for the state machine old working
static states_t STATE;              // current state machine state



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
  myBtn2.begin();              // initialize the button object
  myBtn3.begin();              // initialize the button object

  // Start Outputs
  pinMode(LED_PIN1, OUTPUT);   // set the LED pin as an output
  pinMode(LED_PIN2, OUTPUT);   // set the LED pin as an output

}
//char lineone[16] = " "; // string for first line change[16] to length of line for neatness
//char linefour[16] = " ";
static bool Alarm_on_or_off = LOW;

void loop()
{


  myBtn1.read();                   // read the buttons
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
  }
  //   if (Alarm_on_or_off == HIGH)
  //       strcpy(lineone, "Alarm : ON   ");
  //   else if (Alarm_on_or_off == LOW)
  //       strcpy(lineone, "Alarm : OFF  ");

  if (printoled == HIGH)      // something has changed and its time to update screen
  {
    printoled = !printoled;
    oled.setInvertMode (false);
    oled.setFont(X11fixed7x14);
    oled.set1X();
    oled.setCursor(0, 0);
    //oled.print ("state = ");
    //oled.println (STATE);
    //line One
    if ((STATE == 0) && (Alarm_on_or_off == HIGH))
      oled.println("Alarm On       ");

    else if ((STATE == 0) && (Alarm_on_or_off == LOW))
      oled.println("Alarm Off       ");

    else if (STATE == 1)
      oled.println("Set Alarm      ");
    else if (STATE == 2)
      oled.println("Set Time       ");
    else if (STATE == 3)
      oled.println("Exit           ");
    else if (STATE == 4)
      oled.println("Setting Alarm  ");
    else if (STATE == 5)
      oled.println("Setting Time   ");
    else if (STATE == 6)
      oled.println("Setting Alarm  ");
    else if (STATE == 7)
      oled.println("Setting Alarm  ");

    OLEDprintTime (); //print to LCD
    OLEDprintAlarm();
    //line four
    if (STATE == 0)
      oled.println("Menu    On     Off");

    else if (STATE == 1)
      oled.println("Enter  Prev   Next");
    else if (STATE == 2)
      oled.println("Enter  Prev   Next");
    else if (STATE == 3)
      oled.println("Enter  Prev   Next");
    else if (STATE == 4)
      oled.println("Exit   Plus  Minus");
    else if (STATE == 5)
      oled.println("Exit   Plus  Minus");
    else if (STATE == 6)
      oled.println("Exit   Plus  Minus");
    else if (STATE == 7)
      oled.println("Exit   Plus  Minus");




    //     OLEDlinefour ();
  }

  switch (STATE)
  {
    case WAIT:                              // case 0 wait for a button event
      //           strcpy(linefour, "Menu");
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
      //          strcpy(lineone, "Set Alarm");
      if (myBtn1.wasPressed())
        STATE = SET_ALARM,
        printoled = !printoled;
      else if (myBtn2.wasPressed())
        STATE = MENU_SET_EXIT,
        printoled = !printoled;
      else if (myBtn3.wasPressed())
        STATE = MENU_SET_TIME ,
        printoled = !printoled;
      break;

    case MENU_SET_TIME:               // case 2
      //        using alarm code here for now
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
      //        using alarm code here for now
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


    case SET_ALARM:             // case 4
      if (myBtn1.wasPressed())
        STATE = WAIT,
        printoled = !printoled;
      else if (myBtn2.wasPressed())
        STATE = INCR;
      else if (myBtn3.wasPressed())
        STATE = DECR;
      else if (myBtn2.wasReleased())       // reset the long press interval
        rpt = REPEAT_FIRST,
        SPEED1 = LOW;
      else if (myBtn3.wasReleased())
        rpt = REPEAT_FIRST,
        SPEED1 = LOW;

      else if (myBtn2.pressedFor(rpt))     // check for long press
      {
        if (SPEED1 == HIGH) {
          rpt += REPEAT_INCR_FAST;
        }
        else {
          rpt += REPEAT_INCR;            // increment the long press interval
        }
        STATE = INCR;
        if (rpt > FAST_THRESHOLD) {
          SPEED1 = HIGH;
        }
      }

      else if (myBtn3.pressedFor(rpt))
      {
        if (SPEED1 == HIGH) {
          rpt += REPEAT_INCR_FAST;             // increment the long press interval
        }
        else {
          rpt += REPEAT_INCR;
        }
        STATE = DECR;
        if (rpt > FAST_THRESHOLD) {
          SPEED1 = HIGH;
        }
      }
      break;



    case SET_TIME:             // case 5
      if (myBtn1.wasPressed())
        STATE = WAIT,
        printoled = !printoled;

      break;

    case INCR:  // case 6
      alarmsecond++;    // increment the counter
      if (alarmsecond > 59) {  // but not more than the specified maximum
        alarmminute++;
        alarmsecond = 0;
      }
      printoled = !printoled;
      STATE = SET_ALARM;
      break;

    case DECR:   // case 7
      alarmsecond--;  // decrement the counter
      if (alarmsecond < 0) {  // but not less than the specified minimum
        alarmminute--;
        alarmsecond = 59;
      }
      printoled = !printoled;
      STATE = SET_ALARM;
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
void OLEDprintAlarm()
{
  oled.set1X();
  oled.print("Alarm : ");
  oled.print(String(alarmhour) + ":"); // Print hour
  if (alarmminute < 10)
    oled.print('0'); // Print leading '0' for second
  oled.print(String(alarmminute) + ":"); // Print min
  if (alarmsecond < 10)
    oled.print('0'); // Print leading '0' for second

  oled.println(String(alarmsecond)); // Print sec
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
