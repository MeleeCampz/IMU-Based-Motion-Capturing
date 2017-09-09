// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UnrealNetwork.h"
#include "Sockets.h"
#include "CoreMinimal.h"
#include "UdpSocketReceiver.h"
#include "Components/ActorComponent.h"
#include "IMUReceiver.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClientUpdate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPS_IK_API UIMUReceiver : public UActorComponent
{
	GENERATED_BODY()
private:
	struct IMUClient
	{
		FString adr;
		int32 ID;
		
		IMUClient(FString address, int32 id)
			: adr(address), ID(id)
		{}

		IMUClient()
			:adr("INVALID"), ID(-1)
		{}
	};


	FSocket* _DataReceiveSocket;
	FUdpSocketReceiver* _DataReceiver = nullptr;
	int32 _DataReceiveBufferSize = 24; //in bytes
	static const int32 DATA_PORT = 6676;
	
	FSocket* _BroadcastSocket;
	FUdpSocketReceiver* _BroadcastReceiver = nullptr;
	int32 _BroadcastReceiveBufferSize = 48; //in bytes
	static const int32 BROADCAST_PORT = 6678;

	TArray<IMUClient> _clients;

	TQueue<IMUClient> _clientsToAdd;

	TSharedRef<FInternetAddr> CreateAddr(FString addr, int32 port);

public:	
	// Sets default values for this component's properties
	UIMUReceiver();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void RecvBroadcast(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);
	void RecvData(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);

	UFUNCTION(BlueprintCallable)
		void SendCalibrateRequest(FString ipAddress);

	UFUNCTION(BlueprintCallable)
		void SendIDRequest(FString ipAddress, int32 ID);

	UFUNCTION(BlueprintCallable)
		int32 GetNumClients();

	UFUNCTION(BlueprintCallable)
		void GetClientInfo(TArray<FString>& names, TArray<int32>& ids);

	UPROPERTY(BlueprintAssignable)
		FOnClientUpdate OnClientUpdate;

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
	int16_t unpackInt16(const uint8_t *buffer);
};
