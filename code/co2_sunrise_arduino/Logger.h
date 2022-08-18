#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <SPI.h>
#include <SD.h>

class Logger_ {
    public:
        static void init();

        static void enableSerial(bool);
        static void setBaudRate(int);

        static void enabledSD(bool);
        static void setSDLogFileName(const String &);
        static void setSDCardPin(int8_t);

        static void print(const String &);
        static void print(const char[]);
        static void print(int8_t);
        static void print(uint8_t);
        static void print(uint16_t);

        static void println(const String &);
        static void println(const char[]);
        static void println(int8_t);
        static void println(uint8_t);
        static void println(uint16_t);

    private:
        static bool shouldSerial;
        static int serialBaudRate;

        static bool shouldSD;
        static String sdLogFileName;
        static int8_t sdCardPin;
        static File sdLogFile;

        static bool hasInitizalized;
};

extern Logger_ Logger;

#endif // _LOGGER_H_