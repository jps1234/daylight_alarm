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
max_brightness_LED1 = 40, //relates to pwm steps below...
max_brightness_LED2 = 40, //as percent of PWM Steps
PWM_Steps_LED1 = 0, //relates to pwm steps below...
PWM_Steps_LED2 = 0, //as percent of PWM Steps
//PWM_Steps = 65535; //sent to ICR1 register max value 0xffff or 65535. higher the value the slower the refresh rate
PWM_Steps = 32767; //less than

const long LED1_on_time = 4000; // time in ms that light takes to turn on
const long LED1_off_time = 500; // time in ms that light takes to turn off
const long LED2_on_time = 4000; // time in ms that light takes to turn on
const long LED2_off_time = 4000; // time in ms that light takes to turn off

static bool
turning_LED1_on = LOW,
turning_LED1_off = LOW,
turning_LED2_on = LOW,
turning_LED2_off = LOW;

static int
LED1_brightness_on = 0,
LED1_brightness_off = 0,
LED2_brightness_on = 0,
LED2_brightness_off = 0;

// time elapsed since the last time switch was triggered
static unsigned long
LED1_dimmerStart = 0L,
LED1_dimmerStop = 0L,
LED2_dimmerStart = 0L,
LED2_dimmerStop = 0L,
LED1_time_elapsed = 0L,
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
    LED1_dimmerStart = millis(),
    turning_LED1_on = HIGH,
    turning_LED1_off = LOW,
    //LED2_dimmerStart = millis(),
    //turning_LED2_on = HIGH,
    //turning_LED2_off = LOW,

    Serial.println ("button 6 pressed");

  if (myBtn6.wasReleased())
    LED1_dimmerStop = millis(),
    turning_LED1_off = HIGH,
    turning_LED1_on = LOW,
    //LED2_dimmerStop = millis(),
    //turning_LED2_off = HIGH,
    //turning_LED2_on = LOW,

    Serial.println ("button 6 released");


  Switch_LEDS(LED1_dimmerStart, LED1_dimmerStop, LED2_dimmerStart, LED2_dimmerStop);
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

void  Switch_LEDS(unsigned long LED1_dimmerStart, unsigned long LED1_dimmerStop, unsigned long LED2_dimmerStart, unsigned long LED2_dimmerStop)
{
  // turn leds on and off
  // inputs:
  // LED1_dimmerStart - time at the last time switch was triggered for LED1 to turn on
  // LED1_dimmerStop - time at the last time switch was triggered for LED1 to turn off
  // LED2_dimmerStart - time at the last time switch was triggered for LED2 to turn on
  // LED2_dimmerStop - time at the last time switch was triggered for LED2 to turn off

  // brightness at the last time the LEDS were in switching on/off


  // convert maximum brightness from percent to integer
  PWM_Steps_LED1 = (PWM_Steps / 100 * max_brightness_LED1);
  PWM_Steps_LED2 = (PWM_Steps / 100 * max_brightness_LED2);

  // if the turning_LED1_on flag is HIGH, recalculate the brightness of LED1
  if (turning_LED1_on == HIGH) {

    // calculate a new value for the brightness of LED1, being careful not to exceed the bounds of the scale
    LED1_time_elapsed = millis() - LED1_dimmerStart;
    //LED1_brightness_on = LED1_brightness_off + (PWM_Steps_LED1 * LED1_time_elapsed / LED1_on_time);
    LED1_brightness_on = calculate_LED_brightness_linear(LED1_time_elapsed, LED1_brightness_off, PWM_Steps_LED1,
                         LED1_on_time, HIGH);
    // Switch the flag turning_LED1_on off when the LED has reached full brightness
    if (LED1_brightness_on >= PWM_Steps_LED1) {
      LED1_brightness_on = PWM_Steps_LED1;
      turning_LED1_on = LOW;
      Serial.println("turning_LED1_on = LOW");
      //Serial.println (LED1_brightness_on);
    }

    // Write the brightness to the pin
    analogWrite16(LED_PIN1, LED1_brightness_on);

  }

  // same for the other pins (NEED A FUNCTION HERE AS THIS IS MASSIVELY REPETITIVE!)
  if (turning_LED1_off == HIGH)  {

    LED1_time_elapsed = millis() - LED1_dimmerStop,
    // LED1_brightness_off = LED1_brightness_on - (PWM_Steps_LED1 * LED1_time_elapsed / LED1_off_time);
    LED1_brightness_off = calculate_LED_brightness_linear(LED1_time_elapsed, LED1_brightness_on, PWM_Steps_LED1,
                          LED1_off_time, LOW);

    if (LED1_brightness_off <= 0) {
      LED1_brightness_off = 0;
      turning_LED1_off = LOW;
      Serial.println("turning_LED1_off = LOW");
      //  Serial.println (LED1_brightness_off);
    }
    
    analogWrite16(LED_PIN1, LED1_brightness_off);
  }


  if (turning_LED2_on == HIGH) {

    LED2_time_elapsed = millis() - LED2_dimmerStart,
    LED2_brightness_on = LED2_brightness_off + (PWM_Steps_LED2 * LED2_time_elapsed / LED2_on_time);

    if (LED2_brightness_on >= PWM_Steps_LED2) {
      LED2_brightness_on = PWM_Steps_LED2;
      turning_LED2_on = LOW;
      Serial.println("turning_LED2_on = LOW");
      Serial.println (LED2_brightness_on);
    }
    analogWrite16(LED_PIN2, LED2_brightness_on);
  }

  if (turning_LED2_off == HIGH)  {

    LED2_time_elapsed = millis() - LED2_dimmerStop,
    LED2_brightness_off = LED2_brightness_on - (PWM_Steps_LED2 * LED2_time_elapsed / LED2_off_time);

    if (LED2_brightness_off <= 0) {
      LED2_brightness_off = 0;
      turning_LED2_off = LOW;
      Serial.println("turning_LED2_off = LOW");
      Serial.println (LED2_brightness_off);
    }
    analogWrite16(LED_PIN2, LED2_brightness_off);
  }
}
int calculate_LED_brightness_linear(unsigned long t, int start_brightness, int maximum_brightness,
                                    unsigned long total_time, boolean increasing) {
  // calculate increasing/decreasing LED brightness according to a linear function
  // inputs:
  //   t - time elapsed
  //   start_brightness - LED brightness at time t=0
  //   maximum_brightness - maximum allowed LED brightness
  //   total_time - time taken for LED brightness to go from 0 to maximum_brightness
  //   increasing - HIGH if LED brightness is increasing, LOW otherwise
  // returns:
  //   brightness - new integer value of LED brightness

  int brightness = 0;

  // sign should be +1 if brightness is increasing and -1 otherwise
  int sign = -1;
  if (increasing) {
    sign = 1;
  }

  brightness = start_brightness + sign * maximum_brightness * t / total_time;

  return brightness;
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
  Serial.print ("OCR1A = ");
  Serial.print (OCR1A);
  Serial.print (" : LED1_brightness_on = ");
  Serial.print (LED1_brightness_on);
  Serial.print (" : LED1_brightness_off = ");
  Serial.println (LED1_brightness_off);
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

void Update_Display()
{ if (printoled == HIGH)      // something has changed and its time to update screen
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
  }
}

void State_machine() {
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
