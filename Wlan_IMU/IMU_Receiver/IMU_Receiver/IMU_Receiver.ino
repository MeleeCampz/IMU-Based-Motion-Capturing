/*
 Name:		IMU_Receiver.ino
 Created:	17-Aug-17 11:33:10 PM
 Author:	Tobias
*/


#include <ESP8266WiFi.h>

#define MAX_SRV_CLIENTS 20
const char* ssid = "IMU_AP";
const char* password = "MultiPass";
const uint8_t port = 21;

WiFiServer server(port);
WiFiClient serverClients[MAX_SRV_CLIENTS];


// the setup function runs once when you press reset or power the board
void setup()
{
	Serial.begin(115200);
	delay(1000);
	Serial.println("Booting...");
	WiFi.disconnect();
	WiFi.mode(WIFI_AP);
	WiFi.softAP(ssid, password);
	server.begin();
	server.setNoDelay(true);
	Serial.println("Done!");
}

// the loop function runs over and over again until power down or reset
void loop()
{
	uint8_t i;
	if (server.hasClient())
	{
		for (i = 0; i < MAX_SRV_CLIENTS; i++)
		{
			if (!serverClients[i] || !serverClients[i].connected())
			{
				if (serverClients[i])
				{
					serverClients[i].stop();
				}
				serverClients[i] = server.available();
				continue;
			}
		}
		//no free spot
		WiFiClient serverClient = server.available();
		serverClient.stop();
	}
	for (i = 0; i < MAX_SRV_CLIENTS; i++) 
	{
		if (serverClients[i] && serverClients[i].connected())
		{
			if (serverClients[i].available()) 
			{
				while (serverClients[i].available())
				{
					Serial.write(serverClients[i].read());
				}
				server.write("\r\n");
				//you can reply to the client here
			}
		}
	}
}
