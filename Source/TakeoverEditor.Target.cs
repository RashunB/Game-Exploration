using UnrealBuildTool;

public class TakeoverEditorTarget : TargetRules
{
	public TakeoverEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Takeover");
	}
}
