
signed char incrementToMax(signed char counter, signed char counter_min, signed char counter_max)
{
  counter++;          // increment the counter
  if (counter > counter_max)   // but not more than the specified maximum
    counter = counter_min;
  return counter;
}

signed char incrementAlarmMode(signed char alarm_mode)
{
  return incrementToMax(alarm_mode, 1, num_alarm_modes);
}

signed char incrementClockMode(signed char clock_mode)
{
  return incrementToMax(clock_mode, 1, num_clock_modes);
}

signed char incrementDay(signed char day_counter)
{
  return incrementToMax(day_counter, 1, 7);
}

signed char incrementDate(signed char date_counter)
{
  return incrementToMax(date_counter, 1, 31);
}

signed char incrementMonth(signed char month_counter)
{
  return incrementToMax(month_counter, 1, 12);
}

signed char incrementHour(signed char hour_counter)
{
  return incrementToMax(hour_counter, 0, 23);
}

signed char incrementMinute(signed char minute_counter)
{
  return incrementToMax(minute_counter, 0, 59);
}
