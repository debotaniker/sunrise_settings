#include "RTClib.h"
#include <SD.h>
#include "WireExtended.h"
#include "Logger.h"
#include <avr/pgmspace.h>
#include "SenseairSunrise_006-0-0007.h"

/*---------------*/
/* Configuration */
/*---------------*/

#define ENABLESERIALLOG                 true        /* Enables or disables the log to serial, if the arduino is connected to a computer. */
#define SERIALBAUD                      115200      /* Defines the baud-rate for the serial communiction with the computer. */

#define ENABLESDLOG                     false        /* Enables or disables the log to the SD-Card. */
#define SDLOGFILE                       "log.txt"   /* Defines the name of the log-file on the SD-Card. */

#define WIREWAKEUPATTEMPTS              5           /* How often should wire try to wakeup a device via I2C before it fails */
#define I2CCLOCKSPEED                   50000      /* Defines the clock speed in Hz for the I2C communction with the sensor (and other peripherals). */

#define SDCARDPIN                       10          /* Defines the io-pin for the SD-Card. */

const uint8_t SensorAddress PROGMEM =   0x10U;      /* Defines the address of the sensor on I2C. */

/*------------------*/
/* Global Variables */
/*------------------*/

/*-------*/
/* Setup */
/*-------*/

void setup()
{
    /* Connect the EN-(Enabled)-Pin of the Sensor to Pin 2 to enable it. */
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);

    /*------------------------------*/
    /* Setup Serial Communication   */
    /*------------------------------*/
    
    #if (ENABLESERIALLOG)
        Serial.begin(SERIALBAUD);
        while (!Serial) { ; }
    #endif

    /*------------------------------*/
    /* Setup Logger                 */
    /*------------------------------*/

    Logger.enableSerial(ENABLESERIALLOG);
    Logger.setSDLogFileName(F(SDLOGFILE));
    Logger.enabledSD(ENABLESDLOG);
    Logger.init();

    /*------------------------------*/
    /* Setup Sensor Communication   */
    /*------------------------------*/
    
    WireShouldWakeup = true;    /* Set Wire should always send a 'Wake Up' via I2C as it is required by the sensor. */
    WireWakeupAttempts = WIREWAKEUPATTEMPTS;
    I2CClockspeed = I2CCLOCKSPEED;
    
    SS006_0_0007.printConig(SensorAddress);
    SS006_0_0007.setABC(SensorAddress, true);
}

/*------*/
/* Loop */
/*------*/

void loop()
{
    
}