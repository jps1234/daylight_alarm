void Update_Display()
{ if (printoled == HIGH)      // something has changed and its time to update screen
  {
    printoled = LOW;
    oled.setFont(X11fixed7x14);
    //   oled.set1X();
    oled.home();              // Set the cursor position to (0, 0).

    switch (STATE)
    {
      case HOME:
        OLEDprintDateTime1 ();
        if (Alarm_on_or_off)
          OLEDprintAlarm(false, false);
        else
          oled.println("Alarm : Off        ");

        oled.print("L1: ");

        if (max_brightness_LED1 < 10)
          oled.print(' '); // Print leading '0'
        if (max_brightness_LED1 < 100)
          oled.print(' '); // Print leading '0'
        oled.print(max_brightness_LED1);
        oled.print("% ");
        
        oled.print("L2: ");
        if (max_brightness_LED2 < 10)
          oled.print(' '); // Print leading '0'
        if (max_brightness_LED2 < 100)
          oled.print(' '); // Print leading '0'
        oled.print(max_brightness_LED2);
        oled.println("%");

        oled.print("Menu");
        oled.println ("   dim  bright");


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
        /*     switch (clock_mode) {
               case SET_HOUR:
                 oled.println(blankline);
                 OLEDprintAlarm(true, false);
                 oled.println("              Save");
                 oled.println("Back  Scroll Enter");
               case SET_MINUTE:
                 oled.println(blankline);
                 OLEDprintAlarm(false, true);
                 }
        */
        OLEDprintSetAlarm(clock_mode); //new 2018-10-19
        oled.println("              Save");
        oled.println("Back  Scroll Enter");
        break;




      case SAVE_ALARM:
        oled.println(blankline);
        OLEDprintSetAlarm(false, false);
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

