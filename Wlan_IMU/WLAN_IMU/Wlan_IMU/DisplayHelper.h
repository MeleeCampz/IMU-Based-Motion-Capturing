#pragma once
#include "Adafruit_SSD1306.h"

#define OLED_RESET LED_BUILTIN

class DisplayHelper
{
private:
	Adafruit_SSD1306 display = Adafruit_SSD1306(OLED_RESET);

public:
	inline void BeginDisplay()
	{
		display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
		display.clearDisplay();
		display.setTextSize(1);
		display.setTextColor(WHITE);
	}

	inline void ClearAndDisplay(String toDisplay)
	{
		display.clearDisplay();
		display.setCursor(0, 0);
		display.println(toDisplay);
		display.display();
	}

	inline void ClearDisplay()
	{
		display.clearDisplay();
		display.setCursor(0, 0);

	}

	template <typename T>
	inline void print(T cont)
	{
		display.print(cont);
	}

	template <typename T>
	inline void println(T cont)
	{
		display.println(cont);
	}

	template <typename T>
	inline void printAndDisplay(T cont)
	{
		print(cont);
		display.display();
	}

	template <typename T>
	inline void printlnAndDisplay(T cont)
	{
		println(cont);
		display.display();
	}

	template <unsigned int S, typename ...T>
	void printf(const char(&fmt)[S], T&&... args) 
	{
		display.printf(fmt, std::forward<T>(args)...);
	}

	template <unsigned int S, typename ...T>
	void printfAndDisplay(const char(&fmt)[S], T&&... args)
	{
		printf(fmt, std::forward<T>(args)...);
		display.display();
	}
};