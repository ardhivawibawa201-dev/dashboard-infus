#include <ESP32Servo.h>

#define SERVO_PIN 25
#define SENSOR_PIN 21

Servo servoInfus;

long jumlahTetes = 0;

const int targetTPM = 20;

int sudutServo = 130;

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

  // Hitung tetesan
  if(lastState == HIGH && state == LOW)
  {
    jumlahTetes++;
    delay(80);
  }

  lastState = state;

  // Evaluasi tiap 3 detik
  if(millis() - lastTPM >= 3000)
  {
    int TPM = jumlahTetes * 20;

    Serial.print("TPM = ");
    Serial.print(TPM);

    Serial.print(" | Servo = ");
    Serial.println(sudutServo);

    // Target tercapai
    if(TPM >= 18 && TPM <= 22)
    {
      Serial.println("TPM STABIL");
    }

    // Terlalu cepat
    else if(TPM > 150)
    {
      sudutServo += 5;
      Serial.println("JEPIT +5");
    }
    else if(TPM > 80)
    {
      sudutServo += 3;
      Serial.println("JEPIT +3");
    }
    else if(TPM > 30)
    {
      sudutServo += 1;
      Serial.println("JEPIT +1");
    }

    // Terlalu lambat
    else if(TPM < 10)
    {
      sudutServo -= 3;
      Serial.println("BUKA -3");
    }
    else if(TPM < 18)
    {
      sudutServo -= 1;
      Serial.println("BUKA -1");
    }

    sudutServo = constrain(sudutServo, 0, 180);

    servoInfus.write(sudutServo);

    jumlahTetes = 0;
    lastTPM = millis();
  }
}