#include "HX711.h"
#include <ArduinoOSC.h>
#include <ESP8266WiFi.h>
#include "./WifiSecrets.h"

const int PIN_DT = 12; // Data out (DT)
const int PIN_SCK = 14; // Clock (SCK)
const int PIN_TARE = 2; // Tare - zero the scale
// Other pins on HX711 Board: GND to ground, VCC to +5V

const float CALIBRATION_FACTOR = 31570.0; //This value is obtained using the SparkFun_HX711_Calibration sketch

const String OSC_DEST_IP = "192.168.43.23";
const int OSC_DEST_PORT = 5000;
const String OSC_DEST_ADDR = "/pull";

const int SENSOR_ID = 0; // TODO This should be set by a combination of three switches in order to uniquely identify each sensor

HX711 scale;
OscWiFi osc;

void setup() {
  pinMode(PIN_TARE, INPUT);
  
  Serial.begin(9600);
  Serial.println("HX711 scale demo");
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

  scale.begin(PIN_DT, PIN_SCK);
  scale.set_scale(CALIBRATION_FACTOR); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0
}

void loop() {
  Serial.print("Reading: ");
  float reading = scale.get_units();
  Serial.print(reading, 2);
  Serial.print(" lbs"); //You can change this to kg but you'll need to refactor the calibration_factor
  Serial.println();

  String dest_addr = OSC_DEST_ADDR + "/" + String(SENSOR_ID); // e.g. /pull/0
  osc.send(OSC_DEST_IP, OSC_DEST_PORT, dest_addr, reading);
  
  if (digitalRead(PIN_TARE) == LOW) {
    Serial.println("Zeroing scale.");
    scale.tare();
  }
}
