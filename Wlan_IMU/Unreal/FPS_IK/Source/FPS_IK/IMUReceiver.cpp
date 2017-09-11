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
			if (_clients[i].adr == client.adr)
			{
				contains = true;
				if (_clients[i].ID != client.ID)
				{
					_clients[i].ID = client.ID;
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

float UIMUReceiver::unpack_float(const uint8_t *buffer)
{
	float f;
	uint8_t b[] = { buffer[3], buffer[2], buffer[1], buffer[0] };
	memcpy(&f, &b, sizeof(f));

	//ESP8266 is little endian, so swap byte order!
	return swap_endian(f);
}

int16_t UIMUReceiver::unpack_int16(const uint8_t * buffer)
{
	int16_t ret;
	uint8_t b[] = { buffer[1], buffer[0] };
	memcpy(&ret, &b, sizeof(int16_t));

	return swap_endian(ret);
}

uint32_t UIMUReceiver::unpack_uint32(const uint8_t * buffer)
{
	uint32_t ret;
	uint8_t b[] = { buffer[3], buffer[2], buffer[1], buffer[0] };
	memcpy(&ret, &b, sizeof(uint32_t));

	return swap_endian(ret);
}

void UIMUReceiver::RecvBroadcast(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt)
{
	int32 dataSize = ArrayReaderPtr->Num();
	int32 bytesSent;
	//Send data right back
	_BroadcastSocket->SendTo(ArrayReaderPtr->GetData(), dataSize, bytesSent, EndPt.ToInternetAddr().Get());

	char stringData[1024];
	memcpy(stringData, ArrayReaderPtr->GetData(), dataSize);
	stringData[dataSize < 1024 ? dataSize : 1023] = 0; //Null terminator 
	FString message = ANSI_TO_TCHAR(stringData);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Message: %s"), *message));
	}

	int id = -1;
	FString leftSide, rightSide;
	if (message.Split("ID:", &leftSide, &rightSide))
	{
		id = FCString::Atoi(*rightSide);
	}

	_clientsToAdd.Enqueue(IMUClient(EndPt.ToInternetAddr().Get().ToString(false), id));
}

void UIMUReceiver::RecvData(const FArrayReaderPtr & ArrayReaderPtr, const FIPv4Endpoint & EndPt)
{
	int32 dataSize = ArrayReaderPtr->Num();
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("bytesRead: %i"), dataSize));

	if (dataSize >= 28)
	{
		IMUNetData packet;

		packet.timeStamp = unpack_uint32(&ArrayReaderPtr->GetData()[0]);

		packet.rotation[0] = unpack_float(&ArrayReaderPtr->GetData()[4]);
		packet.rotation[1] = unpack_float(&ArrayReaderPtr->GetData()[8]);
		packet.rotation[2] = unpack_float(&ArrayReaderPtr->GetData()[12]);

		packet.velocity[0] = unpack_float(&ArrayReaderPtr->GetData()[16]);
		packet.velocity[1] = unpack_float(&ArrayReaderPtr->GetData()[20]);
		packet.velocity[2] = unpack_float(&ArrayReaderPtr->GetData()[24]);

		packet.ID = unpack_int16(&ArrayReaderPtr->GetData()[28]);

		_receivedPacketsQueue.Enqueue(packet);

		//DebugRotation.Yaw = r1;
		//DebugRotation.Pitch = r2;
		//DebugRotation.Roll = r3;

		//DebugVelocity.X = v1;
		//DebugVelocity.Y = v2;
		//DebugVelocity.Z = v3;


		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Rotation: %f, %f, %f"), r1, r2, r3));
	}
}

void UIMUReceiver::SendCalibrateRequest(FString ipAddress)
{
	const uint8* req = (const uint8*)"MagCalib"; //TODO: Move this somewhere else
	int32 bytesSent;
	TSharedRef<FInternetAddr> RemoteAddr = CreateAddr(ipAddress, BROADCAST_PORT);
	_BroadcastSocket->SendTo(req, 9, bytesSent, RemoteAddr.Get());
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("Bytes sent: %i (%s)"), bytesSent, *ipAddress));
	}
}

void UIMUReceiver::SendIDRequest(FString ipAddress, int32 ID)
{
	FString message = "ID:" + FString::FromInt(ID);
	const uint8* req = (const uint8*)TCHAR_TO_ANSI(*message); //TODO: Move this somewhere else
	int32 bytesSent;
	TSharedRef<FInternetAddr> RemoteAddr = CreateAddr(ipAddress, BROADCAST_PORT);
	int32 stringLen = message.Len() + 1;

	_BroadcastSocket->SendTo(req, stringLen, bytesSent, RemoteAddr.Get());
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("Bytes sent: %i (%s)"), bytesSent, *ipAddress));
	}
}

int32 UIMUReceiver::GetNumClients()
{
	return _clients.Num();
}

void UIMUReceiver::GetClientInfo(TArray<FString>& names, TArray<int32>& ids)
{
	for (int i = 0; i < _clients.Num(); i++)
	{
		names.Add(_clients[i].adr);
		ids.Add(_clients[i].ID);
	}
}

void UIMUReceiver::StartDataCapture()
{
	_writeArchive.Empty();
	_bCapture = true;
}

bool UIMUReceiver::StopDataCapture(FString dataPath)
{
	_bCapture = false;

	if (FFileHelper::SaveArrayToFile(_writeArchive, *dataPath))
	{
		_writeArchive.FlushCache();
		_writeArchive.Empty();
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

bool UIMUReceiver::GetRotation(int ID, FRotator& out)
{
	IMUNetData * data = _IMUData.Find(ID);
	if (!data)
	{
		return false;
	}
	out.Pitch = data->rotation[0];
	out.Roll = data->rotation[1];
	out.Yaw = data->rotation[2];

	return true;
}