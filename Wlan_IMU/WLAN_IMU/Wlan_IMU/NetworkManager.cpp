#include "NetworkManager.h"



NetworkManager::NetworkManager()
	: _initialted(false), _webServer(80), _curState(CONNECTED)
{
}


NetworkManager::~NetworkManager()
{}

void NetworkManager::Begin()
{
	Serial.println("Trying to reconnect to last network");

	//WiFi.reconnect();

	//int retries = 0;
	//while (WiFi.status() != WL_CONNECTED && retries++ < 16)
	//{
	//	Serial.print(".");
	//	delay(1000);
	//}
	//Serial.println("");
	//
	//if (WiFi.status() == WL_CONNECTED)
	//{
	//	Serial.print("Reconnected to: ");
	//	Serial.println(WiFi.SSID());
	//	_curState = CONNECTED;

	//	return;
	//}

	Serial.println("Failed to reconnect, starting WebConfig!");
	_curState = WAITING_FOR_CREDENTIALS;
	BeginWebConfig();
}

void NetworkManager::Update()
{
	switch (_curState)
	{
	case NetworkManager::WAITING_FOR_CREDENTIALS:
		_webServer.handleClient();
		break;
	case NetworkManager::CONNECTED:

		break;
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
	TryConnectToNetwork(ssid.c_str(), pass.c_str());

	//if this code gets executes connection was a success!

	int n = WiFi.scanNetworks();
	Serial.print(n);
	Serial.println(" WiFi networks found, trying to connect to other ESP-Networks");
	
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

	Serial.print("Finished iterating all ESP-networks, connecting back to: ");
	Serial.println(ssid);
	TryConnectToNetwork(ssid.c_str(), pass.c_str());	
}

void NetworkManager::handleNotFound()
{
	_webServer.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

bool NetworkManager::WriteData(NetData::IMUData data)
{
	return false;
}

bool NetworkManager::Flush()
{
	return false;
}

void NetworkManager::TryConnectToNetwork(const char* ssid, const char* pw)
{
	delay(100);

	Serial.println();
	Serial.println();
	Serial.print("Connecting to network: ");
	Serial.println(ssid);

	WiFi.disconnect();
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, pw);

	int count = 0;
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
		digitalWrite(BUILTIN_LED, _ledToggle = !_ledToggle == false ? LOW : HIGH);
	}
	Serial.println("");
	Serial.println("Connected!");
	Serial.print("IP-Adress: ");
	IPAddress adr = WiFi.localIP();
	Serial.println(adr);

	_broadcastAdress = IPAddress(adr[0], adr[1], adr[2], 255);

	digitalWrite(BUILTIN_LED, HIGH);
}

void NetworkManager::SendUDPBroadcast()
{
	strcpy(_udpBuffer, "ESP8266 Broadcast!");
	_Udp.beginPacket(_broadcastAdress, BROADCAST_PORT);
	Serial.println(_Udp.write(_udpBuffer, sizeof(_udpBuffer)));
	_Udp.endPacket();
	Serial.print("Broadcast packet sent!");
}

void NetworkManager::CheckUDPResponse()
{
	int packetSize = _Udp.parsePacket();
	if (packetSize)
	{
		Serial.print("Received packet of size ");
		Serial.println(packetSize);
		Serial.print("From ");
		_remoteIP = _Udp.remoteIP();
		Serial.print(_remoteIP);
		Serial.print(", port ");
		Serial.println(_Udp.remotePort());
	}
}
