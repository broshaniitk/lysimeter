#include <SPI.h>
#include <SD.h>
#include <HX711.h>
#include <libmaple/iwdg.h>
#include <RTClock.h>
RTClock rtclock(RTCSEL_LSE); // initialise Low speed External clock for RTCcloc

const char* months[] = { "Dummy", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
const char* delim = " :";
char s[128]; // for sprintf
const char* weekdays[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
const char* menu[] = { "Date", "Time", "Update Rate", "Mode"};

time_t unixTime; // a time stamp
struct tm t;
uint32 tt;
tm_t mtt;



File logfile;  // the logging file

// scale for different load cells
HX711 scale1;
HX711 scale2;
HX711 scale3;
HX711 scale4;
HX711 scale5;

#define LOG_INTERVAL  10000 // mills between entries (reduce to take more/faster data)
#define ECHO_TO_SERIAL   1 // echo data to serial port (1 used during debugging)
#define WRITE_TO_SD    1//  Write data to SD card (0 during debugging)
#define WIFI_COM 1 // Communicate with ESP8266 for WIFI transmission
#define BATTERY_STATUS 1 // Read the status of battery voltage (change conversion factor)
#define RAINGAUGE_REED 1 // Read the status of battery voltage (change conversion factor)
#define BATTERY_SATUS 1

// Load cell input arguments
const byte nLoadCell = 5;  // Number of load cells
const byte nObsForAvg = 10; // Number of observations for taking average reading

// SD
const byte chipSelect = PA4;  // for the data logging shield, digital pin 10 is used for the SD cs line

const int led = PB15;   // led pin

// BATTERY
int Battery_volt = 0;
const int Battery_pin = PB1;
// Load Cell
const byte CLK = PA8;  // clock for all load cells
const byte LC1 = PA0;
const byte LC2 = PA1;
const byte LC3 = PA2;
const byte LC4 = PA3;
const byte LC5 = PB0;

const byte RAIN_PIN1 = PC13;
const byte RAIN_PIN2 = PB5;
const byte RAIN_PIN3 = PB4;
const byte RAIN_PIN4 = PA15;
const byte RAIN_PIN5 = PB14;

// Initialzing varaibles to store results
String str_time = "";
String str_data = "";
unsigned long prev_millis = 0;
byte count = 0;

// Conversion of tip to rainfall in mm
const float rain_per_tip = 0.2794;  //Each dump is 0.011" or 0.2794 mm of water
volatile float rain1 = 0;           // [rain in mm]
volatile long tipCount1 = 0;        // tip count
// volatiles are subject to modification by IRQs
volatile unsigned long rainlast1;
volatile unsigned long Last_tip1 = 0;
volatile unsigned long Current_tip1 = 0;

volatile float rain2 = 0;     // [rain in mm]
volatile long tipCount2 = 0;  // tip count
// volatiles are subject to modification by IRQs
volatile unsigned long rainlast2;
volatile unsigned long Last_tip2 = 0;
volatile unsigned long Current_tip2 = 0;

volatile float rain3 = 0;     // [rain in mm]
volatile long tipCount3 = 0;  // tip count
// volatiles are subject to modification by IRQs
volatile unsigned long rainlast3;
volatile unsigned long Last_tip3 = 0;
volatile unsigned long Current_tip3 = 0;

volatile float rain4 = 0;     // [rain in mm]
volatile long tipCount4 = 0;  // tip count
// volatiles are subject to modification by IRQs
volatile unsigned long rainlast4;
volatile unsigned long Last_tip4 = 0;
volatile unsigned long Current_tip4 = 0;

volatile float rain5 = 0;     // [rain in mm]
volatile long tipCount5 = 0;  // tip count
// volatiles are subject to modification by IRQs
volatile unsigned long rainlast5;
volatile unsigned long Last_tip5 = 0;
volatile unsigned long Current_tip5 = 0;


String round_around(int format1)
{
  if (format1 >= 10)
  {
    return String(format1);
  }
  else
  {
    String format2 = "0" + String(format1);
    return format2;
  }
}

String date_time() {
  tt = rtclock.getTime(); //gets the instatnt time
  rtclock.breakTime(rtclock.now(), mtt);
  t.tm_year = mtt.year + 1970;   // user input
  t.tm_mon = mtt.month;           // Month, 0 - jan
  t.tm_mday = mtt.day;          // Day of the month
  t.tm_hour = mtt.hour;
  t.tm_min = mtt.minute;
  t.tm_sec = mtt.second;
  t.tm_isdst = 1;        // Is DST on? 1 = yes, 0 = no, -1 = unknown
  String Time_datef = String(round_around(t.tm_hour)) + ":" + String(round_around(t.tm_min)) + ":" + String(round_around(t.tm_sec)) + ", " + String(round_around(t.tm_mday)) + "/" + String(round_around(t.tm_mon)) + "/" + String(t.tm_year);
  return Time_datef;
}

void rainIRQ1() {
  Current_tip1++;  // Count rain gauge bucket tips as they occur
}

void rainIRQ2() {
  Current_tip2++;  // Count rain gauge bucket tips as they occur
}

void rainIRQ3() {
  Current_tip3++;  // Count rain gauge bucket tips as they occur
}

void rainIRQ4() {
  Current_tip4++;  // Count rain gauge bucket tips as they occur
}

void rainIRQ5() {
  Current_tip5++;  // Count rain gauge bucket tips as they occur
}

void Rain_tip1() {
  if (Last_tip1 != Current_tip1) {
    if (millis() - rainlast1 > 10)  // ignore switch-bounce glitches less than 10mS after initial edge
    {
      rain1 += rain_per_tip;
      tipCount1 += 1;        // Counting each tip
      rainlast1 = millis();  // set up for next event
      Last_tip1 = Current_tip1;
    }
  }
}

void Rain_tip2() {
  if (Last_tip2 != Current_tip2) {
    if (millis() - rainlast2 > 10)  // ignore switch-bounce glitches less than 10mS after initial edge
    {
      rain2 += rain_per_tip;
      tipCount2 += 1;        // Counting each tip
      rainlast2 = millis();  // set up for next event
      Last_tip2 = Current_tip2;
    }
  }
}

void Rain_tip3() {
  if (Last_tip3 != Current_tip3) {
    if (millis() - rainlast3 > 10)  // ignore switch-bounce glitches less than 10mS after initial edge
    {
      rain3 += rain_per_tip;
      tipCount3 += 1;        // Counting each tip
      rainlast3 = millis();  // set up for next event
      Last_tip3 = Current_tip3;
    }
  }
}

void Rain_tip4() {
  if (Last_tip4 != Current_tip4) {
    if (millis() - rainlast4 > 10)  // ignore switch-bounce glitches less than 10mS after initial edge
    {
      rain4 += rain_per_tip;
      tipCount4 += 1;        // Counting each tip
      rainlast4 = millis();  // set up for next event
      Last_tip4 = Current_tip4;
    }
  }
}

void Rain_tip5() {
  if (Last_tip5 != Current_tip5) {
    if (millis() - rainlast5 > 10)  // ignore switch-bounce glitches less than 10mS after initial edge
    {
      rain5 += rain_per_tip;
      tipCount5 += 1;        // Counting each tip
      rainlast5 = millis();  // set up for next event
      Last_tip5 = Current_tip5;
    }
  }
}

void Serial_print() {
  Serial3.print("Current_tip1 ");
  Serial3.println(Current_tip1);
  Serial3.print("Current_tip2 ");
  Serial3.println(Current_tip2);
  Serial3.print("Current_tip3 ");
  Serial3.println(Current_tip3);
  Serial3.print("Current_tip4 ");
  Serial3.println(Current_tip4);
  Serial3.print("Current_tip5 ");
  Serial3.println(Current_tip5);

  Serial3.print("Rain and Tip Count:  1     ");
  Serial3.print(rain1);
  Serial3.print("      ");
  Serial3.println(tipCount1);
  
  Serial3.print("Rain and Tip Count:  2     ");
  Serial3.print(rain2);
  Serial3.print("      ");
  Serial3.println(tipCount2);

  Serial3.print("Rain and Tip Count:  3     ");
  Serial3.print(rain3);
  Serial3.print("      ");
  Serial3.println(tipCount3);

  Serial3.print("Rain and Tip Count:  4     ");
  Serial3.print(rain4);
  Serial3.print("      ");
  Serial3.println(tipCount4);

  Serial3.print("Rain and Tip Count:  5     ");
  Serial3.print(rain5);
  Serial3.print("      ");
  Serial3.println(tipCount5);
}

void setup(void)
{
  Serial3.begin(115200);
  Serial1.begin(115200);
  iwdg_init(IWDG_PRE_256, 4050); // init wd timer
  iwdg_feed();
  Serial3.println("Starting...");
 // setup_date ();
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  delay(1000);
  digitalWrite(led, LOW);
  delay(500);
  pinMode(RAIN_PIN1, INPUT_PULLUP);  // input from wind meters rain gauge sensor
  pinMode(RAIN_PIN2, INPUT_PULLUP);  // input from wind meters rain gauge sensor
  pinMode(RAIN_PIN3, INPUT_PULLUP);  // input from wind meters rain gauge sensor
  pinMode(RAIN_PIN4, INPUT_PULLUP);  // input from wind meters rain gauge sensor
  pinMode(RAIN_PIN5, INPUT_PULLUP);  // input from wind meters rain gauge sensor
  attachInterrupt(RAIN_PIN1, rainIRQ1, FALLING);  // attach external interrupt pins to IRQ functions
  attachInterrupt(RAIN_PIN2, rainIRQ2, FALLING);  // attach external interrupt pins to IRQ functions
  attachInterrupt(RAIN_PIN3, rainIRQ3, FALLING);  // attach external interrupt pins to IRQ functions
  attachInterrupt(RAIN_PIN4, rainIRQ4, FALLING);  // attach external interrupt pins to IRQ functions
  attachInterrupt(RAIN_PIN5, rainIRQ5, FALLING);  // attach external interrupt pins to IRQ functions
  initialize_sd_card(); // initialize the SD card & logger file
}

void loop(void)
{
  iwdg_feed();
  digitalWrite(led, LOW);
  Rain_tip1();
  Rain_tip2();
  Rain_tip3();
  Rain_tip4();
  Rain_tip5();
  //  Serial_print();
  if (millis() - prev_millis >= LOG_INTERVAL)
  {
    prev_millis = millis();
    // Data String for load cell
    str_data = "";
    str_data = String (millis() / 1000);
    String date_times = date_time();
    str_data += " ," + date_times;
    
    for (int i = 1; i <= nLoadCell; i++) {
      Rain_tip1();
      Rain_tip2();
      Rain_tip3();
      Rain_tip4();
      Rain_tip5();
      if (i == 1) {
        scale1.begin(LC1, CLK);
        long reading1 = scale1.read_average(nObsForAvg);
        str_data = str_data + ", " + reading1;
      }
      if (i == 2) {
        scale2.begin(LC2, CLK);
        long reading2 = scale2.read_average(nObsForAvg);
        str_data = str_data + ", " + reading2;
      }
      if (i == 3) {
        scale3.begin(LC3, CLK);
        long reading3 = scale3.read_average(nObsForAvg);
        str_data = str_data + ", " + reading3;
      }
      if (i == 4) {
        scale4.begin(LC4, CLK);
        long reading4 = scale4.read_average(nObsForAvg);
        str_data = str_data + ", " + reading4;
      }
      if (i == 5) {
        scale5.begin(LC5, CLK);
        long reading5 = scale5.read_average(nObsForAvg);
        str_data = str_data + ", " + reading5;
      }
    }

    str_data = str_data + " , " + tipCount1 + ":" + rain1 + " , " + tipCount2 + ":" + rain2 + " , " + tipCount3 + ":" + rain3 + " , " + tipCount4 + ":" + rain4 + " , " + tipCount5 + ":" + rain5;
    // battery voltage
    Battery_volt = analogRead(Battery_pin);
    float bat_volt= (Battery_volt*4.26)/4095.0;
    str_data = str_data + ", " + bat_volt;
    // //////////////////////////

    Serial3.print(str_data);
    Serial3.println();

    // WIFI COMMUNICATION
#if WIFI_COM
    Serial1.print(str_data);
    Serial1.println();
#endif

    // Write to SD card
#if WRITE_TO_SD
    if (logfile) {
      logfile.print(str_time);
      logfile.print(str_data);
      logfile.println();
      logfile.flush();
      count = 0;
    } else {
      Serial3.println(F("error in writing to sd card"));
      // make led high
      digitalWrite(led, HIGH);
      delay(500);
      count++;
      digitalWrite(led, LOW);
    }
    if (count >= 10)
    {
      Serial3.println(F("Restarting"));
      digitalWrite(led, HIGH);
      delay(80000);
    }
#endif
  }
}

//==============================================================================
void initialize_sd_card() {
  pinMode(chipSelect, OUTPUT);    // Chip select to output

  if (!SD.begin(chipSelect)) {
    Serial3.println(F("Card failed, or not present"));
    digitalWrite(led, HIGH);
    //   while (1);
  }
  else {
    //#if ECHO_TO_SERIAL
    Serial3.println(F("card initialized."));
    //#endif
  }

  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i / 10 + '0';
    filename[7] = i % 10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE);
      break;  // leave the loop!
    }
  }

  if (! logfile) {
    Serial3.println(F("couldnt create file"));
    digitalWrite(led, HIGH);
    //    while (1);
  }
  else {
    String temp = "";
    temp = temp + "Second, " + "datetime, ";
    for (byte i = 1; i < nLoadCell; i++) {
      temp = temp + "reading" + i + ", ";
    }
    temp = temp + "reading" + nLoadCell + ", ";

    for (byte i = 1; i <= 5; i++) {
      temp = temp + "TP" + i + ", ";
    }
    temp = temp + "Battery Voltage" ;
    Serial3.println(temp);// by giving t/t/ you can increase the spacing
  }
}
