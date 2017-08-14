// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "serial/serial.h"
#include "CoreMinimal.h"
#include "SerialBPWrapper.generated.h"

using serial::Serial;

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class FPS_IK_API USerialBPWrapper : public UActorComponent
{
	GENERATED_BODY()

private:
	Serial* _serial;

public:
	UFUNCTION(BlueprintCallable, Category = "Serial")
	bool OpenComPort(FString ComPortName, int baudrate);

	UFUNCTION(BlueprintCallable, Category = "Serial")
		bool ReadLine(FString & line);
};
