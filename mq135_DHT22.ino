/*************   Necessary libraries for the project    **************************/

#include <Blynk.h>   
#include <Wire.h>
#include <DHT.h>
#include <MQ135.h> 
#include <LiquidCrystal_I2C.h>  
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

/******    Humidity and temperature sensor DHT-11 paramters and variables    *****/

#define BLYNK_PRINT Serial
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float t, h;
LiquidCrystal_I2C lcd(0x27, 16, 2);

/******************    MQ-135 sensor parameteres and variables    ****************/

#define RZERO 1 
MQ135 gasSensor = MQ135(A0);
int val; 
int sensorPin = A0; 
int sensorValue = 0;
float ppm, zero;

/*****************    LED, Buzzer, LCD Config  ****************************************************/

#define led_green 14
#define led_red 12
#define buzzer 13

//Icon
byte temp[] =
{
  B00100,
  B01010,
  B01010,
  B01110,
  B11111,
  B11111,
  B01110,
  B00000
};

byte hum[] =
{
  B00100,
  B01110,
  B11111,
  B11111,
  B11111,
  B11111,
  B01110,
  B00000
};

/*************   BLYNK    **************************/

#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "****"
#define BLYNK_TEMPLATE_NAME "Air Quality Sensor"
#define BLYNK_AUTH_TOKEN "*****"

char ssid[] = "wifi_ssid";
char pass[] = "wifi_password";

BlynkTimer timer; 
void myTimer()
{
  Blynk.virtualWrite(V0, ppm);
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h); 
} 
/***********************   SETUP BLOCK    ***************************************/

void setup( )
{
  Serial.begin(9600);	
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);						              
  lcd.begin();                            // LCD initialization
  lcd.backlight();
  dht.begin();                            // DHT-22 initialization
  delay(2000);

  
  pinMode(sensorPin, INPUT);					    // MQ-135 pin data
  pinMode(buzzer, OUTPUT);
  pinMode(led_green, OUTPUT);
  pinMode(led_red, OUTPUT);
  digitalWrite(led_green, LOW);
  digitalWrite(led_red, LOW);
  
  timer.setInterval(500L, myTimer);
}

/***********************    LOOP BLOCK    *************************************/

void loop( ) {

  t = dht.readTemperature();					    // Read temperature
  h = dht.readHumidity();					        // Read Humidity

  if (isnan(h) || isnan(t) || isnan(ppm)) {					    // when sensor read error
    Serial.println("Failed to read sensor data!!!");
    lcd.setCursor(8,0);
    lcd.print("Error   ");
    lcd.setCursor(10,1);
    lcd.print("Error   ");
    return;
  }

  val = analogRead(A0);						        // CO2 read
  zero = gasSensor.getRZero();					  // Calibration value for the MQ-135 sensor
  ppm = gasSensor.getPPM();					      // Formula in the library for obtaín the ppm of CO2
  ppm = ppm * 10; 
  
  lcd.createChar(1, temp);
  lcd.createChar(2, hum);
  lcd.setCursor(1,0);
  lcd.write(1);
  lcd.print(":");
  lcd.setCursor(10,0);
  lcd.write(2);
  lcd.print(":");
  lcd.setCursor(3,0);
  lcd.print(t,0);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(12,0);
  lcd.print(h,0);
  lcd.print("%");
  lcd.setCursor(3,1);
  lcd.print("CO2:");
  lcd.print(ppm,0);
  lcd.print("ppm");

  Serial.print( "T = " );					        // Show values by the Serial Port
  Serial.print(t);
  Serial.print(" ºC, H = ");
  Serial.print(h);
  Serial.print( "%, " );
  Serial.print (", ppm: "); 
  Serial.println (ppm);
  delay (3000);
  lcd.clear();
  
  if (ppm >= 800) {                      // If the CO2 concentration is equal o most than 800
    digitalWrite(led_red, HIGH);         // Turn on de red LED
    digitalWrite(buzzer, HIGH);          // Buzzer sound
  } else {
    digitalWrite(led_red, LOW);
    digitalWrite(buzzer, LOW);  }
  
  if (ppm <= 800)  {
    digitalWrite(led_green, HIGH);        // The green LED turn on only when the CO2
  } else { 
    digitalWrite(led_green, LOW); }       // concentration is most low than 551

  Blynk.run();
  timer.run();
}            
