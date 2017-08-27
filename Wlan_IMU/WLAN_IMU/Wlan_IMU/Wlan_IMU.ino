#include <ESP8266WiFi.h>
#include "MPU9250.h"
#include "QuaternionFilters.h"
#include "IMUResult.h"
#include "Adafruit_SSD1306.h"
#include <WiFiUdp.h>

//Pin Defines
//Using default pin D2 for SDA
//Using default pin D1 for SDA
const int m_intPin = D3;
#define OLED_RESET LED_BUILTIN


///////////////////////////////////////////////////////////////////
//Determines how often we sample data
///////////////////////////////////////////////////////////////////
#define samplingRateInMicros 1 //5 * 1000

///////////////////////////////////////////////////////////////////
//Setup for the Accelerometer
///////////////////////////////////////////////////////////////////
#define declination 3.3f  //http://www.ngdc.noaa.gov/geomag-web/#declination . This is the declinarion in the easterly direction in degrees.  
#define calibrateMagnetometer false  //Setting requires requires you to move device in figure 8 pattern when prompted over serial port.  Typically, you do this once, then manually provide the calibration values moving forward.

MPU9250 mpu;
IMUResult magResult, accResult, gyroResult, orientResult;


//WLAN-AP to connect to
const char* ssid = "YouShallNotPass";
const char* password = "MultiPass";
const uint16_t serverPort = 6676;
const IPAddress serverAdress(192,168,0,139);
WiFiClient IMU_AP;

//DisplayTest
Adafruit_SSD1306 display(OLED_RESET);
const bool useDisplay = false;

bool LEDTOGGLE = false;
void tryConnectToNetwork()
{
	delay(100);

	Serial.println();
	Serial.println();
	Serial.print("Connecting to network: ");
	Serial.println(ssid);

	WiFi.disconnect();
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);


	int count = 0;
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
		digitalWrite(BUILTIN_LED, LEDTOGGLE = !LEDTOGGLE == false ? LOW : HIGH);
	}
	Serial.println("");

	Serial.println("Connected!");
	Serial.print("IP-Adress: ");
	Serial.println(WiFi.localIP());

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
}


void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	Serial.begin(921600);

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
			Serial.println("Waiting for MPU-9250");
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
		mpu.setMagCalibrationManually(-192, 403, 93);    //Set manually with the results of magCalibrate() if you don't want to calibrate at each device bootup.														   
		//mpu.setMagCalibrationManually(0, 0, 0);    //Set manually with the results of magCalibrate() if you don't want to calibrate at each device bootup.														   
	}


	if (useDisplay)
	{
		display.clearDisplay();
		display.setCursor(0, 0);
		display.println("MPU9250 calibrated!");
		display.display();
	}

	WiFi.disconnect();
	//IMU_AP.setNoDelay(true);
}


uint32_t lastSample = 0;
uint32_t lastDataCount = 0;

#define TEST_SAMPLRATE false

char* sendBuffer = new char[12]; //3floats
float r1, r2, r3;

void loop()
{
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
		orientResult.printResult();

		if (TEST_SAMPLRATE)
		{
			if (useDisplay)
			{
				display.clearDisplay();
				display.setCursor(0, 0);
				display.println("Sampling rate in HZ:");
				display.println(1000000.0f / (samplingRateInMicros / lastDataCount));
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
				String debugStr = "";
				debugStr += "X: ";
				debugStr += orientResult.getXComponent();
				debugStr += "Y: ";
				debugStr += orientResult.getYComponent();
				debugStr += "Z: ";
				debugStr += orientResult.getZComponent();


				display.clearDisplay();
				display.setCursor(0, 0);
				display.println(debugStr);
				display.display();
			}

			if (!IMU_AP.connected() || !WiFi.isConnected())
			{
				if (!WiFi.isConnected())
				{
					tryConnectToNetwork();
				}
				digitalWrite(BUILTIN_LED, HIGH);
				
				while (!IMU_AP.connect(serverAdress, serverPort))
				{
					Serial.print(".");
					digitalWrite(BUILTIN_LED, LEDTOGGLE = !LEDTOGGLE == false ? LOW : HIGH);	
					delay(100);
				}
				
				digitalWrite(BUILTIN_LED, LOW);
			}

			r1 = orientResult.getXComponent();
			r2 = orientResult.getYComponent();
			r3 = orientResult.getZComponent();

			memcpy(sendBuffer,		&r1,	sizeof(float));
			memcpy(sendBuffer +4,	&r2,	sizeof(float));
			memcpy(sendBuffer +8,	&r3,	sizeof(float));

			IMU_AP.write((const char*)sendBuffer, 3 * sizeof(float));
			IMU_AP.flush();
		}

		if (TEST_SAMPLRATE)
			lastDataCount = 0;

		lastSample = micros();
		mpu.sumCount = 0;
		mpu.sum = 0;
	}
}