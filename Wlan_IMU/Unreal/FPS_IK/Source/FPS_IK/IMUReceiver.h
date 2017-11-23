// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UnrealNetwork.h"
#include "Sockets.h"
#include "CoreMinimal.h"
#include "UdpSocketReceiver.h"
#include "BufferArchive.h"
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
		int32 rate;
		
		IMUClient(FString address, int32 id, int32 smplrate)
			: adr(address), ID(id), rate(smplrate)
		{}

		IMUClient()
			:adr("INVALID"), ID(-1), rate(-1)
		{}
	};

	struct IMUNetData
	{
		uint32_t timeStamp; //in microseconds
		float rotation[4];
		float velocity[3]; // Unit is G*s 
		int16_t ID; //put id last, to optimize padding, could be a int32 as well, as there is 2byte padding atm
	};

	FSocket* _DataReceiveSocket;
	FUdpSocketReceiver* _DataReceiver = nullptr;
	int32 _DataReceiveBufferSize = sizeof(IMUNetData); //in bytes
	static const int32 DATA_PORT = 6676;
	
	FSocket* _BroadcastSocket;
	FUdpSocketReceiver* _BroadcastReceiver = nullptr;
	int32 _BroadcastReceiveBufferSize = 48; //in bytes
	static const int32 BROADCAST_PORT = 6678;

	TQueue<IMUNetData> _receivedPacketsQueue;

	TArray<IMUClient> _clients;
	TQueue<IMUClient> _clientsToAdd;

	FBufferArchive _writeArchive;

	TMap<int32_t, IMUNetData> _IMUData;

	FString _filePath;
	bool _bCapture = false;

	TSharedRef<FInternetAddr> CreateAddr(FString addr, int32 port);
	//Saves or load data - depending on operator overload of "<<"
	void SaveLoadPacket(FArchive& ar, IMUNetData& data);

	void SetNetString(FString ipAddress, FString message);

public:	
	// Sets default values for this component's properties
	UIMUReceiver();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void RecvBroadcast(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);
	void RecvData(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);

	UFUNCTION(BlueprintCallable)
		void SendMagnetometerCalibrateRequest(FString ipAddress);

	UFUNCTION(BlueprintCallable)
		void SendAccGyroCalibrateRequest(FString ipAddress);

	UFUNCTION(BlueprintCallable)
		void SendIDRequest(FString ipAddress, int32 ID);
	
	UFUNCTION(BlueprintCallable)
		void SendSamplingRateToAllClients(int SamplingRateInMicroSeconds);

	UFUNCTION(BlueprintCallable)
		int32 GetNumClients();

	UFUNCTION(BlueprintCallable)
		void GetClientInfo(TArray<FString>& names, TArray<int32>& ids, TArray<int32>& rates);

	UFUNCTION(BlueprintCallable)
		void StartDataCapture();

	UFUNCTION(BlueprintCallable)
		bool StopDataCapture(FString FilePath);

	UFUNCTION(BlueprintCallable)
		void Load(FString FilePath);

	UFUNCTION(BlueprintCallable)
		bool GetRotation(int ID, FQuat& out);

	UPROPERTY(BlueprintAssignable)
		FOnClientUpdate OnClientUpdate;

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

	float unpack_float(const uint8_t *buffer);
	int16_t unpack_int16(const uint8_t *buffer);
	uint32_t unpack_uint32(const uint8_t *buffer);
};
