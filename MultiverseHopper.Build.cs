using UnrealBuildTool;

public class MultiverseHopper : ModuleRules
{
    public MultiverseHopper(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "InputCore",
            "EnhancedInput", "GameplayAbilities", "GameplayTags",
            "GameplayTasks", "Niagara", "UMG", "Slate", "SlateCore",
            "PhysicsCore", "Chaos", "ChaosVehicles", "ChaosCloth",
            "GeometryCollectionEngine", "FieldSystemEngine",
            "AudioMixer", "Synthesis", "MediaAssets", "MoviePlayer",
            "CinematicCamera", "LevelSequence", "MovieScene",
            "MovieSceneTracks", "MovieSceneCapture", "ImageWriteQueue",
            "RenderCore", "RHI", "ShaderCore", "Projects",
            "AIModule", "NavigationSystem"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "InputCore", "EnhancedInput", "GameplayAbilities",
            "GameplayTags", "GameplayTasks", "Niagara", "UMG",
            "PhysicsCore", "Chaos", "GeometryCollectionEngine",
            "DeveloperSettings", "Settings", "AssetRegistry",
            "ContentBrowser", "UnrealEd", "EditorStyle", "EditorWidgets"
        });

        // Enable C++20 features
        CppStandard = CppStandardVersion.Cpp20;

        // Optimization for shipping
        if (Target.Configuration == UnrealTargetConfiguration.Shipping)
        {
            bUseUnityBuild = true;
            bUsePCHFiles = true;
        }

        // Enable modules for dimension system
        PublicDefinitions.Add("MULTIVERSE_HOPPER=1");
        PublicDefinitions.Add("WITH_DIMENSION_SYSTEM=1");

        // Editor module (only for editor builds)
        if (Target.Type == TargetRules.TargetType.Editor)
        {
            PublicDependencyModuleNames.Add("UnrealEd");
            PrivateDependencyModuleNames.Add("EditorWidgets");
        }
    }
}