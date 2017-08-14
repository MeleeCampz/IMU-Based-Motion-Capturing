#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "MPU9250.h"
#include "QuaternionFilters.h"
#include "IMUResult.h"
#include "Adafruit_SSD1306.h"


//Pin Defines
//Using default pin D2 for SDA
//Using default pin D1 for SDA
const int m_intPin = D3;
#define OLED_RESET LED_BUILTIN


///////////////////////////////////////////////////////////////////
//Determines how often we sample data
///////////////////////////////////////////////////////////////////
#define samplingRateInMicros 333 * 1000

///////////////////////////////////////////////////////////////////
//Setup for the Accelerometer
///////////////////////////////////////////////////////////////////
#define declination 3.3f  //http://www.ngdc.noaa.gov/geomag-web/#declination . This is the declinarion in the easterly direction in degrees.  
#define calibrateMagnetometer false  //Setting requires requires you to move device in figure 8 pattern when prompted over serial port.  Typically, you do this once, then manually provide the calibration values moving forward.

MPU9250 mpu;
IMUResult magResult, accResult, gyroResult, orientResult;


//WLAN
ESP8266WebServer server(80);
WiFiEventHandler eConnected, eGotIP;

//DisplayTest
Adafruit_SSD1306 display(OLED_RESET);
const bool useDisplay = true;

bool LEDTOGGLE = false;
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
		digitalWrite(BUILTIN_LED, LEDTOGGLE = !LEDTOGGLE == false ? LOW : HIGH);
	}
	Serial.println("");

	if (WiFi.status() != WL_CONNECTED)
	{
		Serial.println("Failed to connect to WifiNetwork!");
	}

	digitalWrite(BUILTIN_LED, HIGH);
}

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

void OnStationModeConnected(const WiFiEventStationModeConnected& event)
{
	Serial.print("WiFi connected: ");
	Serial.println(event.ssid);

}

void OnStationGoIP(const WiFiEventStationModeGotIP& event)
{
	Serial.print("IP-Adress: ");
	Serial.println(WiFi.localIP());
}



void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	Serial.begin(115200);

	//LCD
	if (useDisplay)
	{
		display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
		display.clearDisplay();
		display.setTextSize(1);
		display.setTextColor(WHITE);
	}

	//Init MPU
	mpu.begin(SDA, SCL, m_intPin);
	//This tests communication between the accelerometer and the ESP8266.  Dont continue until we get a successful reading.
	byte c = 0x00;
	while (c != 0x73)
	{
		c = mpu.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
		if (c != 0x73)
		{
			delay(500);
		}
	}

	// Calibrate gyro and accelerometers, load biases in bias registers, then initialize MPU.
	mpu.selfTest();
	mpu.calibrate();
	mpu.init();
	if (calibrateMagnetometer)
	{
		mpu.magCalibrate();
	}
	else
	{
		mpu.setMagCalibrationManually(-157, 451, 51);    //Set manually with the results of magCalibrate() if you don't want to calibrate at each device bootup.														   
	}


	if (useDisplay)
	{
		display.clearDisplay();
		display.setCursor(0, 0);
		display.println("MPU9250 calibrated!");
		display.display();
	}

	//Init WLAN
	WiFi.disconnect();
	WiFi.mode(WIFI_AP_STA);
	eConnected = WiFi.onStationModeConnected(OnStationModeConnected);
	eGotIP = WiFi.onStationModeGotIP(OnStationGoIP);


	bool result = WiFi.softAP("ESP8266-AP");

	if (result == true)
	{
		if (useDisplay)
			display.println("AP-Setup: Success");
	}
	else
	{
		if (useDisplay)
			display.println("AP-Setup: Fail");
	}
	if (useDisplay)
		display.display();

	IPAddress myIP = WiFi.softAPIP();
	
	if (useDisplay)
	{
		display.print("Address: ");
		display.println(myIP);
		display.display();
	}


	server.on("/", HTTP_GET, handleRoot);
	server.on("/login", HTTP_POST, handleLogin);
	server.onNotFound(handleNotFound);
	server.begin();

	if (useDisplay)
	{
		display.println("HTTP Server started!");
		display.display();
	}
}


uint32_t lastSample = 0;
uint32_t lastDataCount = 0;

#define TEST_SAMPLRATE false


void loop() {
	server.handleClient();

	// If intPin goes high, all data registers have new data
	// On interrupt, check if data ready interrupt
	if (mpu.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
	{
		mpu.readAccelData(&accResult);
		mpu.readGyroData(&gyroResult);
		mpu.readMagData(&magResult);

		if (TEST_SAMPLRATE)
			lastDataCount++;
	}

	// Must be called before updating quaternions!
	mpu.updateTime();
	MahonyQuaternionUpdate(&accResult, &gyroResult, &magResult, mpu.deltat);
	//MadgwickQuaternionUpdate(&accResult, &gyroResult, &magResult, mpu.deltat);

	if (micros() - lastSample > samplingRateInMicros)
	{
		readOrientation(&orientResult, declination);
		//orientResult.printResult();


		if (TEST_SAMPLRATE)
		{
			//Serial.print("Sampling rate in Hz:");
			//Serial.print(1000000.0f / ((micros() - lastSample) / lastDataCount)); //Print out herz
			//Serial.print(" @");
			//Serial.print(lastDataCount);
			//Serial.println(" Samples");
			if (useDisplay)
			{
				display.clearDisplay();
				display.setCursor(0, 0);
				display.println("Sampling rate in HZ:");
				display.println(1000000.0f / ((micros() - lastSample) / lastDataCount));
				display.print(" @");
				display.print(lastDataCount);
				display.println(" Samples");
				display.display();
			}
		}
		else
		{
			if (useDisplay)
			{
				display.clearDisplay();
				display.setCursor(0, 0);
				display.print("X: ");
				display.println(orientResult.getXComponent());
				display.print("Y: ");
				display.println(orientResult.getYComponent());
				display.print("Z: ");
				display.println(orientResult.getZComponent());
				display.display();
			}
		}
		lastSample = micros();

		if (TEST_SAMPLRATE)
			lastDataCount = 0;

		mpu.sumCount = 0;
		mpu.sum = 0;
	}
}