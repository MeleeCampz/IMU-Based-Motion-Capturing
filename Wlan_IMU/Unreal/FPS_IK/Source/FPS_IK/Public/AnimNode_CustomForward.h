#pragma once
#include "Runtime/Engine/Classes/Animation/AnimNodeBase.h"
#include "CoreMinimal.h"
#include "AnimNode_CustomForward.generated.h"

USTRUCT(BlueprintInternalUseOnly)
struct FPS_IK_API FAnimNode_CustomForward : public FAnimNode_Base
{
	GENERATED_BODY()

		//FPoseLink - this can be any combination 
		//of other nodes, not just animation sequences
		//	so you could have an blend space leading into 
		//a layer blend per bone to just use the arm
		//	and then pass that into the PoseLink

	/** Base Pose - This Can Be Entire Anim Graph Up To This Point, or Any Combination of Other Nodes*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
		FPoseLink BasePose;

	/** Sample Property That Will Show Up as a Pin */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links, meta = (PinShownByDefault))
		float SampleFloat;

public:
	// Constructor 
	FAnimNode_CustomForward();

public:
	//Begin FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	//End FAnimNode_Base interface
};