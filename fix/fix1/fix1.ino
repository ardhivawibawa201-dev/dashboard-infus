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

float calibration_factor = -1063.0;

// ================= SERVO =================
#define SERVO_PIN 13

Servo servoInfus;

int sudutServo = 110;

// ================= SENSOR TETES =================
#define SENSOR_PIN 27

long jumlahTetes = 0;
int lastState;

// debounce sensor tetes
unsigned long lastDropTime = 0;

// ================= BUZZER =================
#define BUZZER_PIN 26

// ================= PARAMETER =================
const int TARGET_TPM = 20;
const int TPM_MIN = 18;
const int TPM_MAX = 22;

const float BATAS_INFUS_HABIS = 80.0;

// ================= VARIABEL =================
int TPM = 0;
unsigned long lastHitung = 0;

// =================================================

void setup()
{
  Serial.begin(115200);

  Wire.begin(21,22);

  // OLED
  if(display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("OLED OK");
  }

  // HX711
  scale.begin(DT,SCK);

  delay(1000);

  if(scale.is_ready())
  {
    scale.tare();
    scale.set_scale(calibration_factor);

    Serial.println("HX711 OK");
  }

  // Servo
  servoInfus.attach(SERVO_PIN);
  servoInfus.write(sudutServo);

  // Sensor tetes
  pinMode(SENSOR_PIN, INPUT_PULLUP);

  lastState = digitalRead(SENSOR_PIN);

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(10,20);
  display.println("SMART INFUS");

  display.display();

  delay(1500);
}

// =================================================

void loop()
{
  // ======================
  // BACA BERAT
  // ======================

  float berat = 0;
if(scale.is_ready())
{
  berat = scale.get_units(20);

  if(berat < 0)
  {
    berat = -berat;
  }

  if(abs(berat) < 1)
  {
    berat = 0;
  }
}

  // ======================
  // HITUNG TETESAN
  // ======================

  int state = digitalRead(SENSOR_PIN);

if(lastState == HIGH && state == LOW)
{
  if(millis() - lastDropTime > 150)
  {
    jumlahTetes++;

    Serial.print("DROP : ");
    Serial.println(jumlahTetes);

    lastDropTime = millis();
  }
}

lastState = state;
  // ======================
  // HITUNG TPM
  // ======================

  if(millis() - lastHitung >= 5000)
  {
    TPM = jumlahTetes * 12;

    jumlahTetes = 0;

    // ======================
    // AUTO SERVO
    // ======================

    if(TPM > TPM_MAX)
    {
      sudutServo++;
    }
    else if(TPM < TPM_MIN)
    {
      sudutServo--;
    }

    sudutServo = constrain(
      sudutServo,
      90,
      130
    );

    servoInfus.write(sudutServo);

    lastHitung = millis();

    Serial.println("================");
    Serial.print("BERAT : ");
    Serial.println(berat);

    Serial.print("TPM : ");
    Serial.println(TPM);

    Serial.print("SERVO : ");
    Serial.println(sudutServo);
  }

// ======================
// BUZZER ALARM INFUS
// ======================

if (berat > BATAS_INFUS_HABIS)
{
  digitalWrite(BUZZER_PIN, LOW);
}
else if (berat > 0)
{
  unsigned long pola = millis() % 500;

  if (
      (pola >= 0   && pola < 30) ||
      (pola >= 100 && pola < 130) ||
      (pola >= 200 && pola < 230)
     )
  {
    digitalWrite(BUZZER_PIN, HIGH);
  }
  else
  {
    digitalWrite(BUZZER_PIN, LOW);
  }
}
else
{
  digitalWrite(BUZZER_PIN, LOW);
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
  display.print(" g");

  display.setCursor(0,14);
  display.print("TPM:");
  display.print(TPM);

  display.setCursor(0,28);
  display.print("Servo:");
  display.print(sudutServo);

  display.setCursor(0,42);

  if(berat > BATAS_INFUS_HABIS)
  {
    display.print("Infus:NORMAL");
  }
  else if(berat > 0)
  {
    display.print("Infus:HAMPIR HABIS");
  }
  else
  {
    display.print("CEK LOADCELL");
  }

  display.setCursor(0,54);

  if(TPM >= TPM_MIN &&
     TPM <= TPM_MAX)
  {
    display.print("TPM STABIL");
  }
  else if(TPM < TPM_MIN)
  {
    display.print("TPM RENDAH");
  }
  else
  {
    display.print("TPM TINGGI");
  }

  display.display();

  delay(200);
}