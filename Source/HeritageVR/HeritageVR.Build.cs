using UnrealBuildTool;

public class HeritageVR : ModuleRules
{
	public HeritageVR(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"HTTP",
			"AudioCaptureCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"HeadMountedDisplay",
			"OpenXR",
			"Json",
			"JsonUtilities",
			"AudioMixer",
			"UMG",
			"Slate",
			"SlateCore"
		});
	}
}
