#ifndef _SS006_0_0007_H_
#define _SS006_0_0007_H_

#include <avr/pgmspace.h>

class SS006_0_0007_ {
    public:
        static void reset(uint8_t);
        static void setABC(uint8_t, bool);
        static void printConig(uint8_t);
};

extern SS006_0_0007_ SS006_0_0007;

const uint16_t ERRORSTATUS_REGISTER PROGMEM = 0x00;
const uint16_t MEASURED_CONCENTRATION_FILTERED_AND_PRESSURE_COMPENSATED_REGISTER PROGMEM = 0x06;
const uint16_t MEASURE_MODE_EE_REGISTER PROGMEM = 0x95;
const uint16_t MEASURE_PERIOD_EE_REGISTER PROGMEM = 0x96;
const uint16_t NUMBER_OF_SAMPLES_REGISTER PROGMEM = 0x98;
const uint16_t SCR_REGISTER PROGMEM = 0xA5;
const uint16_t METER_CONTROL_EE_REGISTER PROGMEM = 0xA5;

#endif // _SS006_0_0007_H_