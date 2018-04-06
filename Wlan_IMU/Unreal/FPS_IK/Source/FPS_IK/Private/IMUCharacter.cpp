// Fill out your copyright notice in the Description page of Project Settings.

#include "IMUCharacter.h"
#include "Runtime/Engine/Classes/Engine/SkeletalMeshSocket.h"

// Sets default values
AIMUCharacter::AIMUCharacter()
	:_imuReceiver(nullptr)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AIMUCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	_imuReceiver = Cast<UIMUReceiver>(GetComponentByClass(UIMUReceiver::StaticClass()));
}

// Called every frame
void AIMUCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!_imuReceiver)
		return;

	FQuat out;
	for (FBoneAnimationStructure& bone : BoneRotationData)
	{
		if (_imuReceiver->GetRotation(bone.ID, out))
		{
			bone.CurrentRotation = out * bone.TPoseOffset;
		}
	}
}

// Called to bind functionality to input
void AIMUCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AIMUCharacter::ApplyTPoseConfiguration()
{
	//Force TPose to be played, so the world roations of the virtual bones are correct
	GetMesh()->PlayAnimation(TPoseAnimation, false);
	//GetMesh()->ValidateAnimation();
	GetMesh()->TickAnimation(0.33f, true);	
	
	//TODO: Figure out a way to transform the sensor world space rotation to the correct bones component or world space roation
	for (FBoneAnimationStructure& bone : BoneRotationData)
	{
		FQuat curRotation, worldBoneRotation;
		if (!_imuReceiver->GetRotation(bone.ID, curRotation))
		{
			GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			return;
		}		

		worldBoneRotation = GetMesh()->GetBoneQuaternion(bone.BoneName, EBoneSpaces::WorldSpace);

		//const USkeletalMeshSocket* socket = GetMesh()->GetSocketByName(FName(*bone.BoneName.ToString().Append(TEXT("Socket"))));
		//localSocketRotation = FQuat(socket->RelativeRotation);
		//worldSocketRotation = localSocketRotation * worldBoneRotation;

		//bone.TPoseOffset = worldBoneRotation.Inverse() * curRotation.Inverse(); //Offset to bone zero position
		bone.TPoseOffset = curRotation.Inverse() * worldBoneRotation;
	}

	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
}

