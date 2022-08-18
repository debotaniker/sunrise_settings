#ifndef _WIREEXTENDED_H_
#define _WIREEXTENDED_H_

#include <Wire.h>
#include "Logger.h"

extern bool WireShouldWakeup;
extern int  WireWakeupAttempts;

extern bool Error;
extern int8_t ErrorCode;

extern int I2CClockspeed;
extern int WireDelayMs;

extern bool HasInitializedOnce;

bool Wakeup(uint8_t);
void  ReInitI2C();

bool WireWriteInt16(uint8_t, uint8_t, uint16_t);
uint16_t WireReadInt16(uint8_t, uint8_t);

bool WireWriteInt8(uint8_t, uint8_t, uint8_t);
uint8_t WireReadInt8(uint8_t, uint8_t);

bool WireWriteBit(uint8_t, uint8_t, uint8_t, bool);
bool WireReadBit(uint8_t, uint8_t, uint8_t);

#endif // _WIREEXTENDED_H_