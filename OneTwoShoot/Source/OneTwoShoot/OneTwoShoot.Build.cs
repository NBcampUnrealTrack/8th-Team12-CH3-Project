// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OneTwoShoot : ModuleRules
{
	public OneTwoShoot(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
            "ProceduralMeshComponent",
            "NavigationSystem",
			"AIModule",
			"UMG",
            "Slate",
            "SlateCore"
        }
	);

		PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG" });
	}
}
