// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FPS_IKEditor : ModuleRules
{
	public FPS_IKEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				"FPS_IKEditor/Public"
			});

		PrivateIncludePaths.AddRange(
			new string[]
			{
				"FPS_IKEditor/Private"
			});

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"FPS_IK", "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "Sockets", "Networking"
			});
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd", "BlueprintGraph", "AnimGraph"
			});

		PrivateIncludePathModuleNames.AddRange(
			new string[]
			{
			});

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			});
	}
}
