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
