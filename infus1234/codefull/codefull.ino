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

// ================= SENSOR =================
#define PHOTO_PIN 27
#define BUZZER_PIN 26
#define SERVO_PIN 25

// ================= SERVO =================
Servo servoInfus;

int posisiBuka  = 30;
int posisiJepit = 100;

// ================= DATA =================
volatile long jumlahTetes = 0;

unsigned long lastDropTime = 0;
unsigned long lastTPMTime = 0;

long tetesSebelumnya = 0;

float berat = 0;
int TPM = 0;

bool infusMacet = false;
bool infusHabis = false;

// ================= INTERRUPT =================
void IRAM_ATTR hitungTetes()
{
  jumlahTetes++;
  lastDropTime = millis();
}

void setup()
{
  Serial.begin(115200);

  pinMode(PHOTO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Sensor tetes
  attachInterrupt(
    digitalPinToInterrupt(PHOTO_PIN),
    hitungTetes,
    FALLING      // ganti RISING jika sensor tidak terbaca
  );

  // Loadcell
  scale.begin(DT, SCK);

  delay(2000);

  scale.tare();

  scale.set_scale(calibration_factor);

  // Servo
  servoInfus.attach(SERVO_PIN);
  servoInfus.write(posisiBuka);

  // OLED
  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        0x3C))
  {
    while (1);
  }

  display.clearDisplay();
  display.display();

  lastDropTime = millis();
  lastTPMTime = millis();

  Serial.println("SISTEM INFUS READY");
}

void loop()
{
  // ================= LOADCELL =================
  berat = scale.get_units(30);

  if (abs(berat) < 1)
  {
    berat = 0;
  }

  // ================= TPM =================
  if (millis() - lastTPMTime >= 60000)
  {
    TPM = jumlahTetes - tetesSebelumnya;

    tetesSebelumnya = jumlahTetes;

    lastTPMTime = millis();
  }

  // ================= DETEKSI MACET =================
  if (millis() - lastDropTime > 10000)
  {
    infusMacet = true;
  }
  else
  {
    infusMacet = false;
  }

  // ================= DETEKSI HAMPIR HABIS =================
  if (berat <= 50)
  {
    infusHabis = true;
  }
  else
  {
    infusHabis = false;
  }

  // ================= KONTROL SERVO =================
  if (infusMacet || infusHabis)
  {
    servoInfus.write(posisiJepit);
  }
  else
  {
    servoInfus.write(posisiBuka);
  }

  // ================= BUZZER =================
  if (infusMacet || infusHabis)
  {
    tone(BUZZER_PIN, 2000);
  }
  else
  {
    noTone(BUZZER_PIN);
  }

  // ================= OLED =================
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.print("Berat : ");
  display.print(berat, 1);
  display.println(" g");

  display.setCursor(0, 15);
  display.print("Tetes : ");
  display.println(jumlahTetes);

  display.setCursor(0, 30);
  display.print("TPM   : ");
  display.println(TPM);

  display.setCursor(0, 45);

  if (infusMacet)
  {
    display.println("STATUS: MACET");
  }
  else if (infusHabis)
  {
    display.println("STATUS: HAMPIR HABIS");
  }
  else
  {
    display.println("STATUS: NORMAL");
  }

  display.display();

  // ================= SERIAL =================
  Serial.print("Berat: ");
  Serial.print(berat);

  Serial.print(" g | TPM: ");
  Serial.print(TPM);

  Serial.print(" | Tetes: ");
  Serial.println(jumlahTetes);

  delay(200);
}