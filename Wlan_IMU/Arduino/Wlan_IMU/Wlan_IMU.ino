#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "MPU9250.h"
#include "QuaternionFilters.h"
#include "IMUResult.h"


///////////////////////////////////////////////////////////////////
//Determines how often we sample data
///////////////////////////////////////////////////////////////////
#define samplingRateInMillis 33

///////////////////////////////////////////////////////////////////
//Setup for the Accelerometer
///////////////////////////////////////////////////////////////////
#define declination 13.37  //http://www.ngdc.noaa.gov/geomag-web/#declination . This is the declinarion in the easterly direction in degrees.  
#define calibrateMagnetometer false  //Setting requires requires you to move device in figure 8 pattern when prompted over serial port.  Typically, you do this once, then manually provide the calibration values moving forward.

MPU9250 mpu;
IMUResult magResult, accResult, gyroResult, orientResult;


//WLAN
MDNSResponder mdns;
ESP8266WebServer server(80);
WiFiEventHandler eConnected, eGotIP;

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

void OnStationGoIP(const WiFiEventStationModeGotIP& event)
{
	Serial.print("IP-Adress: ");
	Serial.println(WiFi.localIP());
	trySetupDNS();
}

const int m_SDA = D7;
const int m_SCL = D6;
const int m_intPin = D2;



void setup() {

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	Serial.begin(115200);


	//Init MPU
	mpu.begin(m_SDA, m_SCL, m_intPin);
	//This tests communication between the accelerometer and the ESP8266.  Dont continue until we get a successful reading.
	//It is expected that the WHO_AM_I_MPU9250 register should return a value of 0x71.
	//If it fails to do so try the following:
	//1) Turn power off to the ESP8266 and restart.  Try this a few times first.  It seems to resolve the issue most of the time.  If this fails, then proceed to the followingn steps.
	//2) Go to src/MPU9250.h and change the value of ADO from 0 to 1
	//3) Ensure your i2c lines are 3.3V and that you haven't mixed up SDA and SCL
	//4) Run an i2c scanner program (google it) and see what i2c address the MPU9250 is on.  Verify your value of ADO in src/MPU9250.h is correct.
	//5) Some models apparently expect a hex value of 0x73 and not 0x71.  If that is the case, either remove the below check or change the value fro 0x71 to 0x73.
	byte c;
	do
	{
		c = mpu.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
		if (c != 0x73)
		{
			Serial.println("Failed to communicate with MPU9250");
			Serial.print("WHO_AM_I returned ");
			Serial.println(c, HEX);
			delay(500);
		}
	} while (c != 0x73);

	Serial.println("Successfully communicated with MPU9250");

	// Calibrate gyro and accelerometers, load biases in bias registers, then initialize MPU.
	mpu.calibrate();
	mpu.init();
	if (calibrateMagnetometer)
	{
		mpu.magCalibrate();
	}
	else
	{
		mpu.setMagCalibrationManually(162, 130, 46);    //Set manually with the results of magCalibrate() if you don't want to calibrate at each device bootup.														   
	}


	Serial.println("Accelerometer ready");

	mpu.selfTest();

	WiFi.disconnect();


	//Init WLAN
	WiFi.mode(WIFI_AP_STA);
	eConnected = WiFi.onStationModeConnected(OnStationModeConnected);
	eGotIP = WiFi.onStationModeGotIP(OnStationGoIP);


	bool result = WiFi.softAP("ESP8266-AP");

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


uint32_t lastSample = 0;
uint32_t lastDataCount = 0;

#define TEST_SAMPLRATE  false

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
	else
	{
		Serial.println("to slow?");
	}

	// Must be called before updating quaternions!
	mpu.updateTime();
	MahonyQuaternionUpdate(&accResult, &gyroResult, &magResult, mpu.deltat);
	readOrientation(&orientResult, declination);

	if (millis() - lastSample > samplingRateInMillis)
	{
		//accResult.printResult();
		//gyroResult.printResult();
		//magResult.printResult();
		orientResult.printResult();

		if (TEST_SAMPLRATE)
			Serial.println((millis() - lastSample) / lastDataCount);
		lastSample = millis();

		if (TEST_SAMPLRATE)
			lastDataCount = 0;
		
		mpu.sumCount = 0;
		mpu.sum = 0;
	}
}