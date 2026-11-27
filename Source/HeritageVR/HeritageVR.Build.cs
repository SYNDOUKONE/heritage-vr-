using UnrealBuildTool;

public class HeritageVR : ModuleRules
{
	public HeritageVR(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"HeadMountedDisplay",
			"OpenXR",
			"HTTP",
			"Json",
			"JsonUtilities",
			"AudioCaptureCore",
			"AudioMixer",
			"UMG",
			"Slate",
			"SlateCore"
		});
	}
}
