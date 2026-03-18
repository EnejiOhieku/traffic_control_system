/*
  Firmware thats handles connection to the tscmonitor server at tscmonitor.pythonanywhere.com
  for push requests eg: http://tcsmonitor.pythonanywhere.com/push?data={states:[1,2,3],densities:[3,3,1],count:[1,2,5]}
    states: for each road 0-red, 1-yellow, 2-green
    density: for each road values from 0 to empty to 3-heavily congested
    count: count for each road in the last hour
*/

#define STOP 0
#define READY 1
#define GO 2

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>


using namespace std;




const char *ssid     = "TCS";
const char *password = "12345678";

const long utcOffsetInSeconds = 3600; // UTC +1.00
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);


struct IrSensor {
  int pin;
  int irState;
  int lastIrState = HIGH;
  unsigned long lastDebounceTime = 0;
  static const unsigned long debounceDelay = 50;

  IrSensor(int p): pin(p) {}

  void init() {
    pinMode(pin, INPUT);
  }

  bool getState() {
    int reading = digitalRead(pin);
    bool return_val = LOW;

    if (reading != lastIrState) {
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != irState) {
        irState = reading;

        if (irState == HIGH) return_val = HIGH;
      }
    }

    lastIrState = reading;
    return return_val;
  }

};


struct Road {
  IrSensor ir1, ir2;

  int16_t count = -2, density = 0, state = 0;
  uint8_t lastHour;
  int densityState;
  int lastDensityState = 0;
  unsigned long lastDebounceTime = 0;
  static const unsigned long debounceDelay = 5000;



  Road(uint8_t in1, uint8_t in2) : ir1(in1), ir2(in2) {}

  void init() {
    ir1.init();
    ir2.init();
    lastHour = timeClient.getHours();

  }

  void hourlyReset() {
    uint newHour = timeClient.getHours();
    if (lastHour != newHour) {
      count = 0;
      lastHour = newHour;
    }
  }

  void updateCount() {
    if (ir1.getState()) count++;
    if (ir2.getState()) count++;
  }

  void updateDensity() {
    int density_ = !digitalRead(ir1.pin) + !digitalRead(ir2.pin);

    if (density_ != lastDensityState) {
      lastDebounceTime = millis();
    }

    density = density_;

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (density_ != densityState) {
        densityState = density_;
      }

      if (densityState == 2) {
        density = 3;
      }
    }

    lastDensityState = density_;
  }

  void update() {
    updateCount();
    updateDensity();
  }
};


Road road1(13, 0), road2(14, 12), road3(5, 4);

vector<Road *> roads = {&road1, &road2, &road3};
unsigned long sendDataCounter = 0, pushDataCounter = 0;

void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  Serial.print("Searching for Wifi Network. SSID: TCS, KEY: 12345678.");

  while ( WiFi.status() != WL_CONNECTED ) {
    delay (500);
    Serial.print ( "." );
  }
  Serial.println("\nConnected.");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  timeClient.begin();
  timeClient.update();

  for (Road *road : roads) road->init();

}


void loop() {
  if (Serial.available()) {
    JsonDocument data;
    String data_str = Serial.readStringUntil('\n');

    deserializeJson(data, data_str);

    for (int i = 0; i < 3; i++) {
      Road* road = roads[i];
      road->state = data["states"][i];
    }
  }

  for (Road *road : roads) road->update();
  
  WiFiClient client;
  HTTPClient http;
  
  if (millis() - sendDataCounter > 1000) {
    JsonDocument dense_road; // {"densities": [0, 1, 1]} bools
    String dense_road_str;
    JsonArray dense_road_arr = dense_road["densities"].to<JsonArray>();

    for (Road* road : roads) dense_road_arr.add(uint(road->density == 3));

    serializeJson(dense_road, dense_road_str);
    Serial.println("~"+dense_road_str);
    sendDataCounter = millis();
  }

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);

    if (millis() - pushDataCounter > 500) {
      JsonDocument data; // data={states:[1,2,3],densities:[3,3,1],count:[1,2,5]}
      String data_str;

      JsonArray states = data["states"].to<JsonArray>();
      JsonArray densities = data["densities"].to<JsonArray>();
      JsonArray count = data["count"].to<JsonArray>();
    
      for (Road *road : roads) {
        states.add(road->state);
        densities.add(road->density);
        count.add(road->count);
      }

      serializeJson(data, data_str);

      Serial.println(data_str);
      Serial.println("[HTTP] begin..");
      if (http.begin(client, "http://tcsmonitor.pythonanywhere.com/push?data=" + data_str)) {
        int httpCode = http.GET();
        if (httpCode > 0) Serial.println(http.getString());
        else Serial.println("failed");
        http.end();
      }else Serial.println("Couldn't Connect");

      pushDataCounter = millis();
    }
  } else digitalWrite(LED_BUILTIN, HIGH);

}
