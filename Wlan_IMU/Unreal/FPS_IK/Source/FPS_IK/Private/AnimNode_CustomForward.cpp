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
	if (BoneRotations.Names.Num() != BoneRotations.Rotations.Num())
		return;
	
	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();
	const FBoneContainer& BoneContainerUnModified = ForwardedPose.GetPose().GetBoneContainer();

	//MAp to store applied rotations that we don't want to be applied to child bones
	TMap<int32, FQuat> appliedRotaions;
	
	for (int i = 0; i < _bones.Num(); i++)
	{
		int rotIdx = BoneRotations.Names.IndexOfByKey(_bones[i].Key);
		if (rotIdx == INDEX_NONE)
			continue;

		FQuat inRot = BoneRotations.Rotations[rotIdx];
		
		FCompactPoseBoneIndex index = _bones[i].Value.GetCompactPoseIndex(BoneContainer);
		if (index == INDEX_NONE)
			continue;

		//override rotaion with offset applied to input pose
		FQuat oldRotInv = FQuat::Identity;
		FCompactPoseBoneIndex curParent = Output.Pose.GetPose().GetParentBoneIndex(index);
		
		//Only works if we don't skip a bone in the animation chain....
		while (curParent != INDEX_NONE)
		{
			FQuat* mapVal = appliedRotaions.Find(curParent.GetInt());
			if (mapVal)
			{
				oldRotInv *= mapVal->Inverse();
				break;	
			}
			curParent = Output.Pose.GetPose().GetParentBoneIndex(curParent);
		}
		
		FTransform CSTransform = Output.Pose.GetComponentSpaceTransform(index);
		FQuat compSpaceRot = CSTransform.GetRotation();
		FQuat rotationOverride = compSpaceRot * inRot *oldRotInv;
		appliedRotaions.Add(index.GetInt(), inRot);
		CSTransform.SetRotation(rotationOverride);

		//OutBoneTransforms.Add(FBoneTransform(index, transform));
		//Can be optimized.... Problem is that, any parent bones have to be updated bofore the children, or they willend up being disjointed, due to wrong positon
		TArray<FBoneTransform> TempTransforms;
		TempTransforms.Add(FBoneTransform(index, CSTransform));
		Output.Pose.LocalBlendCSBoneTransforms(TempTransforms, 1.0f);
		TempTransforms.Reset();
	}
}

bool FAnimNode_CustomForward::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
{
	for (TPair<FName, FBoneReference> boneRef : _bones)
		if (!boneRef.Value.IsValidToEvaluate())
			return false;

	return true;
}

void FAnimNode_CustomForward::InitializeBoneReferences(const FBoneContainer & RequiredBones)
{	
	_bones.Empty();	
	for (FName name : BoneRotations.Names)
	{
		FBoneReference bone = FBoneReference(name);
		bone.Initialize(RequiredBones);
		_bones.Add(TPairInitializer<FName, FBoneReference>(name, bone));
	}
	//Bones habe to be sorted in descendinf order, so paraent bones are updated before their childs
	_bones.Sort([](const TPair<FName, FBoneReference> &a, const TPair<FName, FBoneReference> &b) { return a.Value.BoneIndex < b.Value.BoneIndex; })
;}
