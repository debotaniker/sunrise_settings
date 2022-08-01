#include <Wire.h>
#include "RTClib.h"
#include <SPI.h>
#include <SD.h>

#define WIRE_WORKAROUND   (0)
#define DISABLE_ABC       (1)/* It could be necessary to disable ABC if sensor will be tested with CO2 concentrations below 400ppm. */

/*
   Sensor konfiguration
*/
const uint8_t   SUNRISE_ADDR            = 0x10;/* Sunrise communication address, both for Modbus and I2C */
const int       ATTEMPTS                 = 5;/* Amount of wakeup attempts before time-out */
const int       EEPROM_UPDATE_DELAY_MS   = 25;/* It takes 25ms to write one EE register */

const uint8_t ERROR_STATUS             = 0x01;/* Register Addresses */
const uint8_t MEASUREMENT_MODE         = 0x95;/* Register Addresses */
const uint8_t METER_CONTROL            = 0xA5;/* Register Addresses */

const uint16_t CONTINUOUS              = 0x0000;/* Measurement modes */
const uint16_t SINGLE                  = 0x0001;/* Measurement modes */


int readPeriodMs = 4000;/* Reading period, in milliseconds. Default is 4 seconds */

/*
   Logger shield konfiguration
*/
const byte chipSelect = 10; //select pin for SD card
RTC_DS1307 rtc; //definition of RTC used
int start;
File myFile;
char dateBuffer[12];  //buffer to store date
//char sensBuffer[10]; // buffer to store sensor data

/*
   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   Functions used to start the sensor and communicate with it.
   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

*/
void  reInitI2C() {
  Wire.begin();/* Initialize I2C and use default pins defined for the board */
  Wire.setClock(100000);   /* Setup I2C clock to 100kHz */
}

int WireRequestFrom(uint8_t dev_addr, uint8_t bytes_numbers, uint8_t offset_to_read, bool stop) {
  int error;
#if (WIRE_WORKAROUND == 1)
  error = Wire.requestFrom((uint8_t)dev_addr, (uint8_t)bytes_numbers /* how many bytes */, (uint32_t)offset_to_read /* from address*/, (uint8_t)1/* Address size - 1 byte*/, stop /* STOP*/);
#else
  Wire.beginTransmission(dev_addr);
  Wire.write(offset_to_read); //starting register address, from which read data
  Wire.endTransmission(false);
  error = Wire.requestFrom((uint8_t)dev_addr, (uint8_t)bytes_numbers /* how many bytes */, (uint8_t)stop /* STOP*/);
#endif
  return error;
}

bool _wakeup(uint8_t target) {
  int attemps = ATTEMPTS;
  int error;

  do {
    uint8_t byte_0;
    /* */
    Wire.beginTransmission(target);
    error = Wire.endTransmission(true);
  } while (((error != 0 /*success */) && (error != 2 /*Received NACK on transmit of address*/) && (error != 1 /* BUG in STM32 library*/)) && (--attemps > 0));
  /* STM32 driver can stack under some conditions */
  if (error == 4) {
    /* Reinitialize I2C*/
    reInitI2C();
    return false;
  }
  return (attemps > 0);
}

void read_sensor_config(uint8_t target) {
  /* Function variables */
  int error;
  int numBytes = 7;

  /* Wakeup */
  if (!(_wakeup(target))) {
    Serial.print(F("Failed to wake up sensor."));
    return;
  }

  /* Request values */
  error = WireRequestFrom(target, numBytes, MEASUREMENT_MODE /* from address*/, true /* STOP*/);
  if (error != numBytes ) {
    Serial.print(F("Failed to write to target. Error code : "));
    Serial.println(error);
    return;
  }

  /* Read values */
  /* Measurement mode */
  uint8_t measMode = Wire.read();

  /* Measurement period */
  uint8_t byteHi = Wire.read();
  uint8_t byteLo = Wire.read();
  uint16_t measPeriod = ((int16_t)(int8_t) byteHi << 8) | (uint16_t)byteLo;

  /* Number of samples */
  byteHi = Wire.read();
  byteLo = Wire.read();
  uint16_t numSamples = ((int16_t)(int8_t) byteHi << 8) | (uint16_t)byteLo;

  /* ABCPeriod */
  byteHi = Wire.read();
  byteLo = Wire.read();
  uint16_t abcPeriod = ((int16_t)(int8_t) byteHi << 8) | (uint16_t)byteLo;

  /* Most propable that the sensor will not go into sleep mode, but to be insure...*/
  /* Wakeup */
  if (!(_wakeup(target))) {
    Serial.print("Failed to wake up sensor.");
    return;
  }

  /* Request values */
  error = WireRequestFrom(target, 1, METER_CONTROL /* from address*/, true /* STOP*/);
  if (error != 1 ) {
    Serial.print(F("Failed to write to target. Error code : "));
    Serial.println(error);
    return;
  }

  uint8_t  meterControl = Wire.read();

  Serial.print(F("Measurement Mode: "));
  Serial.println(measMode);

  readPeriodMs = measPeriod * 1000;

  Serial.print(F("Measurement Period, sec: "));
  Serial.println(measPeriod);

  Serial.print(F("Number of Samples: "));
  Serial.println(numSamples);

  if ((0U == abcPeriod) ||  (0xFFFFU == abcPeriod) || (meterControl & 0x02U)) {
    Serial.println("ABCPeriod: disabled");
  } else {
    Serial.print(F("ABCPeriod, hours: "));
    Serial.println(abcPeriod);
  }

  Serial.print(F("MeterControl: "));
  Serial.println(meterControl, HEX);

#if (DISABLE_ABC ==0)
  /* If we do not implicity disable ABC try to enable it in case if someone forget to do that...*/
  if ((meterControl & 0x02U) != 0) {
    setABC(target, true);
  }
#endif
}

