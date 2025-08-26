

using UnrealBuildTool;
using System.Collections.Generic;

public class myGalleryTarget : TargetRules
{
	public myGalleryTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "myGallery" } );
	}
}
