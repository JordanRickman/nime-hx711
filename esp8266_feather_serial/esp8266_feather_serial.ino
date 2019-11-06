#include "HX711.h"

const int PIN_DT = 12; // Data out (DT)
const int PIN_SCK = 14; // Clock (SCK)
// Other pins on HX711 Board: GND to ground, VCC to +5V

const float CALIBRATION_FACTOR = 31570.0; //This value is obtained using the SparkFun_HX711_Calibration sketch

HX711 scale;

void setup() {
  Serial.begin(9600);
  Serial.println("HX711 scale demo");

  scale.begin(PIN_DT, PIN_SCK);
  scale.set_scale(CALIBRATION_FACTOR); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0
  
  Serial.println("Readings:");
}

void loop() {
  Serial.print("Reading: ");
  Serial.print(scale.get_units(), 1); //scale.get_units() returns a float
  Serial.print(" lbs"); //You can change this to kg but you'll need to refactor the calibration_factor
  Serial.println();
  // TODO Add a reset (tare/zero) button
}
