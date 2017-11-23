// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#include "AnimNode_CustomForward.h"
#include "AnimInstanceProxy.h"
#include "FPS_IK.h"


FAnimNode_CustomForward::FAnimNode_CustomForward()
{
}

void FAnimNode_CustomForward::GatherDebugData(FNodeDebugData & DebugData)
{
}

void FAnimNode_CustomForward::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext & Output, TArray<FBoneTransform>& OutBoneTransforms)
{	
	if (_bones.Num() != BoneTransforms.Rotations.Num())
		return;

	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();

	for (int i = 0; i < _bones.Num(); i++)
	{
		FCompactPoseBoneIndex index = _bones[i].GetCompactPoseIndex(BoneContainer);
		FTransform originalLocalTM = Output.Pose.GetLocalSpaceTransform(index);

		if (index == INDEX_NONE)
			continue;

		FCompactPoseBoneIndex parent = Output.Pose.GetPose().GetParentBoneIndex(index);

		const FTransform& parentTM = Output.Pose.GetComponentSpaceTransform(parent);
		originalLocalTM.SetRotation(BoneTransforms.Rotations[i]);
		OutBoneTransforms.Add(FBoneTransform(index, originalLocalTM * parentTM));
	}
}

bool FAnimNode_CustomForward::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
{
	for (FBoneReference boneRef : _bones)
	{
		if (!boneRef.IsValidToEvaluate())
		{
			return false;
		}
	}

	return true;
}

void FAnimNode_CustomForward::InitializeBoneReferences(const FBoneContainer & RequiredBones)
{	
	_bones.Empty();
	
	for (FName name : BoneTransforms.Names)
	{
		FBoneReference bone = FBoneReference(name);
		bone.Initialize(RequiredBones);
		_bones.Add(bone);
	}

	//Sort bones in descening order
	_bones.Sort([](const FBoneReference &a, const FBoneReference &b) { return a.BoneIndex < b.BoneIndex; });
	//Rotations have to be sorted as wel.... continue here!!!
}
