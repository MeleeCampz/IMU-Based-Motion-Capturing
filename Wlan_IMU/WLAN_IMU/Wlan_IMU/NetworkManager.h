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
		CONNECTED_TO_WIFI,
		CONNECTED_TO_HOST
	};

	NetworkState _curState;
	
	static const int UDP_PACKET_SIZE = 48;
	char _udpSendBuffer[UDP_PACKET_SIZE];
	static const uint16_t BROADCAST_PORT = 6678;
	static const uint16_t BROADCAST_DELAY_MS = 1000;

	//TCP
	//WiFiClient _Tcp;
	static const uint16_t DATA_PORT = 6676;
	static const int MAX_BYTES_PER_PACKAGE = 48;
	char _DataSendBuffer[MAX_BYTES_PER_PACKAGE];
	size_t _curBufferSize;

	bool _ledToggle = false;
	
	uint16_t _updateTimer = 0;
	uint32_t _lastUpdatetime = 0;

	IPAddress _localIP;
	IPAddress _remoteIP;

	std::function<void()> _magCallback;

	//UDP
	WiFiUDP _Udp;
	IPAddress _broadcastAdress;

	//Server
	ESP8266WebServer _webServer;

	void InitSendBuffer();
	void CheckUDPResponse();
	void SendUDPBroadcast();
	//If ssid is not specified (empty string) mcu tries to connect to last network
	void TryConnectToNetwork(const char* ssid, const char* pw);
	///Webserver stuff
	void BeginWebConfig();
	void handleRoot();
	void handleLogin();
	void handleNotFound();
public:	
	NetworkManager();
	~NetworkManager();

	void Begin();
	void Update();

	void SetCallbackOnMagCalibration(std::function<void()> fcn);

	//Add data to buffer; send if buffer is full
	bool WriteData(const NetData::IMUData &data);
	//Forces to send data, even if buffer isn't full yet
	bool Flush();
};

