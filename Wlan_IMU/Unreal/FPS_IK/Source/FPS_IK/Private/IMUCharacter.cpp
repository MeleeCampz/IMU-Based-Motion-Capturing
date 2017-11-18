// Fill out your copyright notice in the Description page of Project Settings.

#include "IMUCharacter.h"

// Sets default values
AIMUCharacter::AIMUCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AIMUCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AIMUCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//GetMesh()->;
}

// Called to bind functionality to input
void AIMUCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

