// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#include "AnimNode_CustomForward.h"
#include "AnimInstanceProxy.h"
#include "FPS_IK.h"

//#include "AnimationRuntime.h"

FAnimNode_CustomForward::FAnimNode_CustomForward()
	:SampleFloat(128.333)
{}

void FAnimNode_CustomForward::Initialize_AnyThread(const FAnimationInitializeContext & Context)
{
	FAnimNode_Base::Initialize_AnyThread(Context);

	BasePose.Initialize(Context);
}

void FAnimNode_CustomForward::CacheBones_AnyThread(const FAnimationCacheBonesContext & Context)
{
	BasePose.CacheBones(Context);
}

void FAnimNode_CustomForward::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	BasePose.Update(Context);
}

void FAnimNode_CustomForward::Evaluate_AnyThread(FPoseContext & Output)
{
	FPoseContext context(Output);
	BasePose.Evaluate(context);
	Output.Pose.CopyBonesFrom(context.Pose);

	for (FCompactPoseBoneIndex BoneIndex : Output.Pose.ForEachBoneIndex())
	{
		Output.Pose[BoneIndex].ConcatenateRotation(FQuat::MakeFromEuler(FVector(0, 0, SampleFloat)));
	}
}
