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
  oled.print(days[rtc.day() - 1] + " ");      // remember that index of days starts at 0 (rather than 1)
  oled.print(String(rtc.date()) + " ");
  oled.print(months[rtc.month() - 1] + " ");  // similarly index of months starts at 0 (rather than 1)
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
  oled.print(days[set_time_day - 1] + " ");        // remember that index of days starts at 0 (rather than 1)
  oled.setInvertMode (false);

  oled.setInvertMode (clock_mode == SET_DATE);
  oled.print(String(set_time_date) + " ");         // Print date
  oled.setInvertMode (false);

  oled.setInvertMode (clock_mode == SET_MONTH);
  oled.print(months[set_time_month - 1] + " ");    // similarly index of months starts at 0 (rather than 1)
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

