#include "HX711.h"
#include <ArduinoOSC.h>
#include <ESP8266WiFi.h>
#include "./WifiSecrets.h"

const int PIN_DT = 5; // Data out (DT)
const int PIN_SCK = 4; // Clock (SCK)
// Other pins on HX711 Board: GND to ground, VCC to +3V out from the ESP8266

const int PIN_TARE = 2; // Tare - zero the scale

// Pins for the 3-Bit DIP switch to set the sensor ID. The ID is big-endian N2->N0 ( N2*4 + N1*2 + N0 )
const int PIN_N2 = 14;
const int PIN_N1 = 12;
const int PIN_N0 = 13;
const int DEFAULT_SENSOR_ID = 0;

// We scale the load to a number between 0 and 1. Rather than scaling linearly, we use a root function,
// e.g. square root, cube root, fourth root. This gives us the desirable property that small changes in
// force close to zero (e.g. plucking) are noticeable, whereas large changes in force (e.g. between a
// strong pull and full body weight) are not too dramatic.
const int MAX_EXPECTED_LOAD = 200; // Maximum force (in lbs) that I expect to exert on the load cell.
                                   // Anything at or above this will be sent as a 1.
const int CURVE_DEGREE = 4; // The degree of the scaling curve - i.e. 2 for square root.
                            // Higher degrees give a "steeper" curve in which changes close to zero are more
                            // dramatic, but changes farther from zero are less noticeable.

const float CALIBRATION_FACTOR = 31570.0; //This value is obtained using the SparkFun_HX711_Calibration sketch

const String OSC_DEST_IP = "192.168.43.188";
const int OSC_DEST_PORT = 5000;
const String OSC_DEST_ADDR = "/pull"; // Address for the scaled reading. (0-1)
const String OSC_DEST_ADDR_RAW = "/pull/raw"; // Address for the raw reading (lbs)

HX711 scale;
OscWiFi osc;
int sensorID = DEFAULT_SENSOR_ID;

void initWifiAndOSC() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to network");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  osc.begin(OSC_DEST_PORT); // Actually the port number here doesn't matter since we never receive OSC, only send it
}

void initScale() {
  scale.begin(PIN_DT, PIN_SCK);
  scale.set_scale(CALIBRATION_FACTOR); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0
}

void readSensorID() {
  int n0 = digitalRead(PIN_N0) == HIGH ? 1 : 0;
  int n1 = digitalRead(PIN_N1) == HIGH ? 1 : 0;
  int n2 = digitalRead(PIN_N2) == HIGH ? 1 : 0;
  sensorID = 4*n2 + 2*n1 + n0;
  Serial.print("Sensor ID: ");
  Serial.print(sensorID);
  Serial.println();
}

void setup() {
  pinMode(PIN_TARE, INPUT);
  pinMode(PIN_N2, INPUT);
  pinMode(PIN_N1, INPUT);
  pinMode(PIN_N0, INPUT);
  
  Serial.begin(9600);

  initWifiAndOSC();
  readSensorID();
  initScale();
}

void loop() {
  if (digitalRead(PIN_TARE) == LOW) {
    Serial.println("Zeroing scale.");
    scale.tare();
  }
  
  Serial.print("Reading: ");
  float reading_lb = scale.get_units();
  Serial.print(reading_lb, 2);
  Serial.print(" lbs"); //You can change this to kg but you'll need to refactor the calibration_factor
  Serial.println();

  float reading_trimmed = constrain(reading_lb, 0, MAX_EXPECTED_LOAD) / MAX_EXPECTED_LOAD;
  float output = pow(reading_trimmed, 1.0 / CURVE_DEGREE);
  String dest_addr = OSC_DEST_ADDR + "/" + String(sensorID); // e.g. /pull/0
  osc.send(OSC_DEST_IP, OSC_DEST_PORT, dest_addr, output);
  String dest_addr_raw = OSC_DEST_ADDR_RAW + "/" + String(sensorID); // e.g. /pull/raw/0
  osc.send(OSC_DEST_IP, OSC_DEST_PORT, dest_addr_raw, reading_lb);
}
