

using UnrealBuildTool;
using System.Collections.Generic;

public class myGalleryEditorTarget : TargetRules
{
	public myGalleryEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "myGallery" } );
	}
}
