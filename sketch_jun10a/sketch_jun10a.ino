#include <ESP32Servo.h>

#define SERVO_PIN 25
#define SENSOR_PIN 21

Servo servoInfus;

volatile long jumlahTetes = 0;

int targetTPM = 20;

// SESUAIKAN DENGAN MEKANIKMU
int sudutServo = 110;

unsigned long lastTPM = 0;

int lastState;

void setup()
{
  Serial.begin(115200);

  pinMode(SENSOR_PIN, INPUT);

  servoInfus.attach(SERVO_PIN);
  servoInfus.write(sudutServo);

  lastState = digitalRead(SENSOR_PIN);

  Serial.println("AUTO TPM 20");
}

void loop()
{
  int state = digitalRead(SENSOR_PIN);

  // Hitung tetes
  if(lastState == HIGH && state == LOW)
  {
    jumlahTetes++;

    delay(150); // debounce sensor
  }

  lastState = state;

  // Evaluasi setiap 10 detik
  if(millis() - lastTPM >= 10000)
  {
    int TPM = jumlahTetes * 6;

    Serial.println();
    Serial.print("TPM = ");
    Serial.print(TPM);

    Serial.print(" | Servo = ");
    Serial.println(sudutServo);

    // Cari TPM target
    if(TPM > 21)
    {
      sudutServo++;

      Serial.println("Terlalu cepat -> Jepit");
    }
    else if(TPM < 19)
    {
      sudutServo--;

      Serial.println("Terlalu lambat -> Buka");
    }
    else
    {
      Serial.println("TPM OK");
    }

    sudutServo = constrain(
      sudutServo,
      0,
      180
    );

    servoInfus.write(sudutServo);

    jumlahTetes = 0;

    lastTPM = millis();
  }
}