#pragma once
#include "Arduino.h"
#include "FS.h"

class ConfigManager
{
public:
	ConfigManager();
	~ConfigManager();

	static bool saveIDToFlash(uint16_t id);
	static int16_t loadIDFromFlash();
};

