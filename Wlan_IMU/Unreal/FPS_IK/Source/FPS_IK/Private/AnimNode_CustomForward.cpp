// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#include "AnimNode_CustomForward.h"
#include "AnimInstanceProxy.h"
#include "FPS_IK.h"


FAnimNode_CustomForward::FAnimNode_CustomForward()
{}

void FAnimNode_CustomForward::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext & Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();
}

//bool FAnimNode_CustomForward::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
//{
//	//return FAnimNode_SkeletalControlBase::IsValidToEvaluate(Skeleton, RequiredBones);
//}
//
void FAnimNode_CustomForward::InitializeBoneReferences(const FBoneContainer & RequiredBones)
{
	//Continue here!!!
}
