// Fill out your copyright notice in the Description page of Project Settings.

#include "IMUReceiver.h"
#include "TcpSocketBuilder.h"

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

	FIPv4Endpoint Endpoint(localIpAsInt, 6676);

	FTcpSocketBuilder builder = FTcpSocketBuilder(TEXT("IMUSOCKET"));
	builder = builder.AsReusable();
	builder = builder.BoundToEndpoint(Endpoint);
	builder = builder.Listening(MAX_CONNECTIONS);
		
	_socket = builder.Build();

	if (_socket &&_socket->SetReceiveBufferSize(sizeof(_reveiceBuffer), _bufferSize))
	{
		GetOwner()->GetWorldTimerManager().SetTimer(_timeHandleConnection, this, &UIMUReceiver::TCPConnectionListener, 0.01f, true, 0.0f);
		GetOwner()->GetWorldTimerManager().SetTimer(_timeHandleSocket, this, &UIMUReceiver::TCPSocketListener, 0.01f, true, 0.0f);

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Socket started listening!"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Socket failed listening!"));
	}

	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		_clients[i] = nullptr;
	}
}


void UIMUReceiver::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if(_socket)
	{
		_socket->Close();
	}
	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		if (_clients[i])
		{
			_clients[i]->Close();
		}
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
	if (_socket->HasPendingConnection(pending) && pending)
	{
		for (int i = 0; i < MAX_CONNECTIONS; i++)
		{
			if (!_clients[i])
			{
				_clients[i] = _socket->Accept(TEXT("OtherSocket"));
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Accepted Connection"));
				return;
			}
		}
	}
}
	


void UIMUReceiver::TCPSocketListener()
{
	uint32 pendingSize;
	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		FSocket* client = _clients[i];
		if (client && client->HasPendingData(pendingSize))
		{
			int bytesRead;
			client->Recv(&_reveiceBuffer[0], _bufferSize, bytesRead);
			UE_LOG(LogTemp, Warning, TEXT("bytesRead: %i"), bytesRead);

			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("bytesRead: %i"), bytesRead));
		}
		else if (client && client->GetConnectionState() != ESocketConnectionState::SCS_Connected)
		{
			client->Close();
			_clients[i] = nullptr;
		}
	}
}
