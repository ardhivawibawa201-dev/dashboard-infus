#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HX711.h>
#include <ESP32Servo.h>

// ================= OLED =================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// ================= HX711 =================

#define DT 16
#define SCK 17

HX711 scale;

float calibration_factor = 1050.0;

// ================= SERVO =================

#define SERVO_PIN 25

Servo servoInfus;

int sudutServo = 130;

// ================= SENSOR =================

#define SENSOR_PIN 27

long jumlahTetes = 0;
int lastState;

// ================= BUZZER =================

#define BUZZER_PIN 26

// ================= TPM =================

int TPM = 0;
int targetTPM = 20;

unsigned long lastHitung = 0;

void setup()
{
  Serial.begin(115200);

  Serial.println("BOOTING...");

  // OLED
  Wire.begin(21,22);

  if(display.begin(
      SSD1306_SWITCHCAPVCC,
      0x3C))
  {
    Serial.println("OLED OK");
  }
  else
  {
    Serial.println("OLED ERROR");
  }

  // HX711
  scale.begin(DT,SCK);

  delay(500);

  if(scale.is_ready())
  {
    scale.tare();
    scale.set_scale(calibration_factor);

    Serial.println("HX711 OK");
  }
  else
  {
    Serial.println("HX711 NOT READY");
  }

  // Servo
  servoInfus.attach(SERVO_PIN);

  servoInfus.write(sudutServo);

  Serial.println("SERVO OK");

  // Sensor
  pinMode(SENSOR_PIN, INPUT);

  lastState =
  digitalRead(SENSOR_PIN);

  Serial.println("SENSOR OK");

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("SMART INFUS READY");
}

void loop()
{
  // ======================
  // LOADCELL
  // ======================

  float berat = 0;

  if(scale.is_ready())
  {
    berat =
    scale.get_units(10);

    if(abs(berat) < 1)
    {
      berat = 0;
    }
  }

  // ======================
  // SENSOR TETES
  // ======================

  int state =
  digitalRead(
    SENSOR_PIN
  );

  if(
    lastState == HIGH &&
    state == LOW
  )
  {
    jumlahTetes++;

    delay(80);
  }

  lastState = state;

  // ======================
  // HITUNG TPM
  // ======================

  if(
    millis() - lastHitung >= 5000
  )
  {
    TPM =
    jumlahTetes * 12;

    Serial.println("---------------------");

    Serial.print("Berat : ");
    Serial.print(berat);
    Serial.println(" g");

    Serial.print("TPM   : ");
    Serial.println(TPM);

    Serial.print("Servo : ");
    Serial.println(sudutServo);

    // ======================
    // AUTO CARI 20 TPM
    // ======================

    if(TPM > 100)
    {
      sudutServo += 8;
    }
    else if(TPM > 60)
    {
      sudutServo += 5;
    }
    else if(TPM > 30)
    {
      sudutServo += 2;
    }
    else if(TPM > 22)
    {
      sudutServo += 1;
    }
    else if(TPM < 10)
    {
      sudutServo -= 3;
    }
    else if(TPM < 18)
    {
      sudutServo -= 1;
    }

    sudutServo =
    constrain(
      sudutServo,
      90,
      170
    );

    servoInfus.write(
      sudutServo
    );

    if(
      TPM >= 18 &&
      TPM <= 22
    )
    {
      Serial.println(
        "TARGET 20 TPM"
      );
    }

    // ======================
    // BUZZER
    // ======================

    if(berat <= 20)
    {
      digitalWrite(
        BUZZER_PIN,
        HIGH
      );
    }
    else
    {
      digitalWrite(
        BUZZER_PIN,
        LOW
      );
    }

    jumlahTetes = 0;

    lastHitung = millis();
  }

  // ======================
  // OLED
  // ======================

  display.clearDisplay();

  display.setTextSize(1);

  display.setTextColor(
    SSD1306_WHITE
  );

  display.setCursor(0,0);
  display.print("Berat:");
  display.print(berat,0);
  display.println(" g");

  display.setCursor(0,18);
  display.print("TPM:");
  display.println(TPM);

  display.setCursor(0,36);
  display.print("Servo:");
  display.println(sudutServo);

  display.setCursor(0,54);

  if(
    TPM >= 18 &&
    TPM <= 22
  )
  {
    display.print(
      "TPM STABIL"
    );
  }
  else
  {
    display.print(
      "MENCARI..."
    );
  }

  display.display();

  delay(50);
}