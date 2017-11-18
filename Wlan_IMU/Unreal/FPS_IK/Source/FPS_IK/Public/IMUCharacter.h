// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "IMUCharacter.generated.h"

//USTRUCT(BlueprintType)
//struct FIMUSensor
//{
//	GENERATED_BODY()
//	
//	UPROPERTY(EditAnywhere, BlueprintReadWrite)
//	int ID;
//	UPROPERTY(EditAnywhere, BlueprintReadWrite)
//	FString BoneName;
//};


UCLASS()
class FPS_IK_API AIMUCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AIMUCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//TArray<FIMUSensor> SensorData;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	
};
