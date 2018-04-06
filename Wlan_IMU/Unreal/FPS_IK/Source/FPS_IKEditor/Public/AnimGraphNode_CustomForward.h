// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "AnimGraphNode_SkeletalControlBase.h"
#include "AnimNode_CustomForward.h"
#include "AnimGraphNode_CustomForward.generated.h"

/**
 * 
 */
UCLASS()
class FPS_IKEDITOR_API UAnimGraphNode_CustomForward : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_BODY()
	
	
		UPROPERTY(EditAnywhere, Category = Settings)
		FAnimNode_CustomForward Node;
	
public:
	// UEdGraphNode interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FString GetNodeCategory() const override;
	// End of UEdGraphNode interface
};
