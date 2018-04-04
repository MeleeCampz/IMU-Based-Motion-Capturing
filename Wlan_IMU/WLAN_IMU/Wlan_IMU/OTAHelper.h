#pragma once
#include <ArduinoOTA.h>

class OTAHelper
{
public:
	OTAHelper();
	~OTAHelper();

	void StartOTA();

private:
	bool _inProgress;
	
	void OnStart();
	void OnEnd();
	void OnProgress(unsigned int progress, unsigned int total);
	void OnError(ota_error_t error);
};

