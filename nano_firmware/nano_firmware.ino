#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#define STOP 0
#define READY 1
#define GO 2
#define GO_TIME 20000

//                   R       Y        G     
unsigned long def_state_timings[3][5] = {
  {20000, 5000, 45000, 5000, 0},
  {20000, 5000, 20000, 5000, 25000},
  {45000, 5000, 20000, 5000, 0},
};
unsigned long state_timings[3][5] = {
  {20000, 5000, 45000, 5000, 0},
  {20000, 5000, 20000, 5000, 25000},
  {45000, 5000, 20000, 5000, 0},
};

unsigned long last_timings[3], sendDataTimer = 0;
uint8_t densities[3] = {0, 0, 0};
uint8_t roads_go_index[3][3] = {
  {0, 0, 0},
  {2, 2, 0},
  {2, 4, 2}
};


LiquidCrystal_I2C traffic_display1(0x26, 16, 2);

LiquidCrystal_I2C traffic_display2(0x25, 16, 2);

LiquidCrystal_I2C traffic_display3(0x27, 16, 2);


LiquidCrystal_I2C *traffic_displays[] = {&traffic_display1, &traffic_display2, &traffic_display3};
//                         
int traffic_states[3] = {0, 0, 0}; // all ready
int traffic_sequences[3][5] = {
  {GO, READY, STOP, READY, GO},
  {STOP, READY, GO, READY, STOP},
  {STOP, READY, GO, READY, STOP}
};

char *texts[] = {"STOP", "READY", "GO"};

int leds[3][3] = {
  {10, 11, 12},
  {4, 5, 6}, 
  {7, 8, 9}, 
// R  Y  G
};


SoftwareSerial traffic(A0, A1); // RX, TX

void setup()
{
  Serial.begin(9600);
  traffic.begin(9600);

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      pinMode(leds[i][j], OUTPUT);
      digitalWrite(leds[i][j], HIGH);
    }

    traffic_displays[i]->init();
    traffic_displays[i]->backlight();
    
  }

    last_timings[0] = last_timings[1] = last_timings[2] = millis();
}

String space(unsigned int num) {
  String string;
  for (int i = 0; i < num; i++) string += ' ';
  return string;
}

void addTime(int road, unsigned long time) {
  uint8_t *road_go_index = roads_go_index[road];
  for (int i = 0; i < 3; i++) {
    int j = road_go_index[i];
    state_timings[i][j] = def_state_timings[i][j] + time;
  }
}

void loop()
{
  if (traffic.available()) {
    char first_char = traffic.read();
    if (first_char == '~') {
      JsonDocument data;
      String data_str = traffic.readStringUntil('\n');
      deserializeJson(data, data_str);
      for (int i = 0; i < 3; i++) {
        densities[i] = data["densities"][i];
      }
    }
    else {
      // Serial.read;
    }
  }

  if (millis() - sendDataTimer > 250) {
    JsonDocument road_states; // {"densities": [0, 1, 1]} bools
    String road_states_str;
    JsonArray road_states_arr = road_states["states"].to<JsonArray>();

    for (int i = 0; i < 3; i++) {
      int state = traffic_sequences[i][traffic_states[i]];
      road_states_arr.add(state);
    }

    serializeJson(road_states, road_states_str);
    traffic.println(road_states_str);
    sendDataTimer = millis();
  }

  int sum = densities[0] + densities[1] + densities[2];
  int extraTime = 0;
  if (sum == 1) {
    extraTime = GO_TIME;
  } else if (sum == 2) {
    extraTime = GO_TIME / 2;
  }

  for (int i = 0; i < 3; i++) {
    if (densities[i]) {
      addTime(i, extraTime);
    }
  }

  for (int i = 0; i < 3; ++i) { 
    int state = traffic_sequences[i][traffic_states[i]];
    
    for (int j = 0; j < 3; ++j) digitalWrite(leds[i][j], HIGH);
    digitalWrite(leds[i][state], LOW);
  }


  for (int i = 0; i < 3; ++i) {
    int traffic_sequence_len = sizeof(traffic_sequences[i]) / sizeof(traffic_sequences[i][0]);
    int time_left;

    int state = traffic_sequences[i][traffic_states[i]];
    unsigned long duration = millis() - last_timings[i];

    if (traffic_states[i] == 4) {
      time_left = (last_timings[i] + state_timings[i][traffic_states[i]] + state_timings[i][0] - millis()) / 1000;
    } else time_left = (last_timings[i] + state_timings[i][traffic_states[i]] - millis()) / 1000;

    if (time_left < 0) time_left = 0;

    
    int spaces_before = (16 - strlen(texts[state])) / 2;
    String line1 = String(i+1) + space(spaces_before) + texts[state] + "          ";
    String line2 = "Time Left: " + String(time_left) + "s" + "          ";
    
    traffic_displays[i]->setCursor(0, 0);
    traffic_displays[i]->print(line1);
    traffic_displays[i]->setCursor(0, 1);
    traffic_displays[i]->print(line2);
  }

  
  for (int i = 0; i < 3; ++i) {
    int traffic_sequence_len = sizeof(traffic_sequences[i]) / sizeof(traffic_sequences[i][0]);
    int state = traffic_sequences[i][traffic_states[i]];

    long time_left = (last_timings[i] + state_timings[i][traffic_states[i]] - millis());

    if (time_left < 0) {
      traffic_states[i] = (traffic_states[i] + 1) % 5;
      last_timings[i] = millis() + time_left;
    }
  }
}
