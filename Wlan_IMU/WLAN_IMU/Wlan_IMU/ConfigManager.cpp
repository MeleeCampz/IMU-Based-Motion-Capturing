#include "ConfigManager.h"

const char* ConfigManager::FILE_PATH = "/config.txt";
const char* ConfigManager::KEY_ID = "ID";
const char* ConfigManager::KEY_MAGNETCONFIG_1 = "MAG_CNFG_1";
const char* ConfigManager::KEY_MAGNETCONFIG_2 = "MAG_CNFG_2";
const char* ConfigManager::KEY_MAGNETCONFIG_3 = "MAG_CNFG_3";
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

bool ConfigManager::LoadMagnetCalibration(float & out_m1, float & out_m2, float & out_m3)
{
	bool b1 = ReadKey(KEY_MAGNETCONFIG_1, out_m1);
	bool b2 = ReadKey(KEY_MAGNETCONFIG_2, out_m2);
	bool b3 = ReadKey(KEY_MAGNETCONFIG_3, out_m3);

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