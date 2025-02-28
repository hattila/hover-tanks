// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HoverTanks : ModuleRules
{
	public HoverTanks(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "Niagara", "OnlineSubsystem", "OnlineSubsystemSteam", "Landscape" });
		
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "GameplayAbilities", "GameplayTags", "GameplayTasks" });
	}
}
