#include <EEPROM.h>
#include "GravityTDS.h"
#define analogPin A0
#define EEPROM_SIZE 512

GravityTDS gravityTds;

#include <Firebase.h>
#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#define WIFI_SSID "Redmi Note 10S"
#define WIFI_PASSWORD "lupasaya"

//Relay
#define pump1 14
#define pump3 12
#define pump2 13

//Konfigurasi Firebase
#define FIREBASE_HOST "fir-crud-aaa98-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "g7AHKruoQRR431jhNTB5jxV6RXkOQGnQO9deONKR"

#define s0 D0
#define s1 D1
#define s2 D2

float PH4 = 4.67;
float PH7 = 4.05;

int nilaiInput = 0;

String path3 ="/Target";

//variabel nilai sensor
float phValue;
float tdsValue;
float temperature = 25;

int perintah;

void setup() {
  Serial.begin(115200);
  pinMode(analogPin, INPUT);
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);

  EEPROM.begin(EEPROM_SIZE);
  gravityTds.setPin(analogPin);
  gravityTds.setAref(3.3);  //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(4096);  //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();
  
  pinMode(pump1, OUTPUT);
  pinMode(pump2, OUTPUT);
  pinMode(pump3, OUTPUT);
   
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  digitalWrite(pump1, HIGH);
  digitalWrite(pump2, HIGH);
  digitalWrite(pump3, HIGH);
  
}

void loop() {
  //inisialisasi y0
  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  nilaiInput = analogRead(analogPin);
  float Voltage = 5.0/1023.0*nilaiInput;
  float PH_Step = (PH4-PH7)/3;
  float phValue = 7.0 +((PH7-Voltage)/PH_Step);
  Serial.print("PH = ");
  Serial.println(phValue);
  
  delay(200);
  
  //inisialisasi y1
  digitalWrite(s0, HIGH);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  nilaiInput = analogRead(analogPin);
  gravityTds.setTemperature(temperature);  // set the temperature and execute temperature compensation
  gravityTds.update();
  tdsValue = gravityTds.getTdsValue();  // then get the value
  Serial.print("tdsValue = ");
  Serial.print(tdsValue);
  Serial.println(" ppm");
  
  delay(200);

  // set PhValue   
  Firebase.setFloat("/Data/PhValue", phValue);
  // set TdsValue
  Firebase.setFloat("Data/TdsValue", tdsValue);
  delay(200);
 
  /*
  int phUp = Firebase.getInt("/Relay/Pompa1");
  int phDown = Firebase.getInt("/Relay/Pompa2");
  int tdsUp = Firebase.getInt("/Relay/Pompa3");
  
  if(phUp == 1){
      //Serial.println("PhUp menyala");
      digitalWrite(pump1, LOW);
  } else {
    digitalWrite(pump1, HIGH);
  }
  
  if(phDown == 1){
      //Serial.println("PhDown menyala");
      digitalWrite(pump2, LOW);  
  } else {
    digitalWrite(pump2, HIGH);
  }
  
  if(tdsUp == 1){
      //Serial.println("TdsUp menyala");
      digitalWrite(pump3, LOW);
  } else {
    digitalWrite(pump3, HIGH);
  }

  delay(200);
  */
  otomatis();
  
  if(perintah == 1){
    pompa();
    Serial.println("manual");
  }else{
    otomatisasi();
    Serial.println("otomatis");
  }
}

int otomatis(){
  perintah = Firebase.getInt("/Data/Perintah");
  return perintah;
}

void pompa(){ 
  int phUp = Firebase.getInt("/Relay/Pompa1");
  int phDown = Firebase.getInt("/Relay/Pompa2");
  int tdsUp = Firebase.getInt("/Relay/Pompa3");
  
  if(phUp == 1){
      //Serial.println("PhUp menyala");
      digitalWrite(pump1, LOW);
  } else {
    digitalWrite(pump1, HIGH);
  }
  
  if(phDown == 1){
      //Serial.println("PhDown menyala");
      digitalWrite(pump2, LOW);  
  } else {
    digitalWrite(pump2, HIGH);
  }
  
  if(tdsUp == 1){
      //Serial.println("TdsUp menyala");
      digitalWrite(pump3, LOW);
  } else {
    digitalWrite(pump3, HIGH);
  }
}

void otomatisasi(){
  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  nilaiInput = analogRead(analogPin);
  float Voltage = 5.0/1023.0*nilaiInput;
  float PH_Step = (PH4-PH7)/3;
  float phValue = 7.0 +((PH7-Voltage)/PH_Step);
  
  delay(200);
  
  //inisialisasi y1
  digitalWrite(s0, HIGH);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  gravityTds.setTemperature(temperature);  // set the temperature and execute temperature compensation
  gravityTds.update();
  tdsValue = gravityTds.getTdsValue(); 
  
  delay(200);

  if(phValue >= 7){
    Serial.println("Basa ditambahkan");
    digitalWrite(pump1, HIGH);
    digitalWrite(pump2, LOW);
  }
  else if(phValue <= 5){
    Serial.println("Asam ditambahkan");
    digitalWrite(pump1, LOW);
    digitalWrite(pump2, HIGH);
  }
  
  if(tdsValue <= 560.0){
    Serial.println("TDS ditambah");
    digitalWrite(pump3, LOW);
  } else {
    Serial.println("pompa tds mati");
    digitalWrite(pump3, HIGH);
  }
}
