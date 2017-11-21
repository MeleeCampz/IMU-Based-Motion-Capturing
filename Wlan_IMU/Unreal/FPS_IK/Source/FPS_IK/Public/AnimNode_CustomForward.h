#pragma once
#include "CoreMinimal.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_CustomForward.generated.h"

USTRUCT()
struct FPS_IK_API FAnimNode_CustomForward : public FAnimNode_SkeletalControlBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IMU)
		TArray<FString> BoneNames;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IMU)
		TArray<FVector> BoneRots;

public:
	// Constructor 
	FAnimNode_CustomForward();

public:
	// FAnimNode_SkeletalControlBase interface
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	//virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	// FAnimNode_SkeletalControlBase interface

protected:
	// FAnimNode_SkeletalControlBase protected interface
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
	// End of FAnimNode_SkeletalControlBase protected interface
};