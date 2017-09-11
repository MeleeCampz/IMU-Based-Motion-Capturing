#include <ESP8266WiFi.h>
#include "NetworkManager.h"
#include "ConfigManager.h"
#include "DisplayHelper.h"

#include "MPU9250.h"
#include "QuaternionFilters.h"
#include "IMUResult.h"

//Pin Defines
//Using default pin D2 for SDA
//Using default pin D1 for SDA
const int m_intPin = D3;


///////////////////////////////////////////////////////////////////
//Determines how often we sample data
///////////////////////////////////////////////////////////////////
#define samplingRateInMicros 33 * 1000

///////////////////////////////////////////////////////////////////
//Setup for the Accelerometer
///////////////////////////////////////////////////////////////////
#define declination 3.3f  //http://www.ngdc.noaa.gov/geomag-web/#declination . This is the declinarion in the easterly direction in degrees.  

MPU9250 mpu;
IMUResult magResult, accResult, gyroResult, orientResult, velResult;
NetworkManager networkManager;
NetData::IMUData netData;

//DisplayTest
DisplayHelper display;
const bool useDisplay = true;

void setupMPU(bool calibMag)
{
	//Init MPU
	mpu.begin(SDA, SCL, m_intPin);
	//This tests communication between the accelerometer and the ESP8266.  Dont continue until we get a successful reading.
	byte c = 0x00;
	while (c != 0x73)
	{
		c = mpu.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
		if (c != 0x73)
		{
			Serial.println("Waiting for MPU-9250");
			delay(500);
		}
	}

	// Calibrate gyro and accelerometers, load biases in bias registers, then initialize MPU.
	mpu.selfTest();
	mpu.calibrate();
	mpu.init();
	if (calibMag)
	{
		digitalWrite(LED_BUILTIN, LOW);
		mpu.magCalibrate();
		ConfigManager::SaveMagnetCalibration(mpu.magBias[0], mpu.magBias[1], mpu.magBias[2]);
		digitalWrite(LED_BUILTIN, HIGH);
	}
	else
	{
		float f1, f2, f3;
		ConfigManager::LoadMagnetCalibration(f1, f2, f3);
		mpu.setMagCalibrationManually(f1, f2, f3);    //Set manually with the results of magCalibrate() if you don't want to calibrate at each device bootup.														   														 //mpu.setMagCalibrationManually(0, 0, 0);    //Set manually with the results of magCalibrate() if you don't want to calibrate at each device bootup.														   
	}


	if (useDisplay)
	{
		display.ClearAndDisplay("MPU9250 calibrated!");
	}
}

void MagCalibCallback()
{
	ConfigManager::Begin();
	setupMPU(true);
	ConfigManager::End();
}

void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	Serial.begin(921600);
	delay(250);

	if (useDisplay)
		display.BeginDisplay();

	ConfigManager::Begin();
	setupMPU(false);
	ConfigManager::End();

	networkManager.Begin();
	networkManager.SetCallbackOnMagCalibration(&MagCalibCallback);
}

uint32_t lastUpdate = 0;
uint32_t lastSample = 0;
uint32_t lastDataCount = 0;

#define TEST_SAMPLRATE true

char* sendBuffer = new char[12]; //3floats
float r1, r2, r3;

void loop()
{
	networkManager.Update();

	// If intPin goes high, all data registers have new data
	// On interrupt, check if data ready interrupt
	if (mpu.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
	{
		float delta = (micros() - lastUpdate) / 1000000.f;
		lastUpdate = micros();
		if (TEST_SAMPLRATE)
			lastDataCount++;
		
		mpu.readAccelData(&accResult);
		mpu.readGyroData(&gyroResult);
		mpu.readMagData(&magResult);

		//MahonyQuaternionUpdate(&accResult, &gyroResult, &magResult, delta);
		MadgwickQuaternionUpdate(&accResult, &gyroResult, &magResult, delta);
		IntegrateVelocity(&accResult, delta);
	}

	if (micros() - lastSample > samplingRateInMicros)
	{
		lastSample = micros();

		readOrientation(&orientResult, declination);
		readVelocity(&velResult);

		//Serial.print("X: ");
		//Serial.print(accResult.getXComponent());
		//Serial.print("Y: ");
		//Serial.print(accResult.getYComponent());
		//Serial.print("Z: ");
		//Serial.println(accResult.getZComponent());

		//orientResult.printResult();

		if (TEST_SAMPLRATE)
		{
			if (useDisplay)
			{
				float samplingRate = 1000000.0f / (samplingRateInMicros / lastDataCount);

				display.ClearDisplay();
				display.println("Sampling rate in HZ:");
				display.println(samplingRate);
				display.print(" @");
				display.print(lastDataCount);
				display.printlnAndDisplay(" Samples");
			}
		}

		netData.timeStampt = lastSample;
		
		netData.rotation[0] = orientResult.getXComponent();
		netData.rotation[1] = orientResult.getYComponent();
		netData.rotation[2] = orientResult.getZComponent();

		netData.velocity[0] = velResult.getXComponent();
		netData.velocity[1] = velResult.getYComponent();
		netData.velocity[2] = velResult.getZComponent();

		networkManager.WriteData(netData);

		if (TEST_SAMPLRATE)
			lastDataCount = 0;
		ResetVelocity();
	}
}