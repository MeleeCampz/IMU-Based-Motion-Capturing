// Fill out your copyright notice in the Description page of Project Settings.

#include "IMUReceiver.h"
#include "UdpSocketBuilder.h"
#include "FileManager.h"
#include "Engine.h"

#define SWAP_UINT32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))

// Sets default values for this component's properties
UIMUReceiver::UIMUReceiver()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}


// Called when the game starts
void UIMUReceiver::BeginPlay()
{
	Super::BeginPlay();

	bool canBind = false;
	TSharedRef<FInternetAddr> localIp = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, canBind);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, localIp->ToString(false));
	}
	uint32_t localIpAsInt;
	localIp->GetIp(localIpAsInt);

	FIPv4Endpoint Endpoint(localIpAsInt, DATA_PORT);

	FUdpSocketBuilder dataSocketBuilder = FUdpSocketBuilder(TEXT("IMUSOCKET"))
		.AsReusable()
		.BoundToEndpoint(Endpoint)
		.WithReceiveBufferSize(_DataReceiveBufferSize);

	_DataReceiveSocket = dataSocketBuilder.Build();
	FTimespan ThreadWaitTimeData = FTimespan::FromMilliseconds(1000);

	_DataReceiver = new FUdpSocketReceiver(_DataReceiveSocket, ThreadWaitTimeData, TEXT("DataReceiveSocket"));
	_DataReceiver->OnDataReceived().BindUObject(this, &UIMUReceiver::RecvData);
	_DataReceiver->Start();


	FUdpSocketBuilder broadcastSocketBuilder = FUdpSocketBuilder(TEXT("UDPBroadcast"))
		.AsReusable()
		.BoundToAddress(FIPv4Address::Any).BoundToPort(BROADCAST_PORT)
		.WithReceiveBufferSize(_BroadcastReceiveBufferSize);

	_BroadcastSocket = broadcastSocketBuilder.Build();
	FTimespan ThreadWaitTimeBroadcast = FTimespan::FromMilliseconds(1000);

	_BroadcastReceiver = new FUdpSocketReceiver(_BroadcastSocket, ThreadWaitTimeBroadcast, TEXT("BroadcastReceiveSocket"));
	_BroadcastReceiver->OnDataReceived().BindUObject(this, &UIMUReceiver::RecvBroadcast);
	_BroadcastReceiver->Start();
}


void UIMUReceiver::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (_DataReceiver)
	{
		_DataReceiver->Stop();
		delete _DataReceiver;
		_DataReceiver = nullptr;
	}

	if (_DataReceiveSocket)
	{
		_DataReceiveSocket->Close();
		delete _DataReceiveSocket;
		_DataReceiveSocket = nullptr;
	}

	if (_BroadcastReceiver)
	{
		_BroadcastReceiver->Stop();
		delete _BroadcastReceiver;
		_BroadcastReceiver = nullptr;
	}

	if (_BroadcastSocket)
	{
		_BroadcastSocket->Close();
		delete _BroadcastSocket;
		_BroadcastSocket = nullptr;
	}
}

// Called every frame
void UIMUReceiver::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	bool updated = false;;
	while (!_clientsToAdd.IsEmpty())
	{
		IMUClient client;
		_clientsToAdd.Dequeue(client);

		bool contains = false;
		for (int i = 0; i < _clients.Num(); i++)
		{
			if (_clients[i].adr == client.adr) //ip already exsists
			{
				contains = true;
				if (_clients[i].ID != client.ID)
				{
					_clients[i].ID = client.ID;
					updated = true;
				}
				if (_clients[i].rate != client.rate)
				{
					_clients[i].rate = client.rate;
					updated = true;
				}
				break;
			}
		}

		if (!contains)
		{
			_clients.Add(client);
			updated = true;
		}
	}

	if (updated)
	{
		OnClientUpdate.Broadcast();
	}

	while (!_receivedPacketsQueue.IsEmpty())
	{
		IMUNetData data;
		_receivedPacketsQueue.Dequeue(data);
		if (_bCapture)
		{
			SaveLoadPacket(_writeArchive, data);
		}
		_IMUData.Add(data.ID, data);
	}
}

