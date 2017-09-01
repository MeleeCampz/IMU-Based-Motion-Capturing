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
	
	FSocket* _TCPReceiveSocket;
	uint8 _TCPreveiceBuffer[256];
	int32 _TCPreceiveBufferSize;
	
	FTimerHandle _timeHandleTCPConnection;
	FTimerHandle _timeHandleTCPSocket;
	FTimerHandle _timeHandleUDPSocket;

	FSocket* _clients[MAX_CONNECTIONS];
	
	FSocket* _UDPSocket;
	uint8 _UDPReceiveBuffer[256];
	int32 _UDPReceiveBufferSize;
	uint8 _UDPSendBuffer[256];
	int32 _UDPSendBufferSize;

public:	
	// Sets default values for this component's properties
	UIMUReceiver();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Timer functions, could be threads
	void TCPConnectionListener(); 	//can thread this eventually
	void TCPSocketListener();		//can thread this eventually
	void UDPSocketListener();		//can thread this eventually

protected:
	// Called when the game starts
	virtual void BeginPlay() override;	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
