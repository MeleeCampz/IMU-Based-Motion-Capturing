/*
 Name:		Wlan_IMU.ino
 Created:	17-Jul-17 7:43:59 PM
 Author:	Tobias
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

//WLAN-Daten
char ssid[] = "Seppenpoint-WG";
char pass[] = "F3F8671F29";

IPAddress gateway(10, 0, 0, 138); //Gatway
IPAddress subnet(255, 255, 255, 0); //Subnet mask

ESP8266WebServer server(80);

void handleRoot() {
	String message = "*** My fucking server baby! ***\n";
	server.send(200, "text/plain", message);
}

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	
	Serial.begin(115200);
	delay(10);

	Serial.println();
	Serial.println();
	Serial.print("Connecting to network: ");
	Serial.println(ssid);

	WiFi.begin(ssid, pass);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected!");
	Serial.print("IP-Adress: ");
	Serial.println(WiFi.localIP());

	//WiFi.disconnect(); //Without this line, controller would autoconnect to last network
	bool result = WiFi.softAP("AccesPopint");

	if (result == true)
	{
		Serial.println("Ready");
	}
	else
	{
		Serial.println("Failed!");
	}
	
	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);


	server.on("/", handleRoot);
	server.begin();

	Serial.println("HTTP Server started!");
}

bool toggle = false;

void loop() {
	server.handleClient();
	
	toggle = !toggle;
	if (toggle)
	{
		digitalWrite(LED_BUILTIN, HIGH);
	}
	else
	{
		digitalWrite(LED_BUILTIN, LOW);
	}

	Serial.printf("Stations connected = %d\n", WiFi.softAPgetStationNum());
	delay(1000);
}