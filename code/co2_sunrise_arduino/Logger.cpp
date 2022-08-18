#include "Logger.h"

Logger_ Logger;

bool Logger_::shouldSerial = true;
int Logger_::serialBaudRate = 9600;

bool Logger_::shouldSD = false;
String Logger_::sdLogFileName = "";
int8_t Logger_::sdCardPin = 10;
File Logger_::sdLogFile;

bool Logger_::hasInitizalized = false;

void Logger_::init()
{
    if (!Serial && shouldSerial)
    {
        Serial.begin(serialBaudRate);
        while (!Serial)
        {
            ;
        }
    }
    Serial.println(F("---------------------------------------"));
    Serial.println(F("-Startup Logger-\n"));

    if (shouldSD)
    {
        Serial.println(F("Initializing SD card..."));
        while (!SD.begin(sdCardPin))
        {
            Serial.println(F("Failed! Retry in 10 seconds."));
            delay(10000);
        }
        Serial.println(F("Done.\n"));

        Serial.println(F("Initializing Logfile..."));
        sdLogFile = SD.open(sdLogFileName, FILE_WRITE);
        delay(50);
        sdLogFile.close();
        delay(50);
        sdLogFile = SD.open(sdLogFileName, FILE_WRITE);

        if (!SD.exists(sdLogFileName) || !sdLogFile)
        {
            Serial.println(F("Failed to open or create log-file."));
            Serial.println(F("Please restart the controller."));
            Serial.println(F("---------------------------------------"));
        }
        Serial.println(F("Done."));

        sdLogFile.println(F("---------------------------------------"));
    }

    hasInitizalized = true;

    println(F("Startup Loggers: Success."));
    println(F("---------------------------------------"));
}

void Logger_::enableSerial(bool state)
{
    shouldSerial = true;
    if (hasInitizalized && !Serial)
    {
        println("Reinitialize Logger...");

        if (shouldSD && sdLogFile)
            sdLogFile.close();
        
        init();
    }
}

void Logger_::setBaudRate(int baudRate)
{
    serialBaudRate = baudRate;
    if (hasInitizalized && shouldSerial)
    {
        println("Reinitialize Logger...");
    
        if (shouldSD && sdLogFile)
            sdLogFile.close();
        
        init();
    }
}

void Logger_::enabledSD(bool state)
{
    shouldSD = state;
    if (hasInitizalized && !sdLogFile)
    {
        println("Reinitialize Logger...");

        init();
    }
}

void Logger_::setSDLogFileName(const String &fileName)
{
    sdLogFileName = fileName;
    if (hasInitizalized && shouldSD)
    {
        println("Reinitialize Logger...");

        if (sdLogFile)
            sdLogFile.close();

        init();
    }
}

void Logger_::setSDCardPin(int8_t pin)
{
    sdCardPin = pin;
    if (hasInitizalized && shouldSD)
    {
        println("Reinitialize Logger...");

        if (sdLogFile)
            sdLogFile.close();

        init();
    }
}


void Logger_::print(const String &printable)
{
    if (!hasInitizalized) init();
    if (shouldSerial && Serial) Serial.print(printable);
    if (shouldSD && sdLogFile) sdLogFile.print(printable);
}
void Logger_::print(const char printable[])
{
    if (!hasInitizalized) init();
    if (shouldSerial && Serial) Serial.print(printable);
    if (shouldSD && sdLogFile) sdLogFile.print(printable);
}
void Logger_::print(int8_t printable)
{
    if (!hasInitizalized) init();
    if (shouldSerial && Serial) Serial.print(printable);
    if (shouldSD && sdLogFile) sdLogFile.print(printable);
}
void Logger_::print(uint8_t printable)
{
    if (!hasInitizalized) init();
    if (shouldSerial && Serial) Serial.print(printable);
    if (shouldSD && sdLogFile) sdLogFile.print(printable);
}
void Logger_::print(uint16_t printable)
{
    if (!hasInitizalized) init();
    if (shouldSerial && Serial) Serial.print(printable);
    if (shouldSD && sdLogFile) sdLogFile.print(printable);
}

void Logger_::println(const String &printable)
{
    if (!hasInitizalized) init();
    if (shouldSerial && Serial) Serial.println(printable);
    if (shouldSD && sdLogFile) sdLogFile.println(printable);
}
void Logger_::println(const char printable[])
{
    if (!hasInitizalized) init();
    if (shouldSerial && Serial) Serial.println(printable);
    if (shouldSD && sdLogFile) sdLogFile.println(printable);
}
void Logger_::println(int8_t printable)
{
    if (!hasInitizalized) init();
    if (shouldSerial && Serial) Serial.println(printable);
    if (shouldSD && sdLogFile) sdLogFile.println(printable);
}
void Logger_::println(uint8_t printable)
{
    if (!hasInitizalized) init();
    if (shouldSerial && Serial) Serial.println(printable);
    if (shouldSD && sdLogFile) sdLogFile.println(printable);
}
void Logger_::println(uint16_t printable)
{
    if (!hasInitizalized) init();
    if (shouldSerial && Serial) Serial.println(printable);
    if (shouldSD && sdLogFile) sdLogFile.println(printable);
}