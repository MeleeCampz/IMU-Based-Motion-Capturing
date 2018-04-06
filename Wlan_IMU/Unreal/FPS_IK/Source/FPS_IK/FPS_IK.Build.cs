// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FPS_IK : ModuleRules
{
	public FPS_IK(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicIncludePaths.AddRange(
			new string[]
			{
				"FPS_IK/Public"
			});

		PrivateIncludePaths.AddRange(
			new string[]
			{
				"FPS_IK/Private"
			});

		PublicDependencyModuleNames.AddRange(
			new string[] 
			{
				"Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "Sockets", "Networking"
			});
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"AnimGraphRuntime"
			});
	}
}
