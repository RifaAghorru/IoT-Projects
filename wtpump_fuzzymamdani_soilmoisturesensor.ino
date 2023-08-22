#include <ArduinoJson.h>
#include <Fuzzy.h>
#include <LiquidCrystal_I2C.h>
#define BLYNK_PRINT Serial
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>
#include <FirebaseArduino.h>

// Initialize the LCD display
LiquidCrystal_I2C lcd(0x27, 16, 2);

char auth[] = "*****"; // Blynk Auth token
char ssid[] = "wifi_ssid";
char pass[] = "wifi_password";

BlynkTimer timer;
bool Relay = 14;

// Define component pins
#define DHTPIN 2
#define DHTTYPE DHT11
#define sensor A0
#define waterPump 14
#define FIREBASE_HOST "******"
#define FIREBASE_AUTH "******"

DHT dht(DHTPIN, DHTTYPE);
Fuzzy *fuzzy = new Fuzzy();

// FuzzyInput
FuzzySet *kering = new FuzzySet(0, 0, 40, 70);
FuzzySet *lembap = new FuzzySet(65, 70, 80, 90);
FuzzySet *basah = new FuzzySet(90, 90, 100, 100);

// FuzzyOutput
FuzzySet *mati = new FuzzySet(0, 0, 0, 0);
FuzzySet *nyala = new FuzzySet(1, 1, 1, 1);

void setup() {
  Serial.begin(9600);
  dht.begin();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  
  // FuzzyInput
  FuzzyInput *kelembapan = new FuzzyInput(1);
  kelembapan->addFuzzySet(kering);
  kelembapan->addFuzzySet(lembap);
  kelembapan->addFuzzySet(basah);
  fuzzy->addFuzzyInput(kelembapan);

  // FuzzyOutput
  FuzzyOutput *pompa = new FuzzyOutput(1);
  pompa->addFuzzySet(mati);
  pompa->addFuzzySet(nyala);
  fuzzy->addFuzzyOutput(pompa);

  // FuzzyRule
  FuzzyRuleAntecedent *KelembapanKering = new FuzzyRuleAntecedent();
  KelembapanKering->joinSingle(kering);
  
  FuzzyRuleConsequent *PompaNyala = new FuzzyRuleConsequent();
  PompaNyala->addOutput(nyala);
  
  FuzzyRule *fuzzyRule01 = new FuzzyRule(1, KelembapanKering, PompaNyala);
  fuzzy->addFuzzyRule(fuzzyRule01);

  FuzzyRuleAntecedent *KelembapanLembapAndBasah = new FuzzyRuleAntecedent();
  KelembapanLembapAndBasah->joinWithAND(lembap, basah);
  
  FuzzyRuleConsequent *PompaMati = new FuzzyRuleConsequent();
  PompaMati->addOutput(mati);
  
  FuzzyRule *fuzzyRule02 = new FuzzyRule(2, KelembapanLembapAndBasah, PompaMati);
  fuzzy->addFuzzyRule(fuzzyRule02);
  
  digitalWrite(waterPump, LOW);
  pinMode(waterPump, OUTPUT);

  lcd.begin();
  lcd.backlight();

  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  lcd.setCursor(1, 0);
  lcd.print("System Loading");
  for (int a = 0; a <= 15; a++) {
    lcd.setCursor(a, 1);
    lcd.print(".");
    delay(500);
  }
  lcd.clear();
  timer.setInterval(100L, soilMoistureSensor);
}

void soilMoistureSensor() {
  int moist = analogRead(sensor);
  moist = map(moist, 0, 1024, 0, 100);
  moist = (moist - 100) * -1;

  lcd.setCursor(0, 0);
  lcd.print("Kelembapan:");
  lcd.print(moist);
  lcd.print(" %");

  lcd.setCursor(0, 1);
  lcd.print("KONDISI:");

  if (moist <= 70) {
    lcd.setCursor(8, 1);
    lcd.print("KERING");
  } else if (moist >= 70 && moist < 90) {
    lcd.setCursor(8, 1);
    lcd.print("LEMBAB");
  } else {
    lcd.setCursor(8, 1);
    lcd.print("BASAH");
  }
}

void loop() {
  int kelembapan = analogRead(sensor);
  kelembapan = map(kelembapan, 0, 1024, 0, 100);
  kelembapan = (kelembapan - 100) * -1;
  int suhu = dht.readTemperature();

  Blynk.virtualWrite(V0, kelembapan);
  Blynk.virtualWrite(V1, suhu);

  Serial.print("Kelembapan Tanah : ");
  Serial.print(kelembapan);
  Serial.print(" %");
  Serial.println("");
  Serial.print("Suhu : ");
  Serial.print(suhu);
  Serial.print(" °C");
  Serial.println("");

  fuzzy->setInput(1, kelembapan);
  fuzzy->fuzzify();
  int pompa = fuzzy->defuzzify(1);
  digitalWrite(waterPump, pompa);

  Serial.print("Hasil : ");
  Serial.print(pompa);
  Serial.println("");
  String fireHumid = String(kelembapan) + String("%");
  delay(2000);

  Firebase.pushString("/Data/Soil Moisture", fireHumid);
  if (Firebase.failed()) {
    Serial.print("pushing /logs failed:");
    Serial.println(Firebase.error());
    return;
  }

  Blynk.run();
  timer.run();
}
