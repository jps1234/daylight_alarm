void Update_Display()
{ if (printoled == HIGH)      // something has changed and its time to update screen
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
        switch (clock_mode) {
          case SET_HOUR:
            oled.println(blankline);
            OLEDprintAlarm(true, false);
            oled.println("              Save");
            oled.println("Back  Scroll Enter");
          case SET_MINUTE:
            oled.println(blankline);
            OLEDprintAlarm(false, true);
            oled.println("              Save");
            oled.println("Back  Scroll Enter");
            break;
        }



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

