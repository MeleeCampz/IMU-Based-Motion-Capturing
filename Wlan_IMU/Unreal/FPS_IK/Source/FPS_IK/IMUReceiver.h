// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UnrealNetwork.h"
#include "Sockets.h"
#include "CoreMinimal.h"
#include "TimerManager.h"
#include "Components/ActorComponent.h"
#include "IMUReceiver.generated.h"

const int32 MAX_CONNECTIONS = 24;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPS_IK_API UIMUReceiver : public UActorComponent
{
	GENERATED_BODY()
private:
	
	FSocket* _socket;
	uint8 _reveiceBuffer[256];
	int32 _bufferSize;
	FTimerHandle _timeHandleConnection;
	FTimerHandle _timeHandleSocket;

	FSocket* _clients[MAX_CONNECTIONS];

public:	
	// Sets default values for this component's properties
	UIMUReceiver();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Timer functions, could be threads
	void TCPConnectionListener(); 	//can thread this eventually
	void TCPSocketListener();		//can thread this eventually

protected:
	// Called when the game starts
	virtual void BeginPlay() override;	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
