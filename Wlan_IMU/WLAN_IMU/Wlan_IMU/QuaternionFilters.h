//Original source code: https://github.com/sparkfun/MPU-9250_Breakout/tree/master/Libraries/Arduino/src

#ifndef _QUATERNIONFILTERS_H_
#define _QUATERNIONFILTERS_H_

#include <Arduino.h>
#include "IMUResult.h"

void MadgwickQuaternionUpdate(IMUResult * acc, IMUResult * gyro, IMUResult * mag, float deltat);
void MahonyQuaternionUpdate(IMUResult * acc, IMUResult * gyro, IMUResult * mag, float deltat);
void FilterUpdate(IMUResult * acc, IMUResult * gyro, IMUResult * mag, float deltat);
const float * getQ();

void IntegrateVelocity(IMUResult* acc, float deltaTime);
void ResetVelocity();
void readVelocity(IMUResult* vel);
const float* GetVelocity();

#endif // _QUATERNIONFILTERS_H_
