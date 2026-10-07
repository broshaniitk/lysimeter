#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_task_wdt.h>  // WATCH DOG TIMER
#define WDT_TIMEOUT 120    // WATCH DOG TIMEOUT

#include <EEPROM.h>
#define EEPROM_SIZE 25

WiFiClient espClient;
PubSubClient client(espClient);

const char *ssid = "";  // Enter your WiFi name
const char *password = "";  // Enter WiFi password


const char *mqtt_broker = "";       // IP ADDRESS
const char *topic = "";         // PUBLSH TOPIC
const char *subtopic = "";  // SUBSCRIBE TOPIC
const int mqtt_port = ;                      // MQTT PORT NUMBER



////////// DATA LOGGING TIME IN S

int logging_time = 20;  // ENTER THE VALUE IN SECOND
#define LED_BUILTIN 2

String sParams[5];  // STRING ARRAY TO DECODE RECEIVED PARAMETERS
int iCount, ii;
unsigned long previous_millis;
bool fresh_data = false;

void mqtt_connect() {
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  esp_task_wdt_reset();
  if (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), "", "")) {
      Serial.println("MQTT connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(1000);
    }
  }

  esp_task_wdt_reset();
  // publish and subscribe
  //client.publish(topic, "Hi");
  client.subscribe(subtopic);
}


void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  uint32_t wifi_try = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    wifi_try++;
    if (wifi_try >= 20) {
      break;
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial2.begin(115200);
pinMode(LED_BUILTIN,OUTPUT);
digitalWrite(LED_BUILTIN, HIGH);
delay(1000);
  esp_task_wdt_init(WDT_TIMEOUT, true);  //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);                //add current thread to WDT watch
  setup_wifi();
  esp_task_wdt_reset();
  mqtt_connect();
  digitalWrite(LED_BUILTIN, LOW);
}

int StringSplit(String sInput, char cDelim, String sParams[], int iMaxParams) {
  int iParamCount = 0;
  int iPosDelim, iPosStart = 0;
  do {
    // Searching the delimiter using indexOf()
    iPosDelim = sInput.indexOf(cDelim, iPosStart);
    if (iPosDelim > (iPosStart + 1)) {
      // Adding a new parameter using substring()
      sParams[iParamCount] = sInput.substring(iPosStart, iPosDelim);
      iParamCount++;
      // Checking the number of parameters
      if (iParamCount >= iMaxParams) {
        return (iParamCount);
      }
      iPosStart = iPosDelim + 1;
    }
  } while (iPosDelim >= 0);
  if (iParamCount < iMaxParams) {
    // Adding the last parameter as the end of the line
    sParams[iParamCount] = sInput.substring(iPosStart);
    iParamCount++;
  }
  return (iParamCount);
}

void callback(char *subtopic, byte *payload, unsigned int length) {
  esp_task_wdt_reset();
  Serial.print("Message arrived in topic: ");
  Serial.println(subtopic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  char *msg = ((char *)payload);
  String message = String(msg);
  Serial.println(" decoding callback message received  ");
  SetUp_Parameters(message);  // pass the string for decoding
  Serial.println(" decoded  callback message ");
}

void SetUp_Parameters(String msgg) {
  Serial.println(" setup parameters initiated  ");
  esp_task_wdt_reset();
  String payld[3];
  if (msgg.length() > 0) {
    iCount = StringSplit(msgg, ',', payld, 3);
    String L = payld[0].substring(5, 8);
    for (ii = 0; ii < iCount; ii++) {
    }
    ii = 1;
  }
  //payld[0] = payld[0].toLowerCase();
  if (payld[0] == "timer") {
    int a = payld[1].toInt();
    if (a > 255) {
      a = 255;
    }
    if (a < 5) {
      a = 5;
    }
    EEPROM.write(0, a);
    EEPROM.commit();

    logging_time = EEPROM.read(0);
    String feedback = "Time= " + String(logging_time) + " Second ";
    client.publish(topic, (char *)feedback.c_str());
  }

  esp_task_wdt_reset();
  Serial.println(" setup parameters done  ");
}


String Serial_data() {
  if (Serial2.available() > 0) {
    String Ser_Data = "";
    while (Serial2.available()) {
      Ser_Data += (char) Serial2.read();
    }
    Serial2.println(Ser_Data);
    Ser_Data.trim();
    fresh_data = true;
    return Ser_Data;
  }
  return "";
}

void loop() {
  esp_task_wdt_reset();
  if (WiFi.status() != WL_CONNECTED) {
    uint32_t wifi_try = 0;
    Serial.println(" main loop wifi disconnected  ");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      if (wifi_try == 15) {
        break;
      }
    }
    esp_task_wdt_reset();
    mqtt_connect();
  }

  String Sens_Data = Serial_data();
  // if (millis() - previous_millis >= logging_time * 1000) {
  if (fresh_data)
  {
    fresh_data = false;
    previous_millis = millis();
    String final_data = "{ , " + String(previous_millis / 1000) + " , "  + Sens_Data + " , } \n";  // + incomingByte + "\n";
    Serial.print("final_data    ");
    Serial.println(final_data);
    int strlength = final_data.length() + 1;
    char b[strlength];
    final_data.toCharArray(b, strlength);
    char *a = b;
    if (client.publish(topic, a)) {
      Serial.println("Published Successfully");
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      Serial.println("Published failed");
    }
    final_data = "";
    Sens_Data = "";
  }
  delay(500);
   digitalWrite(LED_BUILTIN, LOW);
  client.loop();
}
