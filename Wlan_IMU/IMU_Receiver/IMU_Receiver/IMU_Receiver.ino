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
	Serial.begin(921600);
	delay(1000);
	Serial.println("Booting...");
	WiFi.disconnect();
	WiFi.mode(WIFI_AP);
	WiFi.softAP(ssid, password);
	server.begin();
	server.setNoDelay(true);
	Serial.println("Done!");
}

char* readbuffer = new char[12]; //3 flots = 12chars
const size_t bufferSize = sizeof(float[3]);
float r1, r2, r3;
IPAddress remoteAdress;

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
					serverClients[i].readBytes(readbuffer, bufferSize);
					
					remoteAdress = serverClients[i].remoteIP();
					Serial.print("+"); //||mark begin of data
					Serial.print(remoteAdress[3]); //Only show last part of adress to save bandwidth
					Serial.print(":"); //use ":" to mark start of rotation data
					memcpy(&r1, readbuffer, sizeof(float));
					Serial.print(r1);
					Serial.print(",");
					memcpy(&r2, readbuffer+4, sizeof(float));
					Serial.print(r2);
					Serial.print(",");
					memcpy(&r3, readbuffer+8, sizeof(float));
					Serial.print(r3);
					Serial.println("|"); //mark end of rotation data;
				}
				//you can reply to the client here
			}
		}
	}
}
