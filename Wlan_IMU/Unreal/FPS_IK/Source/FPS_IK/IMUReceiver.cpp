// Fill out your copyright notice in the Description page of Project Settings.

#include "IMUReceiver.h"
#include "TcpSocketBuilder.h"
#include "UdpSocketBuilder.h"


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
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, localIp->ToString(false));
	uint32_t localIpAsInt;
	localIp->GetIp(localIpAsInt);
	//FIPv4Address ip(127, 0, 0, 1);
	//TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	//addr->SetIp(ip.Value);

	FIPv4Endpoint Endpoint(localIpAsInt, TCP_PORT);

	FTcpSocketBuilder builder = FTcpSocketBuilder(TEXT("IMUSOCKET"));
	builder = builder.AsReusable();
	builder = builder.BoundToEndpoint(Endpoint);
	builder = builder.Listening(MAX_CONNECTIONS);

	_TCPReceiveSocket = builder.Build();

	FUdpSocketBuilder udpBuilder = FUdpSocketBuilder(TEXT("UDPBroadcast"));
	udpBuilder = udpBuilder.AsReusable();
	udpBuilder = udpBuilder.WithBroadcast();
	udpBuilder = udpBuilder.BoundToAddress(FIPv4Address::Any).BoundToPort(UDP_PORT);

	_UDPSocket = udpBuilder.Build();



	if (_TCPReceiveSocket &&_TCPReceiveSocket->SetReceiveBufferSize(sizeof(_TCPreveiceBuffer), _TCPreceiveBufferSize))
	{
		GetOwner()->GetWorldTimerManager().SetTimer(_timeHandleTCPConnection, this, &UIMUReceiver::TCPConnectionListener, 0.01f, true, 0.0f);
		GetOwner()->GetWorldTimerManager().SetTimer(_timeHandleTCPSocket, this, &UIMUReceiver::TCPSocketListener, 0.01f, true, 0.0f);

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("TCP Socket started listening!"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("TCP Socket failed listening!"));
	}

	if (_UDPSocket && _UDPSocket->SetReceiveBufferSize(sizeof(_UDPReceiveBuffer), _UDPReceiveBufferSize) && _UDPSocket->SetSendBufferSize(sizeof(_UDPReceiveBuffer), _UDPReceiveBufferSize))
	{
		GetOwner()->GetWorldTimerManager().SetTimer(_timeHandleUDPSocket, this, &UIMUReceiver::UDPSocketListener, 0.01f, true, 0.0f);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("UDP Socket started listening!"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("UDP Socket failed listening!"));
	}

	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		_clients[i] = nullptr;
	}


}


void UIMUReceiver::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (_TCPReceiveSocket)
	{
		delete _TCPReceiveSocket;
		_TCPReceiveSocket = nullptr;
	}
	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		if (_clients[i])
		{
			delete _clients[i];
			_clients[i] = nullptr;
		}
	}
	if (_UDPSocket)
	{
		delete _UDPSocket;
		_UDPSocket = nullptr;
	}
}

// Called every frame
void UIMUReceiver::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UIMUReceiver::TCPConnectionListener()
{
	bool pending;
	if (_TCPReceiveSocket->HasPendingConnection(pending) && pending)
	{
		for (int i = 0; i < MAX_CONNECTIONS; i++)
		{
			if (!_clients[i])
			{
				_clients[i] = _TCPReceiveSocket->Accept(TEXT("OtherSocket"));
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Accepted TCP-Connection"));
				return;
			}
		}
	}
}

float UIMUReceiver::unpackFloat(const uint8_t *buffer)
{
	float f;
	uint8_t b[] = { buffer[3], buffer[2], buffer[1], buffer[0] };
	memcpy(&f, &b, sizeof(f));

	//ESP8266 is little endian, so swap byte order!
	return swap_endian(f);
}

void UIMUReceiver::TCPSocketListener()
{
	uint32 pendingSize;
	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		FSocket* client = _clients[i];
		while (client && client->HasPendingData(pendingSize))
		{
			int bytesRead;
			client->Recv(&_TCPreveiceBuffer[0], _TCPreceiveBufferSize, bytesRead);

			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("bytesRead: %i"), bytesRead));

			if (bytesRead >= 12)
			{
				float r1 = unpackFloat(&_TCPreveiceBuffer[0]);
				float r2 = unpackFloat(&_TCPreveiceBuffer[4]);
				float r3 = unpackFloat(&_TCPreveiceBuffer[8]);

				DebugRotation.Yaw = r1;
				DebugRotation.Pitch = r2;
				DebugRotation.Roll = r3;

				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Rotation: %f, %f, %f"), r1, r2, r3));
			}
		}
		if (client && client->GetConnectionState() != ESocketConnectionState::SCS_Connected)
		{
			client->Close();
			_clients[i] = nullptr;
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("TCP-Connection closed"));
		}
	}
}

void UIMUReceiver::UDPSocketListener()
{
	int32 bytesRead, bytesSent;
	uint32 pendingData;
	while (_UDPSocket->HasPendingData(pendingData))
	{
		TSharedRef<FInternetAddr> senderAdr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		_UDPSocket->GetPeerAddress(senderAdr.Get());

		_UDPSocket->RecvFrom(_UDPReceiveBuffer, _UDPReceiveBufferSize, bytesRead, *senderAdr);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("bytesRead: %i"), bytesRead));

		uint32 adrs;
		senderAdr->GetIp(adrs);
		int32 port = senderAdr->GetPort();

		_UDPSocket->SendTo(_UDPReceiveBuffer, bytesRead, bytesSent, senderAdr.Get());
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("bytesSent: %i"), bytesSent));
	}
}
