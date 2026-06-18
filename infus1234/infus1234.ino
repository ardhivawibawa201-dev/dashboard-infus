#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HX711.h>

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
#define DOUT 16
#define CLK 17

HX711 scale;

// GANTI SESUAI HASIL KALIBRASI
float calibration_factor = 1050.0;

// ================= SENSOR TETES =================
#define SENSOR_PIN 27

bool sensorSebelumnya = HIGH;

long jumlahTetes = 0;

unsigned long waktuTPM = 0;

int TPM = 0;

// ================= BUZZER =================
#define BUZZER 26

bool infusHabis = false;

void setup()
{
  Serial.begin(115200);

  // OLED
  Wire.begin(21,22);

  if(!display.begin(
      SSD1306_SWITCHCAPVCC,
      0x3C))
  {
    Serial.println("OLED GAGAL");
    while(1);
  }

  // HX711
  scale.begin(DOUT, CLK);

  scale.set_scale(calibration_factor);

  scale.tare();

  // Sensor tetes
  pinMode(SENSOR_PIN, INPUT);

  // Buzzer
  pinMode(BUZZER, OUTPUT);

  waktuTPM = millis();

  Serial.println("SISTEM MONITORING INFUS READY");
}

void loop()
{
  // ================= LOADCELL =================
  float berat = scale.get_units(10);

  if(berat < 0)
  {
    berat = 0;
  }

  // ================= SENSOR TETES =================
  bool sensor = digitalRead(SENSOR_PIN);

  if(sensor == LOW && sensorSebelumnya == HIGH)
  {
    jumlahTetes++;
  }

  sensorSebelumnya = sensor;

  // ================= HITUNG TPM =================
  if(millis() - waktuTPM >= 10000)
  {
    TPM = jumlahTetes * 6;

    jumlahTetes = 0;

    waktuTPM = millis();
  }

  // ================= STATUS INFUS =================
  if(berat <= 50)
  {
    infusHabis = true;
  }
  else
  {
    infusHabis = false;
  }

  // ================= BUZZER =================
  if(infusHabis)
  {
    tone(BUZZER, 2000);
  }
  else
  {
    noTone(BUZZER);
  }

  // ================= SERIAL MONITOR =================
  Serial.print("Berat : ");
  Serial.print(berat,1);
  Serial.print(" g");

  Serial.print(" | TPM : ");
  Serial.print(TPM);

  Serial.print(" | Tetes : ");
  Serial.println(jumlahTetes);

  // ================= OLED =================
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0,0);
  display.print("Berat : ");
  display.print(berat,1);
  display.println(" g");

  display.setCursor(0,20);
  display.print("TPM   : ");
  display.println(TPM);

  display.setCursor(0,40);

  if(infusHabis)
  {
    display.println("STATUS: HABIS");
  }
  else
  {
    display.println("STATUS: NORMAL");
  }

  display.display();

  delay(100);
}