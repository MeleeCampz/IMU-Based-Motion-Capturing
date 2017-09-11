#ifndef _QUATERNIONFILTERS_H_
#define _QUATERNIONFILTERS_H_

#include <Arduino.h>
#include "IMUResult.h"

void MadgwickQuaternionUpdate(IMUResult * acc, IMUResult * gyro, IMUResult * mag, float deltat);
void MahonyQuaternionUpdate(IMUResult * acc, IMUResult * gyro, IMUResult * mag, float deltat);
void readOrientation(IMUResult *orien, float dec);
const float * getQ();

void IntegrateVelocity(IMUResult* acc, float deltaTime);
void ResetVelocity();
void readVelocity(IMUResult* vel);
const float* GetVelocity();

#endif // _QUATERNIONFILTERS_H_
