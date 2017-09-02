#pragma once
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>


namespace NetData
{
	struct IMUData
	{
		float rotation[3];
		float acceleration[3];
	};

}

class NetworkManager
{
private:
	enum NetworkState
	{
		WAITING_FOR_CREDENTIALS,
		CONNECTED
	};

	NetworkState _curState;
	
	static const int UDP_PACKET_SIZE = 48;
	char _udpBuffer[UDP_PACKET_SIZE];
	static const uint16_t BROADCAST_PORT = 6677;

	static const int MAX_BYTES_PER_PACKAGE = 16;
	char _TCPSendBuffer[MAX_BYTES_PER_PACKAGE];
	char* curBufferPos;

	bool _ledToggle = false;

	IPAddress _localIP;
	IPAddress _remoteIP;

	//UDP
	WiFiUDP _Udp;
	uint16_t _port;
	IPAddress _broadcastAdress;

	//Server
	ESP8266WebServer _webServer;

	bool _initialted;

	void CheckUDPResponse();
	void SendUDPBroadcast();
	void TryConnectToNetwork(const char* ssid, const char* pw);
public:
	NetworkManager();
	~NetworkManager();

	void Begin();
	void Update();

	///Webserver stuff
	void BeginWebConfig();
	void handleRoot();
	void handleLogin();
	void handleNotFound();


	//Add data to buffer; send if buffer is full
	bool WriteData(NetData::IMUData data);
	//Forces to send data, even if buffer isn't full yet
	bool Flush();
};

