#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HX711.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
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

int sudutServo = 95;

// ================= SENSOR TETES =================
#define SENSOR_PIN 27

long jumlahTetes = 0;
int lastState;

// debounce sensor tetes
unsigned long lastDropTime = 0;
unsigned long lastDropDetected = 0;

bool alarmMacet = false;
// ================= BUZZER =================
#define BUZZER_PIN 25

// ================= PARAMETER =================
int TARGET_TPM = 20;
const int TPM_MIN = 19;
const int TPM_MAX = 25;

const float BATAS_INFUS_HABIS = 200.0;

// ================= VARIABEL =================
int TPM = 0;

int TPM1 = 20;
int TPM2 = 20;
int TPM3 = 20;

unsigned long lastHitung = 0;

// =================================================

const char* ssid = "AKU";
const char* password = "123456788";

const char* mqtt_server =
"870c766cdeac4276a1d1addf8b853c55.s1.eu.hivemq.cloud";

const int mqtt_port = 8883;

const char* mqtt_user = "Infuspintar";
const char* mqtt_pass = "Infuspintar1234_";

WiFiClientSecure espClient;
PubSubClient client(espClient);
void reconnectMQTT()
{
  while (!client.connected())
  {
    Serial.println("MQTT Connecting...");

    if (
      client.connect(
        "ESP32_INFUS",
        mqtt_user,
        mqtt_pass
      )
    )
    {
      Serial.println("MQTT Connected");

      client.subscribe(
        "infus/target"
      );
    }
    else
    {
      Serial.print("MQTT Failed, rc=");
      Serial.println(client.state());

      delay(3000);
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length)
{
  String pesan = "";

  for(int i = 0; i < length; i++)
  {
    pesan += (char)payload[i];
  }

  if(String(topic) == "infus/target")
  {
    TARGET_TPM = constrain(
      pesan.toInt(),
      10,
      60
    );

    Serial.print("TARGET TPM BARU : ");
    Serial.println(TARGET_TPM);
  }
}
void setup()
{
  Serial.begin(115200);
WiFi.begin(
  ssid,
  password
);

while(
  WiFi.status() != WL_CONNECTED
)
{
  delay(500);
  Serial.print(".");
}

Serial.println("");
Serial.println("WiFi Connected");

espClient.setInsecure();

client.setServer(
  mqtt_server,
  mqtt_port
);
client.setCallback(callback);
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
  if(!client.connected())
  {
    reconnectMQTT();
  }

  client.loop();

  // BACA BERAT
  // ======================

  float berat = 0;

if(scale.is_ready())
{
  berat = scale.get_units(5);

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

if(lastState == LOW && state == HIGH)
{
  if(millis() - lastDropTime > 50)
  {
    jumlahTetes++;

    Serial.print("DROP ");
    Serial.print(jumlahTetes);

    Serial.print(" TIME=");
    Serial.println(millis());

    lastDropTime = millis();

    // reset alarm macet
    lastDropDetected = millis();
    alarmMacet = false;
  }
}

lastState = state;
  // ======================
  // HITUNG TPM
  // ======================

if(millis() - lastHitung >= 10000)
{
TPM1 = TPM2;
TPM2 = TPM3;
TPM3 = jumlahTetes * 6;

TPM = (TPM1 + TPM2 + TPM3) / 3;

    jumlahTetes = 0;

    // ======================
    // AUTO SERVO
    // ======================

    int errorTPM = TARGET_TPM - TPM;

    if(abs(errorTPM) > 0)
    {
      int langkah = abs(errorTPM) / 2;

      if(langkah < 1)
        langkah = 1;

      if(langkah > 5)
        langkah = 5;

      if(errorTPM > 0)
      {
        sudutServo -= langkah;
      }
      else
      {
        sudutServo += langkah;
      }
    }

sudutServo = constrain(
  sudutServo,
  70,
  140
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

String statusInfus;

if(alarmMacet)
{
  statusInfus = "MACET";
}
else if(berat <= BATAS_INFUS_HABIS)
{
  statusInfus = "HAMPIR HABIS";
}
else
{
  statusInfus = "NORMAL";
}

    client.publish(
  "infus/berat",
  String(berat,0).c_str()
);
Serial.print("MQTT TPM Kirim: ");
Serial.println(TPM);
client.publish(
  "infus/tpm",
  String(TPM).c_str()
);

client.publish(
  "infus/servo",
  String(sudutServo).c_str()
);
client.publish(
  "infus/status",
  statusInfus.c_str()
);
}

// ======================
// DETEKSI INFUS MACET
// ======================

if(millis() - lastDropDetected > 10000)
{
  alarmMacet = true;
}

// ======================
// BUZZER ALARM
// ======================

if(alarmMacet)
{
  unsigned long pola = millis() % 400;

  if(pola < 200)
  {
    digitalWrite(BUZZER_PIN, HIGH);
  }
  else
  {
    digitalWrite(BUZZER_PIN, LOW);
  }
}

else if(berat <= BATAS_INFUS_HABIS && berat > 0)
{
  unsigned long pola = millis() % 1000;

  if(
      (pola >= 0   && pola < 150) ||
      (pola >= 250 && pola < 400) ||
      (pola >= 500 && pola < 650)
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

static unsigned long lastOLED = 0;

if(millis() - lastOLED >= 500)
{
  lastOLED = millis();

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0,0);
  display.print("Berat:");
  display.print(berat,0);
  display.print(" g");

display.setCursor(0,14);
display.print("TPM:");
display.print(TPM);

display.setCursor(70,14);
display.print("T:");
display.print(TARGET_TPM);

  display.setCursor(0,28);
  display.print("Servo:");
  display.print(sudutServo);

  display.setCursor(0,42);

  if(alarmMacet)
{
  display.print("INFUS MACET!");
}
else if(berat > BATAS_INFUS_HABIS)
{
  display.print("Infus NORMAL");
}
else if(berat > 0)
{
  display.print("HAMPIR HABIS");
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
}

delay(5);
}