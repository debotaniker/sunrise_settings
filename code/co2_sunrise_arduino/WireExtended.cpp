#include "WireExtended.h"

bool WireShouldWakeup = false;
int WireWakeupAttempts = 5;
bool Error = false;
int8_t ErrorCode = 0;
int I2CClockspeed = 100000;
int WireDelayMs = 25;
bool HasInitializedOnce = false;

void ReInitI2C()
{
    Wire.begin();                 /* Initialize I2C and use default pins defined for the board */
    Wire.setClock(I2CClockspeed); /* Setup I2C clock to 100kHz (default) */
}

bool Wakeup(uint8_t deviceAddress)
{
    if (!HasInitializedOnce)
    {
        ReInitI2C();
        HasInitializedOnce=true;
    }

    int attemps = WireWakeupAttempts;

    do
    {
        uint8_t byte_0;
        /* */
        Wire.beginTransmission(deviceAddress);
        ErrorCode = Wire.endTransmission(true);
    } while (((ErrorCode != 0 /*success */) && (ErrorCode != 2 /*Received NACK on transmit of address*/) && (ErrorCode != 1 /* BUG in STM32 library*/)) && (--attemps > 0));

    if ((ErrorCode != 0) && (ErrorCode != 1) && (ErrorCode != 2))
    {
        Error = true;
    }
    else
    {
        Error = false;
        ErrorCode = 0;
    }

    /* STM32 driver can stack under some conditions */
    if (ErrorCode == 4)
    {
        /* Reinitialize I2C*/
        ReInitI2C();
    }

    return !Error;
}

bool WireWriteInt16(uint8_t deviceAddress, uint8_t offset, uint16_t value)
{
    /*Wakeup!*/
    if (!(Wakeup(deviceAddress)))
    {
        Logger.print(F("WireWriteInt16 failed while Wakeup. Error code : "));
        Logger.println(ErrorCode);
        return false;
    }

    /* Convert int16 to int8 MSB LSB */
    uint8_t byteHi = (uint8_t)value >> 8;
    uint8_t byteLo = (uint8_t)value;

    /* Sending MSB to register */
    Wire.beginTransmission(deviceAddress);
    Wire.write(offset);
    Wire.write(byteHi);
    ErrorCode = Wire.endTransmission(true);
    if (ErrorCode != 0)
    {
        Logger.print(F("WireWriteInt16 failed while EndTransmission. Error code : "));
        Logger.println(ErrorCode);
        Error = true;
        return false;
    }
    delay(WireDelayMs);

    /*Wakeup!*/
    if (!(Wakeup(deviceAddress)))
    {
        Logger.print(F("WireWriteInt16 failed while Wakeup. Error code : "));
        Logger.println(ErrorCode);
        return false;
    }

    /* Sending LSB to register */
    Wire.beginTransmission(deviceAddress);
    Wire.write(++offset);
    Wire.write(byteLo);
    ErrorCode = Wire.endTransmission(true);
    if (ErrorCode != 0)
    {
        Logger.print(F("WireWriteInt16 failed while EndTransmission. Error code : "));
        Logger.println(ErrorCode);
        Error = true;
        return false;
    }
    delay(WireDelayMs);

    /* Without errors return true */
    Error = false;
    ErrorCode = 0;
    return true;
}

uint16_t WireReadInt16(uint8_t deviceAddress, uint8_t offset)
{
    /*Functional Variables*/
    uint8_t count = 0;
    uint8_t byteLo;
    uint8_t byteHi;

    /*Wakeup!*/
    if (!(Wakeup(deviceAddress)))
    {
        Logger.print(F("WireReadInt16 failed while Wakeup. Error code : "));
        Logger.println(ErrorCode);
        return false;
    }

    /* Reading both MSB and LSB from register */
    Wire.beginTransmission(deviceAddress);
    Wire.write(offset);
    ErrorCode = Wire.endTransmission(false);
    if (ErrorCode != 0)
    {
        Logger.print(F("WireReadInt16 failed while EndTransmission. Error code : "));
        Logger.println(ErrorCode);
        Error = true;
        return false;
    }
    if (Wire.requestFrom((uint8_t)deviceAddress, (uint8_t)2 /* how many bytes */, (uint8_t) true /* STOP*/) != 2)
    {
        ErrorCode = -2;
        Logger.print(F("WireReadInt16 failed while RequestFrom. Error code : "));
        Logger.println(ErrorCode);
        Error = true;
        return false;
    }
    while (Wire.available())
    {
        if (count++ == 0)
        {
            byteHi = (uint8_t)Wire.read();
        }
        else
        {
            byteLo = (uint8_t)Wire.read();
        }
    }

    /* Without errors return the rescieved value. */
    Error = false;
    ErrorCode = 0;
    return ((uint16_t)(uint8_t)byteHi << 8) | (uint16_t)byteLo;
}

