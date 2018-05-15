#pragma once
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

class OTAHelper
{
public:
	static void StartOTA(String fwURL)
	{
		Serial.println("Preparing to update.");

		t_httpUpdate_return ret = ESPhttpUpdate.update(fwURL);
		switch (ret)
		{
		case HTTP_UPDATE_FAILED:
			Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s \l", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
			break;

		case HTTP_UPDATE_NO_UPDATES:
			Serial.println("HTTP_UPDATE_NO_UPDATES");
			break;

		case HTTP_UPDATE_OK:
			Serial.println("UPDATE SUCCESS!");
			break;
		}
	}
};