void setABC(uint8_t target, bool enable) {
  /* Wakeup */
  if (!(_wakeup(target))) {
    Serial.print(F("Failed to wake up sensor."));
    return;
  }

  /* Request values */
  int error = WireRequestFrom(target, 1, METER_CONTROL /* from address*/, true /* STOP*/);
  if (error != 1 ) {
    Serial.print(F("Failed to write to target. Error code : "));
    Serial.println(error);
    return;
  }

  uint8_t  meterControl = Wire.read();

  if (enable) {
    Serial.println(F("Enabling ABC..."));
    meterControl &= (uint8_t)~0x02U;
  } else {
    Serial.println(F("Disabling ABC..."));
    meterControl |= 0x02U;
  }

  /* Wakeup */
  if (!(_wakeup(target))) {
    Serial.print(F("Failed to wake up sensor."));
    return;
  }

  Wire.beginTransmission(target);
  Wire.write(METER_CONTROL);
  Wire.write(meterControl);
  error = Wire.endTransmission(true);
  delay(EEPROM_UPDATE_DELAY_MS);

  if (error != 0) {
    Serial.print(F("Failed to send request. Error code: "));
    Serial.println(error);
    /* FATAL ERROR */
    while (true);
  }
  Serial.println();
}
void change_measurement_mode(uint8_t target) {
  /* Function variables */
  int error;
  int numBytes = 1;

  /* Wakeup */
  if (!(_wakeup(target))) {
    Serial.print(F("Failed to wake up sensor."));
    /* FATAL ERROR */
    while (true);
  }

  /* Read Value */
  error = WireRequestFrom(target, numBytes /* how many bytes */, MEASUREMENT_MODE /* from address*/, true /* STOP*/);
  if (error != numBytes ) {
    Serial.print(F("Failed to read measurement mode. Error code: "));
    Serial.println(error);
    /* FATAL ERROR */
    while (true);
  }

  /* Change mode if single */
  if (Wire.read() != CONTINUOUS) {
    /* Wakeup */
    if (!(_wakeup(target))) {
      Serial.print(F("Failed to wake up sensor."));
      /* FATAL ERROR */
      while (true);
    }

    Serial.println(F("Changing Measurement Mode to Continuous..."));

    Wire.beginTransmission(target);
    Wire.write(MEASUREMENT_MODE);
    Wire.write(CONTINUOUS);
    error = Wire.endTransmission(true);
    delay(EEPROM_UPDATE_DELAY_MS);

    if (error != 0) {
      Serial.print(F("Failed to send request. Error code: "));
      Serial.println(error);
      /* FATAL ERROR */
      while (true);
    }
    Serial.println(F("Sensor restart is required to apply changes"));
    while (true);
  }
}
int read_sensor_measurements(uint8_t target) {
  /* Function variables */
  int error;
  int numBytes = 7;

  /* Wakeup */
  if (!(_wakeup(target))) {
    Serial.print(F("Failed to wake up sensor."));
    return;
  }

  /* Request values */
  error = WireRequestFrom(target, numBytes /* how many bytes */, ERROR_STATUS /* from address*/, true /* STOP*/);
  if (error != numBytes ) {
    Serial.print(F("Failed to read values. Error code: "));
    Serial.println(error);
    return;
  }

  /* Read values */
  /* Error status */
  uint8_t eStatus = Wire.read();

  /* Reserved */
  uint8_t byteHi = Wire.read();
  uint8_t byteLo = Wire.read();

  byteHi = Wire.read();
  byteLo = Wire.read();

  /* CO2 value */
  uint16_t co2;
  byteHi = Wire.read();
  byteLo = Wire.read();
  uint16_t co2Val = ((int16_t)(int8_t) byteHi << 8) | (uint16_t)byteLo;

  //  Serial.print("CO2: ");
  //  Serial.print(co2Val);
  //  Serial.println(" ppm");
  //
  //  Serial.print("Error Status: 0x");
  //  Serial.println(eStatus, HEX);

  return co2Val;
}

