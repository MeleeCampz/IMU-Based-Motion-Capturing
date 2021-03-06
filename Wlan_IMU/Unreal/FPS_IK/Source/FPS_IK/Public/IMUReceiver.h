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

UENUM(BlueprintType)
enum class EIMUSaveForamt : uint8
{
	Bianry	UMETA(DisplayName = "Binary"),
	CSV		UMETA(DisplayName = "CSV")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
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
		bool StopDataCapture(FString dataPath, EIMUSaveForamt format);
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

	void SendNetString(FString ipAddress, FString message);

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
		void InitOTAFirmwareUpdate(FString ipAddress);

	UFUNCTION(BlueprintCallable)
		void SendSamplingRateToAllClients(int SamplingRateInMicroSeconds);

	UFUNCTION(BlueprintCallable)
		int32 GetNumClients();

	UFUNCTION(BlueprintCallable)
		void GetClientInfo(TArray<FString>& names, TArray<int32>& ids, TArray<int32>& rates);

	UFUNCTION(BlueprintCallable)
		void StartDataCapture();

	UFUNCTION(BlueprintCallable)
		bool StopDataCapture(FString FilePath, EIMUSaveForamt format);

	UFUNCTION(BlueprintCallable)
		void Load(FString FilePath);

	UFUNCTION(BlueprintCallable)
		bool GetRotation(int ID, FQuat& out);

	UFUNCTION(BlueprintCallable)
		bool GetVelocity(int ID, FVector& out);

	UFUNCTION(BlueprintCallable)
		FRotator QuatToRot(FQuat in)
	{
		return FRotator(in);
	}

	UPROPERTY(BlueprintAssignable)
		FOnClientUpdate OnClientUpdate;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
