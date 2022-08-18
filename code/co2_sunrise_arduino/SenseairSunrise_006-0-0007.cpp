#include "SenseairSunrise_006-0-0007.h"
#include "WireExtended.h"
#include "Logger.h"

SS006_0_0007_ SS006_0_0007;

void SS006_0_0007_::reset(uint8_t deviceAddress)
{
    WireWriteInt8(deviceAddress, SCR_REGISTER, 0xFFU);
    delay(500);
    WireWriteInt8(deviceAddress, SCR_REGISTER, 0x00U);
    delay(500);
}

void SS006_0_0007_::printConig(uint8_t deviceAddress)
{
    Logger.println(F("---------------------------------------"));
    Logger.println(F("-Sensor Measurement Configurations-"));

    Logger.print(F("Measurement Mode: "));
    if (WireReadBit(deviceAddress, MEASURE_MODE_EE_REGISTER, 0))
    {
        Logger.println(F("Single"));
    }
    else
    {
        Logger.println(F("Continuous"));
    }

    Logger.print(F("Measurement Period, sec: "));
    Logger.println(WireReadInt16(deviceAddress, MEASURE_PERIOD_EE_REGISTER));

    Logger.print(F("Number of Samples: "));
    Logger.println(WireReadInt16(deviceAddress, NUMBER_OF_SAMPLES_REGISTER));

    Logger.print(F("ABCPeriod: "));
    if (WireReadBit(deviceAddress, METER_CONTROL_EE_REGISTER, 1))
    {
        Logger.println(F("Disabled"));
    }
    else
    {
        Logger.println(F("Enabled"));
    }

    Logger.print(F("MeterControl: "));
    Logger.println(WireReadInt8(deviceAddress, METER_CONTROL_EE_REGISTER));
    Logger.println(F("---------------------------------------"));
}

void SS006_0_0007_::setABC(uint8_t deviceAddress, bool state)
{
    if (state==WireReadBit(deviceAddress, METER_CONTROL_EE_REGISTER, 1))
    {
        Logger.println(F("Changing ABC..."));
        if (!WireWriteBit(deviceAddress, METER_CONTROL_EE_REGISTER, 1, !state))
        {
            Logger.println(F("Failed to change ABC."));
            return;
        }

        delay(100);

        SS006_0_0007_::reset(deviceAddress);

        delay(100);

        if (state==WireReadBit(deviceAddress, METER_CONTROL_EE_REGISTER, 1))
        {
            Logger.println(F("Failed to change ABC."));
            return;
        }
    }

    if (state)
    {
        Logger.println(F("ABC is enabled."));
    }
    else
    {
        Logger.println(F("ABC is disabled."));
    }
}