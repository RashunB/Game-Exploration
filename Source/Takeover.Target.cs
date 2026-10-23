using UnrealBuildTool;

public class TakeoverTarget : TargetRules
{
	public TakeoverTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Takeover");
	}
}
