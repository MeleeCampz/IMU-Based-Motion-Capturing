/*
 Name:		Wlan_IMU.ino
 Created:	17-Jul-17 7:43:59 PM
 Author:	Tobias
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

MDNSResponder mdns;

ESP8266WebServer server(80);

WiFiEventHandler eConnected, eGotIP;

void handleRoot() {
	String Message =
		"<html>\
			<head>\
				<title>ESP8266 Demo</title>\
				<style>\
					body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
				</style>\
			</head>\
			<body>\
				<h1>Hello from ESP8266!</h1>\
				<form action=\"/login\" method=\POST\>\
				<input type =\"text\" name=\"SSID\" placeholder=\"SSID\"></br>\
				<input type =\"password\" name=\"password\" placeholder=\"Password\"></br>\
				<input type=\"submit\" value=\"Login\"></form>\
			</body>\
		</html>";
	server.send(200, "text/html", Message);
}

void handleLogin()
{
	if (!server.hasArg("SSID") || !server.hasArg("password")
		|| server.arg("SSID") == NULL || server.arg("password") == NULL) { // If the POST request doesn't have username and password data
		server.send(400, "text/plain", "400: Invalid Request");         // The request is invalid, so send HTTP status 400
		return;
	}
	

	handleRoot();

	String ssid = server.arg("SSID");
	String pass = server.arg("password");

	tryConnectToNetwork(ssid.c_str(), pass.c_str());
}

void handleNotFound() {
	server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

bool LEDTOOGLE = false;

void tryConnectToNetwork(const char* SSID, const char* PW)
{	
	delay(100);
	
	Serial.println();
	Serial.println();
	Serial.print("Connecting to network: ");
	Serial.println(SSID);
	Serial.println(PW);

	WiFi.disconnect();
	WiFi.begin(SSID, PW);

	int count = 0;
	while (WiFi.status() != WL_CONNECTED && count++ < 20)
	{
		delay(500);
		Serial.print(".");
		digitalWrite(BUILTIN_LED, LEDTOOGLE = !LEDTOOGLE == false ? LOW : HIGH);
	}
	Serial.println("");

	if (WiFi.status() != WL_CONNECTED)
	{
		Serial.println("Failed to connect to WifiNetwork!");
	}

	digitalWrite(BUILTIN_LED, HIGH);
}

void OnStationModeConnected(const WiFiEventStationModeConnected& event)
{
	Serial.print("WiFi connected: ");
	Serial.println(event.ssid);
	
}

void OnStationGoIP(const WiFiEventStationModeGotIP& event)
{
	Serial.print("IP-Adress: ");
	Serial.println(WiFi.localIP());
	trySetupDNS();
}


void trySetupDNS()
{
	if (mdns.begin("esp8266"))  // Start the mDNS responder for esp8266.local
	{
		Serial.println("mDNS responder started");
	}
	else
	{
		Serial.println("Error setting up MDNS responder!");
	}
}

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	Serial.begin(115200);

	WiFi.mode(WIFI_AP_STA);

	eConnected = WiFi.onStationModeConnected(OnStationModeConnected);
	eGotIP = WiFi.onStationModeGotIP(OnStationGoIP);
	
	delay(10);

	if (WiFi.isConnected())
	{
		OnStationModeConnected(WiFiEventStationModeConnected{});
		OnStationGoIP(WiFiEventStationModeGotIP{});
	}

	//WiFi.disconnect(); //Without this line, controller would autoconnect to last network
	bool result = WiFi.softAP("AccesPopint");

	if (result == true)
	{
		Serial.println("AP-Setup: Success");
	}
	else
	{
		Serial.println("AP-Setup: Fail");
	}

	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);

	

	server.on("/", HTTP_GET, handleRoot);
	server.on("/login", HTTP_POST, handleLogin);
	server.onNotFound(handleNotFound);
	server.begin();

	Serial.println("HTTP Server started!");

}


void loop() {
	server.handleClient();

	delay(10);
}