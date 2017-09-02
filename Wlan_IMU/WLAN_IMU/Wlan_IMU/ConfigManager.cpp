#include "ConfigManager.h"



ConfigManager::ConfigManager()
{
	SPIFFS.begin();
}


ConfigManager::~ConfigManager()
{
	SPIFFS.end();
}


int16_t ConfigManager::loadIDFromFlash() 
{
	File configFile = SPIFFS.open("/config.txt", "r");
	if (!configFile) {
		Serial.println("Failed to open config file");
		return -1;
	}

	size_t size = configFile.size();
	if (size > 1024) {
		Serial.println("Config file size is too large");
		return -1;
	}

	// Allocate a buffer to store contents of the file.
	std::unique_ptr<char[]> buf(new char[size]);

	// We don't use String here because ArduinoJson library requires the input
	// buffer to be mutable. If you don't use ArduinoJson, you may as well
	// use configFile.readString instead.
	size_t fileSize = configFile.readBytes(buf.get(), size);

	if (fileSize <= 0)
	{
		Serial.println("Failed to read config file");
		return -1;
	}


	int16_t ID = atoi(buf.get());

	configFile.close();

	return ID;
}

bool ConfigManager::saveIDToFlash(uint16_t id)
{
	File configFile = SPIFFS.open("/config.txt", "w");
	if (!configFile)
	{
		Serial.println("Failed to open config file for writing");
		return false;
	}

	String str(id);

	for (int i = 0; i < str.length(); i++)
	{
		configFile.write(str[i]);
	}

	configFile.close();

	return true;
}