TSharedRef<FInternetAddr> UIMUReceiver::CreateAddr(FString addr, int32 port)
{
	TSharedRef<FInternetAddr> RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	RemoteAddr.Get().SetPort(port);
	bool valid;
	RemoteAddr.Get().SetIp(*addr, valid);
	return RemoteAddr;
}

void UIMUReceiver::SaveLoadPacket(FArchive& ar, IMUNetData& data)
{
	ar << data.timeStamp;
	ar << data.rotation[0];
	ar << data.rotation[1];
	ar << data.rotation[2];
	ar << data.velocity[0];
	ar << data.velocity[1];
	ar << data.velocity[2];
	ar << data.ID;
}

void UIMUReceiver::SendNetString(FString ipAddress, FString message)
{
	const uint8* req = (const uint8*)TCHAR_TO_ANSI(*message);
	int32 stringLen = message.Len() + 1;

	int32 bytesSent;
	TSharedRef<FInternetAddr> RemoteAddr = CreateAddr(ipAddress, BROADCAST_PORT);
	_BroadcastSocket->SendTo(req, stringLen, bytesSent, RemoteAddr.Get());
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("Bytes sent: %i (%s)"), bytesSent, *ipAddress));
	}
}

void UIMUReceiver::RecvBroadcast(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt)
{		
	int32 dataSize = ArrayReaderPtr->Num();
	int32 bytesSent;

	char stringData[1024];
	memcpy(stringData, ArrayReaderPtr->GetData(), dataSize);
	stringData[dataSize < 1024 ? dataSize : 1023] = 0; //Null terminator 
	FString message = ANSI_TO_TCHAR(stringData);

	//Ignore broadcasts that are not sent by esps
	if (!message.StartsWith("ESP"))
	{
		return;
	}

	//Send data right back so esp knows host pc address
	_BroadcastSocket->SendTo(ArrayReaderPtr->GetData(), dataSize, bytesSent, EndPt.ToInternetAddr().Get());

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Message: %s"), *message));
	}

	int id = -1;
	int rate = -1;
	FString leftSide, rightSide, rightrightSide;  //naming....
	if (message.Split("ID:", &leftSide, &rightSide))
	{
		if (rightSide.Split("_Rate:", &leftSide, &rightrightSide))
		{
			id = FCString::Atoi(*leftSide);
			rate = FCString::Atoi(*rightrightSide);
		}

	}

	_clientsToAdd.Enqueue(IMUClient(EndPt.ToInternetAddr().Get().ToString(false), id, rate));
}

void UIMUReceiver::RecvData(const FArrayReaderPtr & ArrayReaderPtr, const FIPv4Endpoint & EndPt)
{
	int32 dataSize = ArrayReaderPtr->Num();
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("bytesRead: %i"), dataSize));

	if (dataSize == sizeof(IMUNetData))
	{
		IMUNetData packet;
		memcpy(&packet, ArrayReaderPtr->GetData(), sizeof(IMUNetData));
		_receivedPacketsQueue.Enqueue(packet);
	}
}

void UIMUReceiver::SendMagnetometerCalibrateRequest(FString ipAddress)
{
	SendNetString(ipAddress, "MagCalib");
}

void UIMUReceiver::SendAccGyroCalibrateRequest(FString ipAddress)
{
	SendNetString(ipAddress, "SensorCalib");
}

void UIMUReceiver::SendIDRequest(FString ipAddress, int32 ID)
{
	FString message = "ID:" + FString::FromInt(ID);
	SendNetString(ipAddress, message);
}

void UIMUReceiver::InitOTAFirmwareUpdate(FString ipAddress)
{
	FString message = "OTA";
	SendNetString(ipAddress, message);
}

