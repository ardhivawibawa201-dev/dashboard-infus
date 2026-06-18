#include "HX711.h"

#define DT 16
#define SCK 17

HX711 scale;

float calibration_factor = 1050.0;

void setup() {

  Serial.begin(115200);

  scale.begin(DT, SCK);

  delay(2000);

  if(scale.is_ready()) {

    Serial.println("HX711 READY");

    scale.tare();

    scale.set_scale(calibration_factor);

    Serial.println("LOADCELL READY");

  } else {

    Serial.println("HX711 NOT FOUND");

  }
}

void loop() {

  if(scale.is_ready()) {

    float berat = scale.get_units(10);

    Serial.print("Berat : ");
    Serial.print(berat);
    Serial.println(" gram");

  } else {

    Serial.println("HX711 LOST");

  }

  delay(1000);
}