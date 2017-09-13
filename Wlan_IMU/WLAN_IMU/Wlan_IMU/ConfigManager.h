#pragma once
#include "Arduino.h"
#include "ArduinoJson.h"
#include "FS.h"

class ConfigManager
{
private:
	ConfigManager() {};
	~ConfigManager() {};
	
	static const char* FILE_PATH;
	static const char* KEY_ID;
	static const char* KEY_MAGNETCONFIG_1;
	static const char* KEY_MAGNETCONFIG_2;
	static const char* KEY_MAGNETCONFIG_3;
	static const char* KEY_SAMPLING_RATE;
	static const uint32_t JSON_BUFFER_SIZE = 512;

	template<typename T>
	inline static bool WriteKeyValue(const char* key, T value)
	{
		File configFile = SPIFFS.open(FILE_PATH, "r");
		if (!configFile)
		{
			Serial.println("Failed to load existing File, creating new one");
			return WriteKeyValueToNewFile(key, value);
		}

		size_t size = configFile.size();
		std::unique_ptr<char[]> buf(new char[size]);
		configFile.readBytes(buf.get(), size);


		StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(buf.get());

		if (!root.success())
		{
			return WriteKeyValueToNewFile(key, value);
		}

		configFile = SPIFFS.open(FILE_PATH, "w");
		root.set<T>(key, value);
		root.printTo(configFile);
		return true;
	};
	template<typename T>
	inline static bool ReadKey(const char* key, T& outValue)
	{
		File configFile = SPIFFS.open(FILE_PATH, "r");
		if (!configFile)
		{
			Serial.println("Failed to open or create config file for reading");
			return false;
		}

		size_t size = configFile.size();
		std::unique_ptr<char[]> buf(new char[size]);
		configFile.readBytes(buf.get(), size);

		StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(buf.get(), size);

		if (!root.success())
		{
			Serial.println("parseObject() failed");
			return false;
		}

		if (!root.containsKey(key))
		{
			return false;
		}

		outValue = root.get<T>(key);
		return true;
	};

	template<typename T>
	inline static bool WriteKeyValueToNewFile(const char* key, T value)
	{
		StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();
		if (!root.success())
		{
			return false;
		}
		File configFile = SPIFFS.open(FILE_PATH, "w");
		root.set<T>(key, value);
		root.printTo(configFile);
		return true;
	}
public:

	static void Begin();
	static void End();
	static bool SaveID(int16_t id);
	static int16_t LoadID();
	static bool SaveMagnetCalibration(float m1, float m2, float m3);
	static bool LoadMagnetCalibration(float& out_m1, float& out_m2, float& out_m3);
	static bool SaveSamplingRate(int32_t rate);
	static int32_t LoadSamplingRate();
};
