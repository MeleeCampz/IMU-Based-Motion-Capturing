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
	static const int32 TCP_PORT = 6676;
	
	FTimerHandle _timeHandleTCPConnection;
	FTimerHandle _timeHandleTCPSocket;
	FTimerHandle _timeHandleUDPSocket;

	FSocket* _clients[MAX_CONNECTIONS];
	
	FSocket* _UDPSocket;
	uint8 _UDPReceiveBuffer[256];
	int32 _UDPReceiveBufferSize;
	static const int32 UDP_PORT = 6678;

public:	
	// Sets default values for this component's properties
	UIMUReceiver();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Timer functions, could be threads
	void TCPConnectionListener(); 	//can thread this eventually
	void TCPSocketListener();		//can thread this eventually
	void UDPSocketListener();		//can thread this eventually

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator DebugRotation;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	template <typename T>
	inline T swap_endian(T u)
	{
		static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

		union
		{
			T u;
			unsigned char u8[sizeof(T)];
		} source, dest;

		source.u = u;

		for (size_t k = 0; k < sizeof(T); k++)
			dest.u8[k] = source.u8[sizeof(T) - k - 1];

		return dest.u;
	};

	float unpackFloat(const uint8_t *buffer);
};
