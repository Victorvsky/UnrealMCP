using UnrealBuildTool;

public class UnrealMCP : ModuleRules
{
	public UnrealMCP(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Sockets",
			"Networking",
			"Json",
			"JsonUtilities",
			"InputCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd",
			"BlueprintGraph",
			"KismetCompiler",
			"Kismet",
			"Slate",
			"SlateCore",
			"Landscape",
			"LandscapeEditor",
			"Foliage",
			"LevelEditor",
			"ImageWrapper",
			"RenderCore",
			"UMG",
			"UMGEditor",
			"AssetTools",
			"EditorScriptingUtilities",
			"MovieScene",
			"MovieSceneTracks",
			"LevelSequence"
		});

		// Niagara support (optional — disable if Niagara plugin is not available)
		bool bEnableNiagara = true;
		if (bEnableNiagara)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"Niagara",
				"NiagaraCore",
				"NiagaraEditor"
			});
			PublicDefinitions.Add("WITH_NIAGARA=1");
		}
		else
		{
			PublicDefinitions.Add("WITH_NIAGARA=0");
		}
	}
}