void setup()  {
  delay(1000);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  reInitI2C(); //Initialize I2C and use default pins defined for the board

  Serial.begin(115200);
  Serial.println(F("Initialization complete\n"));

  Serial.println(F("Sensor Measurement Configurations:"));
  read_sensor_config(SUNRISE_ADDR);
  Serial.println();

  //#if (DISABLE_ABC ==1)
  //  setABC(SUNRISE_ADDR, false);
  //#endif

  /* Change measurement mode if single */
  change_measurement_mode(SUNRISE_ADDR);
  delay(readPeriodMs);

  /*
    Initialize the RTC
  */
  if (! rtc.begin()) {
    Serial.println(F("No RTC"));
    Serial.flush();
    abort();
  } else {
    Serial.println(F("RTC is running"));
  }
  if (! rtc.isrunning()) {
    Serial.println(F("RTC was off. Setting new time."));
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  Serial.println("RTC old time:");
  DateTime old = rtc.now();
  sprintf(dateBuffer, "%02u-%02u-%04u", old.day(), old.month(), old.year());
  Serial.print(dateBuffer);
  Serial.print(",");
  sprintf(dateBuffer, "%02u:%02u:%02u", old.hour(), old.minute(), old.second());
  Serial.print(dateBuffer);
  Serial.println();

  Serial.println(F("Adjusting RTC time to Now"));
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  Serial.println(F("RTC new time:"));
  DateTime neu = rtc.now();
  sprintf(dateBuffer, "%02u-%02u-%04u", neu.day(), neu.month(), neu.year());
  Serial.print(dateBuffer);
  Serial.print(F(","));
  sprintf(dateBuffer, "%02u:%02u:%02u", neu.hour(), neu.minute(), neu.second());
  Serial.println(dateBuffer);
  Serial.println();

  /*
    Initialize the SD card and check for a logger file
  */
  Serial.print(F("Initializing SD card... "));
  if (!SD.begin(chipSelect)) {
    Serial.println(F("SD initialization failed"));
    while (1);
  } else {
    Serial.println(F("initialization done."));
  }
  if (SD.exists("afclog.txt")) {
    Serial.println(F("afclog.txt exists."));
  } else {
    Serial.println(F("afclog.txt doesn't exist."));
    Serial.println(F("Now creating afclog.txt ..."));
    myFile = SD.open("afclog.txt", FILE_WRITE);
    myFile.close();
    if (SD.exists("afclog.txt")) {
      Serial.println(F("SUCCESS afclog.txt was created."));
    } else {
      Serial.println(F("FAILED"));
    }
  }

  DateTime time = rtc.now();
  start = millis();
  Serial.println(F("Writing header to logfile"));
  myFile = SD.open("afclog.txt", FILE_WRITE);
  myFile.println("date,time,co2,co2_unit,start,now");
  sprintf(dateBuffer, "%02u-%02u-%04u", time.day(), time.month(), time.year());
  myFile.print(dateBuffer);
  myFile.print(",");
  sprintf(dateBuffer, "%02u:%02u:%02u", time.hour(), time.minute(), time.second());
  myFile.print(dateBuffer);
  myFile.print(",");
  myFile.print("0");
  myFile.print(",");
  myFile.print(start);
  myFile.print("0");
  myFile.close();

  Serial.println();
  Serial.println(F("Measurements:"));
  Serial.println();
}

void loop() {

  myFile = SD.open("afclog.txt", FILE_WRITE);    //open the log file
  if (myFile) {
    DateTime time = rtc.now();

    unsigned long timenow = millis();

    sprintf(dateBuffer, "%02u-%02u-%04u", time.day(), time.month(), time.year());
    Serial.print(dateBuffer);
    Serial.print(F(","));
    myFile.print(dateBuffer);    // write in the file
    myFile.print(",");    // write in the file

    sprintf(dateBuffer, "%02u:%02u:%02u", time.hour(), time.minute(), time.second());
    Serial.print(dateBuffer);
    Serial.print(F(","));
    myFile.print(dateBuffer);    // write in the file
    myFile.print(",");    // write in the file

    /* Read measurements */
    read_sensor_measurements(SUNRISE_ADDR);

    /* Delay between readings
      Serial.print(F("Wait "));
      Serial.print(readPeriodMs);
      Serial.println(F(" milliseconds"));
    */

    Serial.print(read_sensor_measurements(SUNRISE_ADDR));
    Serial.print(F(","));
    myFile.print(read_sensor_measurements(SUNRISE_ADDR));    // write in the file
    myFile.print(",");    // write in the file

    Serial.print(start);
    Serial.print(F(","));
    myFile.print(start);    // write in the file
    myFile.print(",");    // write in the file

    Serial.println(timenow);
    myFile.println(timenow);    // write in the file

    myFile.close();    // close the file:
  } else {
    Serial.println(F("Logging not available"));
  }
  delay(readPeriodMs);
}
