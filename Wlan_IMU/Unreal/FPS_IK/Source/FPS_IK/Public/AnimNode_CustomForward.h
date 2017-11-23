#pragma once
#include "CoreMinimal.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_CustomForward.generated.h"

USTRUCT(BlueprintType)
struct FBoneRotations
{
	GENERATED_BODY()
		/*Array of names*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BonesTransfroms")
	TArray<FName> Names;
	/*Array of transforms*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BonesTransfroms")
	TArray<FQuat> Rotations;
};


USTRUCT()
struct FPS_IK_API FAnimNode_CustomForward : public FAnimNode_SkeletalControlBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "IMU", meta = (PinShownByDefault))
	FBoneRotations BoneRotations;
		
	//TMap<FName, FQuat> rotations;

private:
	TArray<TPair<FName, FBoneReference>> _bones;

public:
	// Constructor 
	FAnimNode_CustomForward();

public:
	// FAnimNode_SkeletalControlBase interface
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	// FAnimNode_SkeletalControlBase interface

protected:
	// FAnimNode_SkeletalControlBase protected interface
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
	// End of FAnimNode_SkeletalControlBase protected interface
};