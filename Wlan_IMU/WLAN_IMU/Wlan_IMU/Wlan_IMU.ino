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
#define declination 3.3f  //http://www.ngdc.noaa.gov/geomag-web/#declination . This is the declinarion in the easterly direction in degrees.  
#define TEST_SAMPLRATE false

MPU9250 mpu;
IMUResult magResult, accResult, gyroResult, velResult;
NetworkManager networkManager;
NetData::IMUData netData;

//DisplayTest
DisplayHelper display;
const bool useDisplay = false;

float samplingRateInMicros = 33 * 1000;

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


	//mpu.selfTest();
	int32_t gX, gY, gZ;
	float aX, aY, aZ;
	bool success = ConfigManager::LoadGyroCalibration(gX, gY, gZ);
	success = success && ConfigManager::LoadAccelCalibration(aX, aY, aZ);
	if (success)
	{
		mpu.manualCalibrate(gX, gY, gZ, aX, aY, aZ);
	}
	else
	{
		Serial.println("Failed to load Sensor Bias");
	}
	mpu.init();

	float f1, f2, f3;
	success = ConfigManager::LoadMagnetCalibration(f1, f2, f3);
	if (success)
	{
		mpu.setMagCalibrationManually(f1, f2, f3);
	}
	else
	{
		Serial.println("Failed to load magnetometer bias");
	}
	if (useDisplay)
	{
		display.ClearAndDisplay("MPU9250 calibrated!");
	}
}

void SensorCalibCallback()
{
	mpu.autoCalibrate();
	ConfigManager::Begin();
	ConfigManager::SaveAccelCalibration(mpu.accelBias[0], mpu.accelBias[1], mpu.accelBias[2]);
	ConfigManager::SaveGyroCalibration(mpu.gyroBias[0], mpu.gyroBias[1], mpu.gyroBias[2]);
	setupMPU();
	ConfigManager::End();
}

void MagCalibCallback()
{
	digitalWrite(LED_BUILTIN, LOW);
	mpu.magCalibrate();
	ConfigManager::Begin();
	ConfigManager::SaveMagnetCalibration(mpu.magBias[0], mpu.magBias[1], mpu.magBias[2]);
	digitalWrite(LED_BUILTIN, HIGH);
	setupMPU();
	ConfigManager::End();
}

void SampleRateCallback(int32_t newRate)
{
	samplingRateInMicros = newRate;
	ConfigManager::Begin();
	ConfigManager::SaveSamplingRate(samplingRateInMicros);
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
	setupMPU();
	int32_t rate = ConfigManager::LoadSamplingRate();
	if (rate > 0)
	{
		samplingRateInMicros = rate;
	}
	else
	{
		ConfigManager::SaveSamplingRate(samplingRateInMicros);
	}
	ConfigManager::End();

	//Serial.println("Gyro Bias:");
	//Serial.print("X: ");
	//Serial.print(mpu.gyroBias[0]);
	//Serial.print("  Y: ");
	//Serial.print(mpu.gyroBias[1]);
	//Serial.print("  Z: ");
	//Serial.println(mpu.gyroBias[2]);

	//Serial.println("Accel Bias:");
	//Serial.print("X: ");
	//Serial.print(mpu.accelBias[0]);
	//Serial.print("  Y: ");
	//Serial.print(mpu.accelBias[1]);
	//Serial.print("  Z: ");
	//Serial.println(mpu.accelBias[2]);

	networkManager.Begin();
	networkManager.SetCallbackOnMagCalibration(&MagCalibCallback);
	networkManager.SetCallbackOnNewSampleRate(&SampleRateCallback);
	networkManager.SetCallbackOnCallibrateSensor(&SensorCalibCallback);
}

uint32_t lastUpdate = 0;
uint32_t lastSample = 0;
uint32_t lastDataCount = 0;

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

		//readOrientation(&orientResult, declination);
		readVelocity(&velResult);

		//float x, y, z;
		//x = accResult.getXComponent();
		//y = accResult.getYComponent();
		//z = accResult.getZComponent();

		//float length = sqrt(x*x + y*y + z*z);

		//Serial.print("X: ");
		//Serial.print(x);
		//Serial.print("Y: ");
		//Serial.print(y);
		//Serial.print("Z: ");
		//Serial.println(z);
		//Serial.print("Len: ");
		//Serial.println(length);

		//orientResult.printResult();
		//gyroResult.printResult();

		if (TEST_SAMPLRATE)
		{
			float samplingRate = 1000000.0f / (samplingRateInMicros / lastDataCount);
			if (useDisplay)
			{
				display.ClearDisplay();
				display.println("Sampling rate in HZ:");
				display.println(samplingRate);
				display.print(" @");
				display.print(lastDataCount);
				display.printlnAndDisplay(" Samples");
			}
			else
			{
				Serial.println("Sampling rate in HZ:");
				Serial.println(samplingRate);
				Serial.print(" @");
				Serial.print(lastDataCount);
				Serial.println(" Samples");
			}
		}

		netData.timeStampt = lastSample;

		netData.rotation[0] = getQ()[0];
		netData.rotation[1] = getQ()[1];
		netData.rotation[2] = getQ()[2];
		netData.rotation[3] = getQ()[3];

		netData.velocity[0] = velResult.getXComponent();
		netData.velocity[1] = velResult.getYComponent();
		netData.velocity[2] = velResult.getZComponent();

		networkManager.WriteData(netData);

		if (TEST_SAMPLRATE)
			lastDataCount = 0;
		ResetVelocity();
	}
}