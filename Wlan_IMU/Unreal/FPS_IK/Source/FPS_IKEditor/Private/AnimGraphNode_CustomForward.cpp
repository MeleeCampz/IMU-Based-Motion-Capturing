// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimGraphNode_CustomForward.h"

FText UAnimGraphNode_CustomForward::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString("CustomForward");
}

FLinearColor UAnimGraphNode_CustomForward::GetNodeTitleColor() const
{
	return FLinearColor(0,128,128.1);
}

FString UAnimGraphNode_CustomForward::GetNodeCategory() const
{
	return FString("IMU");
}