bool WireWriteInt8(uint8_t deviceAddress, uint8_t offset, uint8_t value)
{
    /*Wakeup!*/
    if (!(Wakeup(deviceAddress)))
    {
        Logger.print(F("WireWriteInt8 failed while Wakeup. Error code : "));
        Logger.println(ErrorCode);
        return false;
    }

    /* Sending value to register */
    Wire.beginTransmission(deviceAddress);
    Wire.write(offset);
    Wire.write(value);
    ErrorCode = Wire.endTransmission(true);
    if (ErrorCode != 0)
    {
        Logger.print(F("WireWriteInt8 failed while EndTransmission. Error code : "));
        Logger.println(ErrorCode);
        Error = true;
        return false;
    }
    delay(WireDelayMs);

    /* Without errors return true */
    Error = false;
    ErrorCode = 0;
    return true;
}

uint8_t WireReadInt8(uint8_t deviceAddress, uint8_t offset)
{
    /*Functional Variables*/
    uint8_t value;

    /*Wakeup!*/
    if (!(Wakeup(deviceAddress)))
    {
        Logger.print(F("WireReadInt8 failed while Wakeup. Error code : "));
        Logger.println(ErrorCode);
        return false;
    }

    /* Reading value from register */
    Wire.beginTransmission(deviceAddress);
    Wire.write(offset);
    ErrorCode = Wire.endTransmission(false);
    if (ErrorCode != 0)
    {
        Logger.print(F("WireReadInt8 failed while EndTransmission. Error code : "));
        Logger.println(ErrorCode);
        Error = true;
        return false;
    }
    if (Wire.requestFrom((uint8_t)deviceAddress, (uint8_t)1 /* how many bytes */, (uint8_t) true /* STOP*/) != 1)
    {
        ErrorCode = -2;
        Logger.print(F("WireReadInt8 failed while RequestFrom. Error code : "));
        Logger.println(ErrorCode);
        Error = true;
        return false;
    }
    while (Wire.available())
    {
        value = (uint8_t)Wire.read();
    }

    /* Without errors return the rescieved value. */
    Error = false;
    ErrorCode = 0;
    return value;
}

bool WireWriteBit(uint8_t deviceAddress, uint8_t offset, uint8_t position, bool state)
{
    /*Functional Variables*/
    uint8_t value;

    /*Wakeup!*/
    if (!(Wakeup(deviceAddress)))
    {
        Logger.print(F("WireWriteBit failed while Wakeup. Error code : "));
        Logger.println(ErrorCode);
        return false;
    }

    /* Reading value from register */
    Wire.beginTransmission(deviceAddress);
    Wire.write(offset);
    ErrorCode = Wire.endTransmission(false);
    if (ErrorCode != 0)
    {
        Logger.print(F("WireWriteBit failed while EndTransmission. Error code : "));
        Logger.println(ErrorCode);
        Error = true;
        return false;
    }
    if (Wire.requestFrom((uint8_t)deviceAddress, (uint8_t)1 /* how many bytes */, (uint8_t) true /* STOP*/) != 1)
    {
        ErrorCode = -2;
        Logger.print(F("WireWriteBit failed while RequestFrom. Error code : "));
        Logger.println(ErrorCode);
        Error = true;
        return false;
    }
    while (Wire.available())
    {
        value = (uint8_t)Wire.read();
    }

    /* Get current state. */
    uint8_t mask = 0x01U<<position;

    /* Modify state in variable. */
    if (state) {
        value |= mask;
    } else {
        value &= (uint8_t)~mask;
    }

    /*Wakeup!*/
    if (!(Wakeup(deviceAddress)))
    {
        Logger.print(F("WireWriteBit failed while Wakeup. Error code : "));
        Logger.println(ErrorCode);
        return false;
    }

    /* Sending modified byte to register */
    Wire.beginTransmission(deviceAddress);
    Wire.write(offset);
    Wire.write(value);
    ErrorCode = Wire.endTransmission(true);
    if (ErrorCode != 0)
    {
        Logger.print(F("WireWriteBit failed while EndTransmission. Error code : "));
        Logger.println(ErrorCode);
        Error = true;
        return false;
    }
    delay(WireDelayMs);

    /* Without errors return true */
    Error = false;
    ErrorCode = 0;
    return true;
}

bool WireReadBit(uint8_t deviceAddress, uint8_t offset, uint8_t position)
{
    /*Functional Variables*/
    uint8_t value;

    /*Wakeup!*/
    if (!(Wakeup(deviceAddress)))
    {
        Logger.print(F("WireWriteBit failed while Wakeup. Error code : "));
        Logger.println(ErrorCode);
        return false;
    }

    /* Reading value from register */
    Wire.beginTransmission(deviceAddress);
    Wire.write(offset);
    ErrorCode = Wire.endTransmission(false);
    if (ErrorCode != 0)
    {
        Logger.print(F("WireWriteBit failed while EndTransmission. Error code : "));
        Logger.println(ErrorCode);
        Error = true;
        return false;
    }
    if (Wire.requestFrom((uint8_t)deviceAddress, (uint8_t)1 /* how many bytes */, (uint8_t) true /* STOP*/) != 1)
    {
        ErrorCode = -2;
        Logger.print(F("WireWriteBit failed while RequestFrom. Error code : "));
        Logger.println(ErrorCode);
        Error = true;
        return false;
    }
    while (Wire.available())
    {
        value = Wire.read();
    }

    /* Get current state. */
    uint8_t mask = 0x01U<<position;

    value &= mask;
    
    return value!=0;
}