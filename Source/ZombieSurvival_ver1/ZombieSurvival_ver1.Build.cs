// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ZombieSurvival_ver1 : ModuleRules
{
	public ZombieSurvival_ver1(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "UMG", "InputCore", "HeadMountedDisplay", "AIModule", "NavigationSystem" });
	}
}
