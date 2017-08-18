// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "serial/serial.h"
#include "CoreMinimal.h"
#include "SerialBPWrapper.generated.h"

using serial::Serial;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCloseDelegate);

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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION(BlueprintCallable, Category = "Serial")
	bool OpenComPort(FString ComPortName, int baudrate);

	UFUNCTION(BlueprintCallable, Category = "Serial")
		bool ReadLine(FString & line);

	//Return false, if no port was open
	UFUNCTION(BlueprintCallable, Category = "Serial")
		bool ClosePort();


	UPROPERTY(BlueprintAssignable, Category = "Serial")
		FCloseDelegate FOnCloseDelegate;


};
