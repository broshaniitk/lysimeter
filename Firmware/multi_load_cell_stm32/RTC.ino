
uint8_t str2month(const char* d)
{
  uint8_t i = 13;
  while ((--i) && strcmp(months[i], d) != 0);
  return i;
}


void ParseBuildTimestamp(tm_t& mt)
{
  sprintf(s, "Timestamp: %s, %s\n", __DATE__, __TIME__);   //  
  char* token = strtok(s, delim); // get first token
  while (token != NULL) {
    uint8_t m = str2month((const char*)token);
    if (m > 0) {
      mt.month = m;
      //Serial2.print(" month: "); Serial2.println(mt.month);
      token = strtok(NULL, delim); // get next token
      mt.day = atoi(token);
      //Serial2.print(" day: "); Serial2.println(mt.day);
      token = strtok(NULL, delim); // get next token
      mt.year = atoi(token) - 1970;
      //Serial2.print(" year: "); Serial2.println(mt.year);
      token = strtok(NULL, delim); // get next token
      mt.hour = atoi(token);
      //Serial2.print(" hour: "); Serial2.println(mt.hour);
      token = strtok(NULL, delim); // get next token
      mt.minute = atoi(token);
      //Serial2.print(" minute: "); Serial2.println(mt.minute);
      token = strtok(NULL, delim); // get next token
      mt.second = atoi(token);
      //Serial2.print(" second: "); Serial2.println(mt.second);
    }
    token = strtok(NULL, delim);
  }
}


void setup_date ()
{
  ParseBuildTimestamp(mtt);  // get the Unix epoch Time counted from 00:00:00 1 Jan 1970
  tt = rtclock.makeTime(mtt) + 25; // additional seconds to compensate build and upload delay
  rtclock.setTime(tt);
}
