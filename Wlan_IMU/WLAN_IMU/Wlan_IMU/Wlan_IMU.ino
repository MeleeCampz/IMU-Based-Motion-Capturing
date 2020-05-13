//Arduino
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

//Manager classes
#include "NetworkManager.h"
#include "ConfigManager.h"
#include "OTAHelper.h"

//IMU
#include "MPU9250.h"
#include "QuaternionFilters.h"
#include "IMUResult.h"

//Version
static const int FW_VERSION = 14;
//Server for OTA Update
const char* fwUrl = "http://meleecampz.ddns.net/OTAUpdate/Wlan_IMU.bin";

//Pin Defines
//Using default pin D2 for SDA
//Using default pin D1 for SDA
const int m_intPin = D3;
#define TEST_SAMPLRATE true

MPU9250 mpu;
IMUResult magResult, accResult, gyroResult, velResult;
NetworkManager networkManager;
NetData::IMUData netData;

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

void OTAUpdate()
{
	OTAHelper::StartOTA(fwUrl);
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
	delay(100);

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

	Serial.print("Current FW Verison: ");
	Serial.println(FW_VERSION);

	networkManager.Begin();
	networkManager.SetCallbackOnMagCalibration(&MagCalibCallback);
	networkManager.SetCallbackOnNewSampleRate(&SampleRateCallback);
	networkManager.SetCallbackOnCallibrateSensor(&SensorCalibCallback);
	networkManager.SetCallbackOTAUpdate(&OTAUpdate);
}

uint32_t lastUpdate = 0;
uint32_t lastSample = 0;
#if TEST_SAMPLRATE
uint32_t lastDataCount = 0;
#endif

void loop()
{
	//Update delta time
	float delta = (micros() - lastUpdate) / 1000000.f;
	lastUpdate = micros();

#if TEST_SAMPLRATE
	lastDataCount++;
#endif

	//Update network manager
	networkManager.Update();

	// If intPin goes high, all data registers have new data
	// On interrupt, check if data ready interrupt
	if (mpu.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
	{		
		mpu.readAccelData(&accResult);
		mpu.readGyroData(&gyroResult);
		mpu.readMagData(&magResult);
	}

	//Perform update steps even if no new data was read
	MadgwickQuaternionUpdate(&accResult, &gyroResult, &magResult, delta);
	IntegrateVelocity(&accResult, delta);


	//Check if we should send a new network package, based on adjustable sampling rate
	if (micros() - lastSample > samplingRateInMicros)
	{
		readVelocity(&velResult);

		//Generate a netData object that is serialized over the network to the client application
		netData.timeStampt = lastSample;

		netData.rotation[0] = getQ()[0];
		netData.rotation[1] = getQ()[1];
		netData.rotation[2] = getQ()[2];
		netData.rotation[3] = getQ()[3];

		netData.velocity[0] = velResult.getXComponent() / lastDataCount;
		netData.velocity[1] = velResult.getYComponent() / lastDataCount;
		netData.velocity[2] = velResult.getZComponent() / lastDataCount;

		//Send data
		networkManager.WriteData(netData);

		//Reset integrated velocity back to 0
		ResetVelocity();
		lastSample = micros();

#if TEST_SAMPLRATE
		float samplingRate = 1000000.0f / (samplingRateInMicros / lastDataCount);
		Serial.println("Sampling rate in HZ:");
		Serial.println(samplingRate);
		Serial.print(" @");
		Serial.print(lastDataCount);
		Serial.println(" Samples");

		lastDataCount = 0;
#endif
	}
}