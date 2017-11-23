// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "IMUReceiver.h"
#include "IMUCharacter.generated.h"

USTRUCT(BlueprintType)
struct FBoneAnimationStructure
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int ID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FQuat CurrentRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FQuat RotationOffset;

	FBoneAnimationStructure()
	{
		ID = -1;
		BoneName = "None";
		CurrentRotation = FQuat::Identity;
		RotationOffset = FQuat::Identity;
	}
};


UCLASS()
class FPS_IK_API AIMUCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FBoneAnimationStructure> BoneRotationData;

public:
	// Sets default values for this character's properties
	AIMUCharacter();

private:
	UIMUReceiver* _imuReceiver;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	UFUNCTION(BlueprintCallable)
	void ApplyTPoseConfiguration();
};
