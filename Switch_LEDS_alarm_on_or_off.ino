void  Switch_LEDS_alarm_on_or_off() {
  // convert maximum brightness from percent to integer

  PWM_Steps_LED1 = (PWM_Steps / 100 * max_brightness_LED1);
  PWM_Steps_LED2 = (PWM_Steps / 100 * max_brightness_LED2);

  // if the turning_LED1_on flag is HIGH, recalculate the brightness of LED1
  if (turning_alarm_LED1_on == HIGH) {

    // calculate a new value for the brightness of LED1, being careful not to exceed the bounds of the scale
    LED1_time_elapsed = millis() - LED1_millis_at_turn_on;
    LED1_alarm_brightness_turning_on = calculate_LED_brightness_linear(LED1_time_elapsed, LED1_alarm_brightness_turning_off, PWM_Steps_LED1,
                                 LED1_alarm_turn_on_time, HIGH);
    // Switch the flag turning_LED1_on off when the LED has reached full brightness
    if (LED1_alarm_brightness_turning_on >= PWM_Steps_LED1) {
      LED1_alarm_brightness_turning_on = PWM_Steps_LED1;
      turning_alarm_LED1_on = LOW;
      Serial.println("turning_LED1_on = LOW");
      //Serial.println (LED1_brightness_turning_on);
    }

    // Write the brightness to the pin
    analogWrite16(LED_PIN1, LED1_alarm_brightness_turning_on);

  }
  if (turning_alarm_LED1_off == HIGH)  {

    LED1_time_elapsed = millis() - LED1_millis_at_turn_off;
    LED1_alarm_brightness_turning_off = calculate_LED_brightness_linear(LED1_time_elapsed, LED1_alarm_brightness_turning_on, PWM_Steps_LED1,
                                  LED1_alarm_turn_off_time, LOW);
    if (LED1_alarm_brightness_turning_off <= 0) {
      LED1_alarm_brightness_turning_off = 0;
      turning_alarm_LED1_off = LOW;
      Serial.println("turning_LED1_off = LOW");
      //  Serial.println (LED1_brightness_turning_off);
    }

    analogWrite16(LED_PIN1, LED1_alarm_brightness_turning_off);
  }

  if (turning_alarm_LED2_on == HIGH) {

    LED2_time_elapsed = millis() - LED2_millis_at_turn_on,
    LED2_alarm_brightness_turning_on = calculate_LED_brightness_linear(LED2_time_elapsed, LED2_alarm_brightness_turning_off, PWM_Steps_LED2,
                                 LED2_alarm_turn_on_time , HIGH);

    if (LED2_alarm_brightness_turning_on >= PWM_Steps_LED2) {
      LED2_alarm_brightness_turning_on = PWM_Steps_LED2;
      turning_alarm_LED2_on = LOW;
      Serial.println("turning_LED2_on = LOW");
      Serial.println (LED2_brightness_turning_on);
    }
    analogWrite16(LED_PIN2, LED2_alarm_brightness_turning_on);
  }

  if (turning_alarm_LED2_off == HIGH)  {

    LED2_time_elapsed = millis() - LED2_millis_at_turn_off,
    LED2_alarm_brightness_turning_off = calculate_LED_brightness_linear(LED2_time_elapsed, LED2_alarm_brightness_turning_on, PWM_Steps_LED2,
                                  LED2_alarm_turn_off_time , LOW);

    if (LED2_alarm_brightness_turning_off <= 0) {
      LED2_alarm_brightness_turning_off = 0;
      turning_alarm_LED2_off = LOW;
      Serial.println("turning_LED2_off = LOW");
      Serial.println (LED2_brightness_turning_off);
    }
    analogWrite16(LED_PIN2, LED2_alarm_brightness_turning_off);
  }
}
