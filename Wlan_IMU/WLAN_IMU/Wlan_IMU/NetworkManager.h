#pragma once
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>


namespace NetData
{
	struct IMUData
	{
		uint32_t timeStampt;
		float rotation[4];
		float velocity[3];
		int16_t ID; //Put ID as the last pos to avoid padding
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
	
	//Broadcast
	static const int UDP_PACKET_SIZE = 48;
	char _udpSendBuffer[UDP_PACKET_SIZE];
	static const uint16_t BROADCAST_PORT = 6678;
	static const uint16_t BROADCAST_DELAY_MS = 1000;

	//UDP pata stuff
	static const uint16_t DATA_PORT = 6676;
	static const int MAX_BYTES_PER_PACKAGE = sizeof(NetData::IMUData);
	char _DataSendBuffer[MAX_BYTES_PER_PACKAGE];
	int16_t _ID;
	int32_t _sampleRate;
	size_t _curBufferSize;

	bool _ledToggle = false;
	
	uint16_t _updateTimer = 0;
	uint32_t _lastUpdatetime = 0;

	IPAddress _localIP;
	IPAddress _remoteIP;

	std::function<void()> _magCallback;
	std::function<void(int32_t samplRate)> _sampleRateCallback;

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
	void SetCallbackOnNewSampleRate(std::function<void(int32_t rate)> fcn);

	//Add data to buffer (and add id); send if buffer is full
	bool WriteData(NetData::IMUData &data);
	//Forces to send data, even if buffer isn't full yet
	bool Flush();
};

