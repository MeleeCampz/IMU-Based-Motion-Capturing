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
#define calibrateMagnetometer false  //Setting requires requires you to move device in figure 8 pattern when prompted over serial port.  Typically, you do this once, then manually provide the calibration values moving forward.

MPU9250 mpu;
IMUResult magResult, accResult, gyroResult, orientResult;
NetworkManager networkManager;

//DisplayTest
DisplayHelper display;
const bool useDisplay = true;

void setupMPU()
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
	if (calibrateMagnetometer)
	{
		mpu.magCalibrate();
		ConfigManager::SaveMagnetCalibration(mpu.magBias[0], mpu.magBias[1], mpu.magBias[2]);
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

void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	Serial.begin(921600);
	delay(250);

	if (useDisplay)
		display.BeginDisplay();

	ConfigManager::Begin();
	setupMPU();
	ConfigManager::End();

	networkManager.Begin();
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
		mpu.readAccelData(&accResult);
		mpu.readGyroData(&gyroResult);
		mpu.readMagData(&magResult);
		if (TEST_SAMPLRATE)
			lastDataCount++;
	}

	float delta = (micros() - lastUpdate) / 1000000.f;
	lastUpdate = micros();
	MahonyQuaternionUpdate(&accResult, &gyroResult, &magResult, delta);
	//MadgwickQuaternionUpdate(&accResult, &gyroResult, &magResult, delta);

	if (micros() - lastSample > samplingRateInMicros)
	{

		readOrientation(&orientResult, declination);
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


		NetData::IMUData data;
		data.rotation[0] = orientResult.getXComponent();
		data.rotation[1] = orientResult.getYComponent();
		data.rotation[2] = orientResult.getZComponent();

		data.acceleration[0] = 111;
		data.acceleration[1] = 222;
		data.acceleration[2] = 333;

		networkManager.WriteData(data);

		if (TEST_SAMPLRATE)
			lastDataCount = 0;

		lastSample = micros();
	}
}