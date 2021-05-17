#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <MPU6050_light.h>
#include <String>
#include <String.h>
#include<Tello.h>

// ------------ //
// declaration //
// ---------- //

using namespace std;
MPU6050 mpu(Wire);
const uint8_t scl = D6;
const uint8_t sda = D7;
unsigned long timer = 0;
int x,y,z;
int x_1,y_1,z_1;
int lr,fb,up,yv;
int value = 50 , threshold = 20, threshold_gyro = 100;
// HARDWARE CONNECTION
const int ONBOARD_LED = 2;
const int BUTTON_TAKEOFF = D1;
const int BUTTON_LAND = D2;

// tello settings
const char* DEFAULT_SSID = "deadgost";
const char* DEFAULT_PW = "karan123";
const char* TELLO_IP = "192.168.10.1";
const int PORT = 8889;

// state constants
const int INIT = 0;
const int CONNECTED = 1;
bool take_off = false;
bool xx=false,yy=false,zz=false;
WiFiUDP Udp;
char packetBuffer[255]; //buffer to hold incoming packet
int networkState = 0;

Tello tello;

// --- Setup ---- //
void setup() {
  Serial.begin(9600);
  //WIFI
  //connecting to Wifi 
  Serial.println("Connecting to ");
  Serial.println(DEFAULT_SSID);
  WiFi.begin(DEFAULT_SSID, DEFAULT_PW);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Button
  pinMode(D8, INPUT);
  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(ONBOARD_LED, LOW);
  Serial.println("initialise ..");
  tello.init();
  tello.get_battery();
  
  //MPU
  Wire.begin(sda, scl);
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while (status != 0) {
    Serial.print("MPU6050 not working !! \n");
   } // stop everything if could not connect to MPU6050
   delay(1000);
   mpu.calcOffsets(); // gyro and accelero
   Serial.println("Done!\n");
   for(int i=0;i<8;i++)
   {
     digitalWrite(ONBOARD_LED, i%2);
     delay(500);
   }
}

// -- Loop --- //
void loop(){
  mpu.update();
  if ((millis() - timer) > 10) { 
    x = mpu.getAngleX();
    y = mpu.getAngleY();
    z = mpu.getAngleZ();
    x_1 = abs(mpu.getGyroX());
    y_1 = abs(mpu.getGyroY());
    z_1 = abs(mpu.getGyroZ());
    // 
     Serial.println("\n");
    Serial.println("Angle : ");
    Serial.print("X : ");
    Serial.print(x);
    Serial.print("\tY : ");
    Serial.print(y);
    Serial.print("\tZ : ");
    Serial.println(z);
    Serial.println("Gyro : ");
    Serial.print("x_1 : ");
    Serial.print(x_1);
    Serial.print("\ty_1 : ");
    Serial.print(y_1);
    Serial.print("\tz_1 : ");
    Serial.println(z_1);
    
    Serial.print("x_1 : ");
    Serial.print(x_1 > threshold_gyro);
    Serial.print("\ty_1 : ");
    Serial.print(y_1 > threshold_gyro);
    Serial.print("\tz_1 : ");
    Serial.println(z_1 > threshold_gyro);
    timer = millis();
  }
  if(digitalRead(D8) && !take_off)
  {
    Serial.print("takeoff");
    Serial.println(F("Calculating offsets, do not move MPU6050"));
    delay(1000);
    mpu.calcOffsets(); // gyro and accelero
    Serial.println("Done!\n");
    tello.takeoff();
    delay(500);
    take_off =! take_off;
    digitalWrite(ONBOARD_LED, HIGH);
    delay(500);
  }
  else if(digitalRead(D8) && take_off)
  {
    Serial.print("land");
    tello.land();
    delay(500);
    take_off =! take_off;
    digitalWrite(ONBOARD_LED, LOW);
    delay(500);
  }
  if( take_off){
    lr = 0,fb = 0,up = 0,yv = 0;
    // X
    if(x_1 > threshold_gyro || xx){
      if(x>threshold){
        fb = -value;
        xx = true;
      }
      else if(x < -threshold){
        fb = value;
        xx = true;
      }
      else{
        xx = false;
      }
    }
    // Y
    if( y_1 > threshold_gyro || yy)
    {
      if(y > threshold){
        lr = value;
        yy = true;
      }
      else if(y  < -threshold){
        lr = -value;
        yy = true;
      }
      else{
        yy = false;
      }
    }
    // Z
    if(z_1 > threshold_gyro){
      
      if(z >threshold){
        yv = -value;
        zz = true;
      }
      else if(z  < -threshold){
        yv = value;
        zz = true;
      }
      else{
        zz = false;
      }
    }     
    Serial.print("lr: ");
    Serial.print(lr);
    Serial.print("\t fb: ");
    Serial.print(fb);
    Serial.print("\t up: ");
    Serial.print(up);
    Serial.print("\t yv: ");
    Serial.println(yv);
  
    tello.sendRCcontrol(lr,fb,up,yv);
  }
}