void UIMUReceiver::SendSamplingRateToAllClients(int SamplingRateInMicroSeconds)
{
	FString message = "SMPL_RATE:" + FString::FromInt(SamplingRateInMicroSeconds);
	for (int i = 0; i < _clients.Num(); i++)
	{
		SendNetString(_clients[i].adr, message);
	}
}

int32 UIMUReceiver::GetNumClients()
{
	return _clients.Num();
}

void UIMUReceiver::GetClientInfo(TArray<FString>& names, TArray<int32>& ids, TArray<int32>& rates)
{
	for (int i = 0; i < _clients.Num(); i++)
	{
		names.Add(_clients[i].adr);
		ids.Add(_clients[i].ID);
		rates.Add(_clients[i].rate);
	}
}

void UIMUReceiver::StartDataCapture()
{
	_writeArchive.Empty();
	_writeArchive.FlushCache();
	_writeArchive.Seek(0);
	_bCapture = true;
}

bool UIMUReceiver::StopDataCapture(FString dataPath, EIMUSaveForamt format)
{
	_bCapture = false;

	bool success = false;
	switch (format)
	{
	case EIMUSaveForamt::Bianry:
		success = FFileHelper::SaveArrayToFile(_writeArchive, *dataPath);
		break;
	case EIMUSaveForamt::CSV:	
		//Convert binary to string array
		FMemoryReader reader = FMemoryReader(_writeArchive, true);
		reader.Seek(0);
		
		TArray<FString> asString;
		FString header = "ID,RotX,RotY,RotZ,VelX,VelY,VelZ,Timestamp";
		asString.Add(header);
		while (reader.Tell() < reader.TotalSize())
		{
			IMUNetData packet;
			SaveLoadPacket(reader, packet);
			FQuat rot(-packet.rotation[1], packet.rotation[2], -packet.rotation[3], packet.rotation[0]); //why did I do this?
			FRotator rotator = rot.Rotator();
			FVector asVector = rotator.Euler();
			FString line = FString::Printf(TEXT("%d,%f,%f,%f,%f,%f,%f,%d"),
				packet.ID,
				asVector.X,
				asVector.Y,
				asVector.Z,
				packet.velocity[0],
				packet.velocity[1],
				packet.velocity[2],
				packet.timeStamp
				);
			asString.Add(line);
		}
		success = FFileHelper::SaveStringArrayToFile(asString, *dataPath, FFileHelper::EEncodingOptions::ForceUTF8);
		break;
	}
	if(success)
	{
		_writeArchive.Empty();
		_writeArchive.FlushCache();
		_writeArchive.Seek(0);
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("File saved! - Data cleard"));
		return true;
	}
	else
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to save file"));
		return false;
	}
}

void UIMUReceiver::Load(FString FilePath)
{
	TArray<uint8> binary;
	if (!FFileHelper::LoadFileToArray(binary, *FilePath))
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to load file"));
	}

	FMemoryReader reader = FMemoryReader(binary, true);
	reader.Seek(0);

	TArray<IMUNetData> data;
	while (reader.Tell() < reader.TotalSize())
	{
		IMUNetData packet;
		SaveLoadPacket(reader, packet);
		data.Add(packet);
	}
}

bool UIMUReceiver::GetRotation(int ID, FQuat& out)
{
	IMUNetData* data = _IMUData.Find(ID);
	if (!data)
	{
		return false;
	}

	//Covert left hand site to right hand side roation... or the other way around... it works somehow^^
	FQuat rot(-data->rotation[1], data->rotation[2], -data->rotation[3], data->rotation[0]);
	//FQuat rot(data->rotation[3], data->rotation[2], -data->rotation[1], data->rotation[0]);
	out = rot;

	return true;
}

bool UIMUReceiver::GetVelocity(int ID, FVector& out)
{
	IMUNetData* data = _IMUData.Find(ID);
	if (!data)
	{
		return false;
	}

	out = FVector(data->velocity[0], data->velocity[1], data->velocity[2]);

	return true;
}
