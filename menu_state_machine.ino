
void Menu_State_Machine ()
{

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
            alarm_hour_temp = incrementHour(alarm_hour_temp);
            break;
          case SET_MINUTE:
            alarm_minute_temp = incrementMinute(alarm_minute_temp);
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
        break;
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
        alarm_hour = alarm_hour_temp;
        alarm_minute = alarm_minute_temp;
        STATE = HOME;
        break;
      case SAVE_TIME:
        STATE = HOME;
        break;
    }
    printoled = HIGH;
  }
}

