#include "NetworkManager.h"



NetworkManager::NetworkManager()
	: _webServer(80), _curState(WAITING_FOR_CREDENTIALS), _curBufferSize(0)
{
}


NetworkManager::~NetworkManager()
{}

void NetworkManager::Begin()
{
	//Setup udp
	_Udp.begin(BROADCAST_PORT);
	String message = "ESP8266 Broadcast from " + WiFi.macAddress();
	strcpy(_udpSendBuffer, message.c_str());

	_Tcp.setNoDelay(true);
	
	//Try to reconnect
	TryConnectToNetwork("", "");
	
	if (WiFi.status() == WL_CONNECTED && !WiFi.SSID().startsWith("ESP8266"))
	{
		_curState = CONNECTED_TO_WIFI;
		return;
	}

	Serial.println("Failed to reconnect, starting WebConfig!");
	_curState = WAITING_FOR_CREDENTIALS;
	BeginWebConfig();
}

void NetworkManager::Update()
{
	float delta = millis() - _lastUpdatetime;
	_lastUpdatetime = millis();
	_updateTimer += delta;
	
	switch (_curState)
	{
	case NetworkManager::WAITING_FOR_CREDENTIALS:
		_webServer.handleClient();
		break;
	case NetworkManager::CONNECTED_TO_WIFI:
		if (_updateTimer >= BROADCAST_DELAY_MS)
		{
			SendUDPBroadcast();
			CheckUDPResponse();
			_updateTimer = 0;
		}
		break;
	case NetworkManager::CONNECTED_TO_HOST:
		if (!_Tcp.connected())
		{
			_Tcp.connect(_remoteIP, TCP_PORT);
		}
	default:
		break;
	}
}

void NetworkManager::BeginWebConfig()
{
	WiFi.mode(WiFiMode::WIFI_AP);
	
	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);

	_webServer.on("/", HTTP_GET, std::bind(&NetworkManager::handleRoot, this));
	_webServer.on("/login", HTTP_POST, std::bind(&NetworkManager::handleLogin, this));
	_webServer.onNotFound(std::bind(&NetworkManager::handleNotFound, this));
	_webServer.begin();

	String name = "ESP8266_" + WiFi.macAddress();
	WiFi.softAP(name.c_str());

}

void NetworkManager::handleRoot()
{
	String Message =
		"<html>\
				<head>\
					<title>ESP8266 WiFi connector</title>\
					<style>\
						body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
					</style>\
				</head>\
				<body>\
				<form action=\"/login\" method=\POST\>\
				<input type =\"text\" name=\"SSID\" placeholder=\"SSID\"></br>\
				<input type =\"text\" name=\"password\" placeholder=\"Password\"></br>\
				<input type=\"submit\" value=\"Login\"></form>\
			</body>\
			</html>";
	_webServer.send(200, "text/html", Message);
}

void NetworkManager::handleLogin()
{
	Serial.println("Handling login request!");
	
	if (!_webServer.hasArg("SSID") || !_webServer.hasArg("password")
		|| _webServer.arg("SSID") == NULL || _webServer.arg("password") == NULL) { // If the POST request doesn't have username and password data
		_webServer.send(400, "text/plain", "400: Invalid Request");         // The request is invalid, so send HTTP status 400
		return;
	}

	handleRoot();

	String ssid = _webServer.arg("SSID");
	String pass = _webServer.arg("password");

	Serial.println("Propagating Login-Data to other ESPs");
	int n = WiFi.scanNetworks();	
	for (int i = 0; i < n; ++i)
	{
		String SSID = WiFi.SSID(i);
		if (SSID.startsWith("ESP8266"))
		{
			Serial.print("Connecting to: ");
			Serial.println(SSID);
			WiFi.begin(SSID.c_str());
			int retries = 0;
			while ((WiFi.status() != WL_CONNECTED) && (retries < 15)) 
			{
				retries++;
				delay(1000);
				Serial.print(".");
			}
			Serial.println("");
			if (WiFi.status() == WL_CONNECTED)
			{
				Serial.println("Success!");
				HTTPClient http;
				http.begin("http://192.168.4.1:80/login");
				http.addHeader("Content-Type", "text/plain");
				String postReq = "SSID=" + ssid + "&password=" + pass;
				http.POST(postReq);
				http.writeToStream(&Serial);
				http.end();
			}
			else
			{
				Serial.println("Failed!");
			}
		}
	}
	
	Serial.println(ssid);
	TryConnectToNetwork(ssid.c_str(), pass.c_str());

	if (WiFi.status() == WL_CONNECTED && !WiFi.SSID().startsWith("ESP8266"))
	{
		_curState = CONNECTED_TO_WIFI;
	}
}

void NetworkManager::handleNotFound()
{
	_webServer.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

bool NetworkManager::WriteData(const NetData::IMUData &data)
{
	if (!_Tcp.connected())
	{
		return false;
	}
	
	if (sizeof(data) + _curBufferSize > MAX_BYTES_PER_PACKAGE * sizeof(char))
	{
		Flush();
	}
	memcpy(_TCPSendBuffer + _curBufferSize, &data, sizeof(data));
	_curBufferSize += sizeof(data);
	Flush();

	return true;
}

bool NetworkManager::Flush()
{
	if (_curBufferSize > 0)
	{
		_Tcp.write(&_TCPSendBuffer[0], _curBufferSize);
		_Tcp.flush();
		_curBufferSize = 0;
		return true;
	}

	return false;
}

void NetworkManager::TryConnectToNetwork(const char* ssid, const char* pw)
{
	delay(100);

	Serial.println();

	WiFi.mode(WIFI_STA);
	if (ssid == "")
	{
		Serial.println("Trying to reconnect to last network!");
		WiFi.reconnect();
	}
	else
	{
		Serial.print("Connecting to network: ");
		Serial.println(ssid);
		WiFi.begin(ssid, pw);
	}

	int count = 0;
	while (WiFi.status() != WL_CONNECTED && count++ < 40)
	{
		delay(500);
		digitalWrite(BUILTIN_LED, _ledToggle = !_ledToggle == false ? LOW : HIGH);
	}
	
	if (WiFi.status() == WL_CONNECTED)
	{
		Serial.println("");
		Serial.println("Connected!");
		Serial.print("IP-Adress: ");
		IPAddress adr = WiFi.localIP();
		Serial.println(adr);
		_broadcastAdress = IPAddress(adr[0], adr[1], adr[2], 255);
	}
	else
	{
		Serial.println("Failed to connect");
	}

	digitalWrite(BUILTIN_LED, HIGH);
}

void NetworkManager::SendUDPBroadcast()
{
	_Udp.beginPacket(_broadcastAdress, BROADCAST_PORT);
	_Udp.write(_udpSendBuffer, sizeof(_udpSendBuffer));
	_Udp.endPacket();
	Serial.println("Broadcast packet sent!");
}

void NetworkManager::CheckUDPResponse()
{
	
	while (_Udp.parsePacket())
	{
		Serial.println(".");
		String response = _Udp.readString();
		if (response.equals(_udpSendBuffer))
		{
			_curState = NetworkManager::CONNECTED_TO_HOST;
			_remoteIP = _Udp.remoteIP();
			Serial.println("Received broadcast package!");
		}
	}
}
