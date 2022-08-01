#include<Wire.h>

int SunriseAddress = 0x68;

void setup() {

  // put your setup code here, to run once:

  Serial.begin(115200);

  Wire.begin();

  Wire.setClock(100000);

  delay(500);

}

void loop() {

  int error;

  //wakeup;

  Wire.beginTransmission(SunriseAddress);

  error = Wire.endTransmission(true);



  //Read CO2

  Wire.beginTransmission(SunriseAddress);

  Wire.write(0xA7);

  //Neue Adresse

  Wire.write(0x10);

  error = Wire.endTransmission(false);

  delay(10);

  Wire.beginTransmission(SunriseAddress);

  Wire.write(0xA3);

  Wire.write(0xFF);

  error = Wire.endTransmission(false);

  Wire.requestFrom(SunriseAddress, 2, true);

}
