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

	for (int i = 0; i < _bones.Num(); i++)
	{
		int rotIdx = BoneRotations.Names.IndexOfByKey(_bones[i].Key);
		if (rotIdx == INDEX_NONE)
			continue;
		
		FCompactPoseBoneIndex index = _bones[i].Value.GetCompactPoseIndex(BoneContainer);
		FTransform transform = Output.Pose.GetLocalSpaceTransform(index);
		FQuat localRot = transform.GetRotation();

		if (index == INDEX_NONE)
			continue;


		FCompactPoseBoneIndex parent = Output.Pose.GetPose().GetParentBoneIndex(index);
		const FTransform& parentTM = Output.Pose.GetComponentSpaceTransform(parent);
		
		//calculate component space transform
		transform *= parentTM; 
		//override rotaion
		transform.SetRotation(BoneRotations.Rotations[rotIdx]);

		//OutBoneTransforms.Add(FBoneTransform(index, originalLocalTM));
		//Can be optimized.... Problem is that, any parent bones have to be updated bofore the children, or they willend up being disjointed, due to wrong positon
		
		TArray<FBoneTransform> TempTransforms;
		TempTransforms.Add(FBoneTransform(FBoneTransform(index, transform)));
		Output.Pose.LocalBlendCSBoneTransforms(TempTransforms, 1.0f);
		TempTransforms.Reset();
	}
}

bool FAnimNode_CustomForward::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
{
	for (TPair<FName, FBoneReference> boneRef : _bones)
	{
		if (!boneRef.Value.IsValidToEvaluate())
		{
			return false;
		}
	}

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
