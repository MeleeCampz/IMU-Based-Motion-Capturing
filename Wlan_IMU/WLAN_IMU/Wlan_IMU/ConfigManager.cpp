#include "ConfigManager.h"

const char* ConfigManager::FILE_PATH = "/config.txt";
const char* ConfigManager::KEY_ID = "ID";
const char* ConfigManager::KEY_MAGNETCONFIG_1 = "MAG_CNFG_1";
const char* ConfigManager::KEY_MAGNETCONFIG_2 = "MAG_CNFG_2";
const char* ConfigManager::KEY_MAGNETCONFIG_3 = "MAG_CNFG_3";
const char* ConfigManager::KEY_ACCELCONFIG_1 = "ACC_CNFG_1";
const char* ConfigManager::KEY_ACCELCONFIG_2 = "ACC_CNFG_2";
const char* ConfigManager::KEY_ACCELCONFIG_3 = "ACC_CNFG_3";
const char* ConfigManager::KEY_GYROCONFIG_1 = "GYR_CNFG_1";
const char* ConfigManager::KEY_GYROCONFIG_2 = "GYR_CNFG_2";
const char* ConfigManager::KEY_GYROCONFIG_3 = "GYR_CNFG_3";
const char* ConfigManager::KEY_SAMPLING_RATE = "SMPL_RATE";

void ConfigManager::Begin()
{
	SPIFFS.begin();
}

void ConfigManager::End()
{
	SPIFFS.end();
}

bool ConfigManager::SaveID(int16_t id)
{
	return WriteKeyValue(KEY_ID, id);
}

int16_t ConfigManager::LoadID() 
{
	int16_t out = -1;
	ReadKey(KEY_ID, out);
	return out;
}

bool ConfigManager::SaveMagnetCalibration(float m1, float m2, float m3)
{
	bool b1 = WriteKeyValue(KEY_MAGNETCONFIG_1, m1);
	bool b2 = WriteKeyValue(KEY_MAGNETCONFIG_2, m2);
	bool b3 = WriteKeyValue(KEY_MAGNETCONFIG_3, m3);

	return b1 && b2 && b3;
}

bool ConfigManager::LoadMagnetCalibration(float &out_m1, float &out_m2, float &out_m3)
{
	bool b1 = ReadKey(KEY_MAGNETCONFIG_1, out_m1);
	bool b2 = ReadKey(KEY_MAGNETCONFIG_2, out_m2);
	bool b3 = ReadKey(KEY_MAGNETCONFIG_3, out_m3);

	return b1 && b2 && b3;
}

bool ConfigManager::SaveAccelCalibration(float aX, float aY, float aZ)
{
	bool b1 = WriteKeyValue(KEY_ACCELCONFIG_1, aX);
	bool b2 = WriteKeyValue(KEY_ACCELCONFIG_2, aY);
	bool b3 = WriteKeyValue(KEY_ACCELCONFIG_3, aZ);

	return b1 && b2 && b3;
}

bool ConfigManager::LoadAccelCalibration(float &out_aX, float &out_aY, float &out_aZ)
{
	bool b1 = ReadKey(KEY_ACCELCONFIG_1, out_aX);
	bool b2 = ReadKey(KEY_ACCELCONFIG_2, out_aY);
	bool b3 = ReadKey(KEY_ACCELCONFIG_3, out_aZ);

	return b1 && b2 && b3;
}

bool ConfigManager::SaveGyroCalibration(int32_t gX, int32_t gY, int32_t gZ)
{
	bool b1 = WriteKeyValue(KEY_GYROCONFIG_1, gX);
	bool b2 = WriteKeyValue(KEY_GYROCONFIG_2, gY);
	bool b3 = WriteKeyValue(KEY_GYROCONFIG_3, gZ);

	return b1 && b2 && b3;
}

bool ConfigManager::LoadGyroCalibration(int32_t &out_gX, int32_t &out_gY, int32_t &out_gZ)
{
	bool b1 = ReadKey(KEY_GYROCONFIG_1, out_gX);
	bool b2 = ReadKey(KEY_GYROCONFIG_2, out_gY);
	bool b3 = ReadKey(KEY_GYROCONFIG_3, out_gZ);

	return b1 && b2 && b3;
}

bool ConfigManager::SaveSamplingRate(int32_t rate)
{
	return WriteKeyValue(KEY_SAMPLING_RATE, rate);
}

int32_t ConfigManager::LoadSamplingRate()
{
	int32_t out = -1;
	ReadKey(KEY_SAMPLING_RATE, out);
	return out;
}