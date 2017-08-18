// Fill out your copyright notice in the Description page of Project Settings.

#include "SerialBPWrapper.h"

void USerialBPWrapper::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UActorComponent::EndPlay(EndPlayReason);
	ClosePort();
}

bool USerialBPWrapper::OpenComPort(FString ComPortName, int baudrate)
{
	//if (_serial)
	//{
	//	ClosePort();
	//}

	try
	{
		std::string stdString(TCHAR_TO_UTF8(*ComPortName));
		uint32_t baud = baudrate;

		_serial = new serial::Serial(stdString, baud);

		bool open = _serial->isOpen();
		if (!open)
		{
			delete _serial;
		}
		return open;
	}

	catch (const std::exception e)
	{
		UE_LOG(LogTemp, Warning, TEXT("OpenComport -Expection: %s"), *FString(e.what()));
		ClosePort();
		return false;

	}
}

const FString lineBreak("\r\n");

bool USerialBPWrapper::ReadLine(FString & line)
{
	try
	{
		line = "";
		if (!_serial || _serial->available() < 1)
			return false;


		std::string stdstr = _serial->readline();

		line = UTF8_TO_TCHAR(stdstr.c_str());
		line.RemoveFromEnd(lineBreak);
		return true;
	}
	catch (std::exception& e)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReadLine - Expection: %s"), *FString(e.what()));
		ClosePort();
		return false;

	}
}

bool USerialBPWrapper::ClosePort()
{
	try
	{
		if (!_serial)
			return false;

		if (!_serial->isOpen())
		{
			delete _serial;
			_serial = nullptr;
			return false;
		}

		_serial->close();
		delete _serial;
		_serial = nullptr;
		FOnCloseDelegate.Broadcast();
		return true;
	}
	catch (const std::exception e)
	{
		UE_LOG(LogTemp, Warning, TEXT("Close Port - Expection: %s"), *FString(e.what()));
		return false;

	}
}
