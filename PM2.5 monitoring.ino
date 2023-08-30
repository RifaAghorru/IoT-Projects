#include <Blynk.h>   
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <GP2YDustSensor.h>
#include "DHT.h"
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_PRINT Serial
#define DHTPIN 2
#define DHTTYPE DHT22 

#define BLYNK_TEMPLATE_ID "TMPL6HbzxwGwr"
#define BLYNK_TEMPLATE_NAME "Skripsi Project"
#define BLYNK_AUTH_TOKEN "AC1E_naOCdKSp_CC9eU-oJ4eoN0f7NPU"

#define i2c_Address 0x3c 

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET -1   
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const uint8_t SHARP_LED_PIN = 14;
const uint8_t SHARP_VO_PIN = A0; 

GP2YDustSensor dustSensor(GP2YDustSensorType::GP2Y1010AU0F, SHARP_LED_PIN, SHARP_VO_PIN);
DHT dht(DHTPIN, DHTTYPE);

char ssid[] = "testport";
char pass[] = "ikanteri";

float hum;
float temp;
float p = dustSensor.getDustDensity();
float h = dht.readHumidity();
float t = dht.readTemperature();


BlynkTimer timer; 
void myTimer() 
{
  Blynk.virtualWrite(V1, dustSensor.getDustDensity());
  Blynk.virtualWrite(V2, hum);
  Blynk.virtualWrite(V3, temp); 
}

void setup()   {
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  dustSensor.begin();
  dht.begin();
  timer.setInterval(500L, myTimer);
  display.begin(i2c_Address, true);
  display.clearDisplay();
  display.display();
}



 void loop(){
  //dht calibrate
  hum = dht.readHumidity();
  temp = dht.readTemperature();
  hum = hum + 28.0;
  temp = temp - 9; 
  Serial.print("Dust density: ");
  Serial.print(dustSensor.getDustDensity());
  Serial.println(" ug/m3");
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println(" %");
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" °C");
  delay(500);
  
  display.setTextSize(1.9);
  display.setTextColor(SH110X_WHITE);
  
  display.setCursor(0, 15);
  display.write("PM2.5: ");
  display.print(dustSensor.getDustDensity());
  display.println(" ug/m3");
  
  display.setCursor(0, 30);
  display.write("Humidity: ");
  display.print(hum);
  display.print("%");
  
  display.setCursor(0, 45);
  display.write("Temp: ");
  display.print(temp); 
  display.print(char(247));
  display.print("C");
  
  display.display();
  display.clearDisplay();
  delay(1000);
  
  Blynk.run();
  timer.run();
}
