// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPS_IKGameMode.h"
#include "FPS_IKCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFPS_IKGameMode::AFPS_IKGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
