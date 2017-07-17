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

IPAddress ip(10, 0, 0, 66); //fixed ip adress
IPAddress gateway(10, 0, 0, 138); //Gatway
IPAddress subnet(255, 255, 255, 0); //Subnet mask

ESP8266WebServer server(80);

void handleRoot() {
	String message = "*** My fucking server baby! ***\n";
	server.send(200, "text/plain", message);
}

void setup() {
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

	server.on("/", handleRoot);
	server.begin();

	Serial.println("HTTP Server started!");
}

void loop() {
	server.handleClient();
	delay(500);
}