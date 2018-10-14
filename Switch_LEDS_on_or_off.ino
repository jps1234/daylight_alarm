void  Switch_LEDS_on_or_off() {
    // convert maximum brightness from percent to integer
  PWM_Steps_LED1 = (PWM_Steps / 100 * max_brightness_LED1);
  PWM_Steps_LED2 = (PWM_Steps / 100 * max_brightness_LED2);

  // if the turning_LED1_on flag is HIGH, recalculate the brightness of LED1
  if (turning_LED1_on == HIGH) {

    // calculate a new value for the brightness of LED1, being careful not to exceed the bounds of the scale
    LED1_time_elapsed = millis() - LED1_millis_at_turn_on;
    LED1_brightness_turning_on = calculate_LED_brightness_linear(LED1_time_elapsed, LED1_brightness_turning_off, PWM_Steps_LED1,
                                 LED1_turn_on_time, HIGH);
    // Switch the flag turning_LED1_on off when the LED has reached full brightness
    if (LED1_brightness_turning_on >= PWM_Steps_LED1) {
      LED1_brightness_turning_on = PWM_Steps_LED1;
      turning_LED1_on = LOW;
      Serial.println("turning_LED1_on = LOW");
      //Serial.println (LED1_brightness_turning_on);
    }

    // Write the brightness to the pin
    analogWrite16(LED_PIN1, LED1_brightness_turning_on);

  }
  if (turning_LED1_off == HIGH)  {

    LED1_time_elapsed = millis() - LED1_millis_at_turn_off;
    LED1_brightness_turning_off = calculate_LED_brightness_linear(LED1_time_elapsed, LED1_brightness_turning_on, PWM_Steps_LED1,
                                  LED1_turn_off_time, LOW);
    if (LED1_brightness_turning_off <= 0) {
      LED1_brightness_turning_off = 0;
      turning_LED1_off = LOW;
      Serial.println("turning_LED1_off = LOW");
      //  Serial.println (LED1_brightness_turning_off);
    }

    analogWrite16(LED_PIN1, LED1_brightness_turning_off);
  }

  if (turning_LED2_on == HIGH) {

    LED2_time_elapsed = millis() - LED2_millis_at_turn_on,
    LED2_brightness_turning_on = calculate_LED_brightness_linear(LED2_time_elapsed, LED2_brightness_turning_off, PWM_Steps_LED2, LED2_turn_on_time , HIGH);

    if (LED2_brightness_turning_on >= PWM_Steps_LED2) {
      LED2_brightness_turning_on = PWM_Steps_LED2;
      turning_LED2_on = LOW;
      Serial.println("turning_LED2_on = LOW");
      Serial.println (LED2_brightness_turning_on);
    }
    analogWrite16(LED_PIN2, LED2_brightness_turning_on);
  }

  if (turning_LED2_off == HIGH)  {

    LED2_time_elapsed = millis() - LED2_millis_at_turn_off,
    LED2_brightness_turning_off = calculate_LED_brightness_linear(LED2_time_elapsed, LED2_brightness_turning_on, PWM_Steps_LED2, LED2_turn_off_time , LOW);

    if (LED2_brightness_turning_off <= 0) {
      LED2_brightness_turning_off = 0;
      turning_LED2_off = LOW;
      Serial.println("turning_LED2_off = LOW");
      Serial.println (LED2_brightness_turning_off);
    }
    analogWrite16(LED_PIN2, LED2_brightness_turning_off);
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
  if (increasing)
    brightness = start_brightness + maximum_brightness * t / total_time;

  else
    brightness = start_brightness - maximum_brightness * t / total_time;

  return brightness;
}

