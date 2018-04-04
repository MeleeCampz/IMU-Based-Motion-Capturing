#include "OTAHelper.h"



OTAHelper::OTAHelper()
	:_inProgress(false)
{

}


OTAHelper::~OTAHelper()
{
}

void OTAHelper::StartOTA()
{
	//ArduinoOTA.setHostname("esp8266-OTA-40");
	Serial.println("Initializing OTA!");

	ArduinoOTA.onStart(std::bind(&OTAHelper::OnStart, this));
	ArduinoOTA.onEnd(std::bind(&OTAHelper::OnEnd, this));
	ArduinoOTA.onError(std::bind(&OTAHelper::OnError, this, std::placeholders::_1));
	//Binding two arguments didn't work out...
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});

	_inProgress = true;
	ArduinoOTA.begin();
	//Block rest of the programm!
	while (_inProgress)
	{
		ArduinoOTA.handle();
		delay(10);
	}
}

void OTAHelper::OnStart()
{
	Serial.println("OTA Started!");
}

void OTAHelper::OnEnd()
{
	Serial.println("OTA Ended!");

	_inProgress = false;
}

void OTAHelper::OnProgress(unsigned int progress, unsigned int total)
{
	Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
}

void OTAHelper::OnError(ota_error_t error)
{
	Serial.printf("Error[%u]: ", error);
	if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
	else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
	else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
	else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
	else if (error == OTA_END_ERROR) Serial.println("End Failed");
	_inProgress = false;
}
