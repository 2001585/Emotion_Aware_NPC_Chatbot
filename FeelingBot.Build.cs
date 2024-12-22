using UnrealBuildTool;

public class FeelingBot : ModuleRules
{
    public FeelingBot(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput" });

        // HTTP, Json, JsonUtilities 모듈 추가
        PublicDependencyModuleNames.AddRange(new string[] { "HTTP", "Json", "JsonUtilities" });

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}
