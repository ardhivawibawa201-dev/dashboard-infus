#include "HX711.h"

#define DT 16
#define SCK 17

HX711 scale;

// calibration factor final
float calibration_factor = -1050.0;

void setup() {

  Serial.begin(115200);

  scale.begin(DT, SCK);

  delay(2000);

  // reset nol
  scale.tare();

  // set kalibrasi
  scale.set_scale(calibration_factor);

  Serial.println("LOADCELL READY");

}

void loop() {

  // averaging lebih stabil
  float berat = scale.get_units(30);

  // hilangkan noise kecil
  if(abs(berat) < 1){

    berat = 0;

  }

  Serial.print("Berat : ");
  Serial.print(berat);
  Serial.println(" gram");

  delay(1000);

}