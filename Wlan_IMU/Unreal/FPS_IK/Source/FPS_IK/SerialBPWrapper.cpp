// Fill out your copyright notice in the Description page of Project Settings.

#include "SerialBPWrapper.h"

bool USerialBPWrapper::OpenComPort(FString ComPortName, int baudrate)
{
	if (_serial)
	{
		delete _serial;
	}


	std::string stdString(TCHAR_TO_UTF8(*ComPortName));
	uint32_t baud = baudrate;
	
	_serial = new serial::Serial(stdString, baud);

	return _serial->isOpen();
}

bool USerialBPWrapper::ReadLine(FString & line)
{
	line = "";
	if (!_serial)
		return false;

	if (!_serial->available())
		return false;
	
	line = UTF8_TO_TCHAR(_serial->readline().c_str());
	return true;
}
