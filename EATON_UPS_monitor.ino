#include <ArduinoJson.h>
#define leds 4

enum OutputFormat { RAW, LEDS, STATE };

OutputFormat outFormat = STATE;

int onOffEdge = 350;
int outputEveryMillis = 5000;
int minBlinksForFast = 7;

unsigned long valueSum[leds] = {0, 0, 0, 0};
unsigned int valueCur[leds] = {0, 0, 0, 0};
unsigned int valueLastChange[leds] = {0, 0, 0, 0};
unsigned short changesCntr[leds] = {0, 0, 0, 0};

unsigned int loopCntr = 0;
unsigned long stopTime = millis() + outputEveryMillis;
bool raw = false;
String prevOut = "";


String on = "on";
String off = "off";
String slow = "slow";
String fast = "fast";
String a = "A";

void setup()
{
  Serial.begin(9600);
}


void loop()
{
  if (millis() >= stopTime) {
    switch (outFormat) {
      case RAW:
        doRawOutput();
        break;
      case LEDS:
        doDiscreteOutput();
        break;
      case STATE:
        doStateOutput();
        break;
    }

    dataReset();
  } else {
    for (int i = 0; i < leds; i++) {
      valueCur[i] = analogRead(i);
      valueSum[i] += valueCur[i];
      if (valueLastChange[i] > onOffEdge != valueCur[i] > onOffEdge) {
        changesCntr[i]++;
        valueLastChange[i] = valueCur[i];
      }
    }
    loopCntr++;
  }
}

void doRawOutput() {
  Serial.print("{\"A0\":");
  Serial.print(valueSum[0] / loopCntr);

  Serial.print(",\"A1\":");
  Serial.print(valueSum[1] / loopCntr);

  Serial.print(",\"A2\":");
  Serial.print(valueSum[2] / loopCntr);

  Serial.print(",\"A3\":");
  Serial.print(valueSum[3] / loopCntr);
  Serial.println("}");
}

void doDiscreteOutput() {
  StaticJsonDocument<100> doc;
  for (int i = 0; i < leds; i++) {
    if (changesCntr[i] < 2) {
      // we are on or off
      doc[a + i] = valueSum[i] / loopCntr > onOffEdge ? on : off;
    } else {
      //we are blinking
      doc[a + i] = changesCntr[i] < minBlinksForFast ? slow : fast;
    }
  }
  String out = "";
  serializeJson(doc, out);
  output(out);
}

void doStateOutput() {

  String states[leds];
  for (int i = 0; i < leds; i++) {
    if (changesCntr[i] < 2) {
      // we are on or off
      states[i] = valueSum[i] / loopCntr > onOffEdge ? on : off;
    } else {
      //we are blinking
      states[i] = changesCntr[i] < minBlinksForFast ? slow : fast;
    }
  }

  String state = "error";
  short value = 0;

  // if a single LED in on we are on mains
  short onCntr = 0;
  short id = 0;
  for (short i = 0; i < leds; i++) {
    if (states[i] == on) {
      onCntr++;
      id = i;
    }
  }
  if (onCntr == 1) {
    state = "normal";
    switch (id) {
      case 0:
        value = 9;
        break;
      case 1:
        value = 12;
        break;
      case 2:
        value = 15;
        break;
      case 3:
        value = 19;
        break;
    }
  } else if (states[0] == fast && states[1] == off && states[2] == off && states[3] == off) {
    //we are discharging <5%
    state = "discharging";
    value = 3;
  } else if (states[0] == slow && states[1] == off && states[2] == off && states[3] == off) {
    //we are discharging >5 <10%
    state = "discharging";
    value = 8;
  } else if (states[0] == on && states[1] == slow && states[2] == off && states[3] == off) {
    //we are discharging >25 <50%
    state = "discharging";
    value = 38;
  } else if (states[0] == on && states[1] == on && states[2] == slow && states[3] == off) {
    //we are discharging >50 <75%
    state = "discharging";
    value = 63;
  } else if (states[0] == on && states[1] == on && states[2] == on && states[3] == slow) {
    //we are discharging >75 <100%
    state = "discharging";
    value = 88;
  } else if (states[0] == on && states[1] == on && states[2] == on && states[3] == on) {
    //we are discharging 100%
    state = "discharging";
    value = 100;
  }


  StaticJsonDocument<100> doc;
  doc["state"] = state;
  doc["value"] = value;

  String out = "";
  serializeJson(doc, out);
  output(out);
}


void dataReset() {
  for (int i = 0; i < leds; i++) {
    valueSum[i] = 0;
    changesCntr[i] = 0;
  }
  stopTime = millis() + outputEveryMillis;
  loopCntr = 0;
}

void output(String out) {
  if (out.equals(prevOut)) {
    Serial.println(out);
  } else {
    prevOut = out;
  }
}
