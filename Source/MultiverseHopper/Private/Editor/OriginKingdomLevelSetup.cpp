#include "Editor/OriginKingdomLevelSetup.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"
#include "EditorLevelLibrary.h"
#include "EditorAssetLibrary.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/VolumetricCloud.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/TriggerBox.h"
#include "Engine/AmbientSound.h"
#include "AI/Navigation/NavMeshBoundsVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/BaseEnemy.h"
#include "Enemy/HeavyEnemy.h"

void UOriginKingdomLevelSetup::NativeConstruct()
{
    Super::NativeConstruct();

    SetupDefaultValues();

    if (GenerateButton)
        GenerateButton->OnClicked.AddDynamic(this, &UOriginKingdomLevelSetup::GenerateMission001);
    if (ClearButton)
        ClearButton->OnClicked.AddDynamic(this, &UOriginKingdomLevelSetup::ClearLevel);
    if (LightingButton)
        LightingButton->OnClicked.AddDynamic(this, &UOriginKingdomLevelSetup::SetupLightingOnly);
    if (NavigationButton)
        NavigationButton->OnClicked.AddDynamic(this, &UOriginKingdomLevelSetup::SetupNavigationOnly);
    if (SpawnActorsButton)
        SpawnActorsButton->OnClicked.AddDynamic(this, &UOriginKingdomLevelSetup::SpawnMissionActors);
}

void UOriginKingdomLevelSetup::SetupDefaultValues()
{
    if (LevelNameBox)
        LevelNameBox->SetText(FText::FromString(TEXT("/Game/Levels/Mission001/Mission001_FirstThreat")));

    if (SeedBox)
        SeedBox->SetText(FText::FromString(TEXT("12345")));

    if (QualityPresetCombo)
    {
        QualityPresetCombo->ClearOptions();
        QualityPresetCombo->AddOption(TEXT("Low (30 FPS)"));
        QualityPresetCombo->AddOption(TEXT("Medium (60 FPS)"));
        QualityPresetCombo->AddOption(TEXT("High (60 FPS + Lumen)"));
        QualityPresetCombo->AddOption(TEXT("Epic (60 FPS + RT)"));
        QualityPresetCombo->SetSelectedIndex(2);
    }

    if (GenerateFoliageCheck)
        GenerateFoliageCheck->SetIsChecked(true);
    if (GenerateStructuresCheck)
        GenerateStructuresCheck->SetIsChecked(true);
    if (GenerateEnemiesCheck)
        GenerateEnemiesCheck->SetIsChecked(true);
    if (GenerateMissionCheck)
        GenerateMissionCheck->SetIsChecked(true);
}

void UOriginKingdomLevelSetup::UpdateStatus(const FString& Message, float Progress)
{
    if (StatusText)
        StatusText->SetText(FText::FromString(Message));
    if (ProgressBar)
        ProgressBar->SetPercent(Progress);
}

void UOriginKingdomLevelSetup::ClearLevel()
{
    UpdateStatus(TEXT("Clearing level..."), 0.0f);

    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World) return;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->IsValidLowLevel())
        {
            // Keep essential actors
            FString ClassName = Actor->GetClass()->GetName();
            if (ClassName != "WorldSettings" && ClassName != "LevelScriptActor")
            {
                UEditorLevelLibrary::DestroyActor(Actor);
            }
        }
    }

    UpdateStatus(TEXT("Level cleared"), 1.0f);
}

void UOriginKingdomLevelSetup::SetupLightingOnly()
{
    UpdateStatus(TEXT("Setting up lighting..."), 0.0f);

    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World) return;

    // Remove existing lighting
    TArray<AActor*> LightActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), LightActors);
    for (AActor* Actor : LightActors) UEditorLevelLibrary::DestroyActor(Actor);

    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), LightActors);
    for (AActor* Actor : LightActors) UEditorLevelLibrary::DestroyActor(Actor);

    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), LightActors);
    for (AActor* Actor : LightActors) UEditorLevelLibrary::DestroyActor(Actor);

    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), LightActors);
    for (AActor* Actor : LightActors) UEditorLevelLibrary::DestroyActor(Actor);

    UGameplayStatics::GetAllActorsOfClass(World, AVolumetricCloud::StaticClass(), LightActors);
    for (AActor* Actor : LightActors) UEditorLevelLibrary::DestroyActor(Actor);

    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), LightActors);
    for (AActor* Actor : LightActors) UEditorLevelLibrary::DestroyActor(Actor);

    // Directional Light (Sun)
    ADirectionalLight* SunLight = World->SpawnActor<ADirectionalLight>(FVector(0, 0, 0), FRotator(-45, -30, 0));
    SunLight->SetActorLabel(TEXT("Sun_DirectionalLight"));
    UDirectionalLightComponent* SunComp = SunLight->GetDirectionalLightComponent();
    SunComp->SetIntensity(10.0f);
    SunComp->SetLightColor(FLinearColor(1.0f, 0.95f, 0.85f));
    SunComp->SetTemperature(5500.0f);
    SunComp->SetVolumetricScatteringIntensity(5.0f);
    SunComp->SetMobility(EComponentMobility::Movable);
    SunComp->SetAffectsWorld(true);
    SunComp->SetCastShadows(true);

    UpdateStatus(TEXT("Directional light created"), 0.2f);

    // Sky Light
    ASkyLight* SkyLight = World->SpawnActor<ASkyLight>(FVector(0, 0, 0), FRotator(0, 0, 0));
    SkyLight->SetActorLabel(TEXT("SkyLight"));
    USkyLightComponent* SkyComp = SkyLight->GetLightComponent();
    SkyComp->SetIntensity(1.5f);
    SkyComp->SetSourceType(ESkyLightSourceType::SLS_SpecifiedCubemap);
    SkyComp->SetMobility(EComponentMobility::Movable);

    UpdateStatus(TEXT("Sky light created"), 0.4f);

    // Sky Atmosphere
    ASkyAtmosphere* SkyAtmos = World->SpawnActor<ASkyAtmosphere>(FVector(0, 0, 0), FRotator(0, 0, 0));
    SkyAtmos->SetActorLabel(TEXT("SkyAtmosphere"));
    USkyAtmosphereComponent* AtmosComp = SkyAtmos->GetComponent();
    AtmosComp->SetSunMultiplier(1.0f);
    AtmosComp->SetRayleighScattering(FLinearColor(0.065f, 0.135f, 0.3f));
    AtmosComp->SetMieScattering(FLinearColor(0.006f, 0.006f, 0.006f));
    AtmosComp->SetMieAbsorption(FLinearColor(0.001f, 0.001f, 0.001f));

    UpdateStatus(TEXT("Sky atmosphere created"), 0.6f);

    // Exponential Height Fog
    AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(FVector(0, 0, 0), FRotator(0, 0, 0));
    Fog->SetActorLabel(TEXT("HeightFog"));
    UExponentialHeightFogComponent* FogComp = Fog->GetComponent();
    FogComp->SetFogDensity(0.0015f);
    FogComp->SetFogHeightFalloff(0.2f);
    FogComp->SetFogInscatteringColor(FLinearColor(0.6f, 0.5f, 0.4f));
    FogComp->SetFullyDirectionalInscatteringColorDistance(10000.0f);
    FogComp->SetNonDirectionalInscatteringColorDistance(5000.0f);
    FogComp->SetStartDistance(0.0f);

    UpdateStatus(TEXT("Height fog created"), 0.8f);

    // Volumetric Clouds
    AVolumetricCloud* Clouds = World->SpawnActor<AVolumetricCloud>(FVector(0, 0, 0), FRotator(0, 0, 0));
    Clouds->SetActorLabel(TEXT("VolumetricClouds"));
    UVolumetricCloudComponent* CloudComp = Clouds->GetComponent();
    CloudComp->SetLayerBottomAltitude(2000.0f);
    CloudComp->SetLayerThickness(3000.0f);

    UpdateStatus(TEXT("Volumetric clouds created"), 0.9f);

    // Post Process Volume
    APostProcessVolume* PPVolume = World->SpawnActor<APostProcessVolume>(FVector(0, 0, 0), FRotator(0, 0, 0));
    PPVolume->SetActorLabel(TEXT("GlobalPostProcess"));
    UPostProcessComponent* PPComp = PPVolume->GetComponent();
    PPComp->SetUnbound(true);
    PPComp->SetPriority(0);

    FPostProcessSettings& Settings = PPComp->Settings;
    Settings.BloomIntensity = 0.8f;
    Settings.BloomThreshold = 0.9f;
    Settings.AutoExposureMinBrightness = 0.5f;
    Settings.AutoExposureMaxBrightness = 2.0f;
    Settings.VignetteIntensity = 0.3f;
    Settings.FilmSlope = 0.9f;
    Settings.FilmToe = 0.5f;
    Settings.FilmShoulder = 0.3f;
    Settings.SceneColorTint = FLinearColor(1.0f, 0.98f, 0.95f);
    Settings.AmbientOcclusionIntensity = 0.8f;
    Settings.AmbientOcclusionRadius = 200.0f;
    Settings.MotionBlurAmount = 0.5f;

    UpdateStatus(TEXT("Lighting setup complete!"), 1.0f);
}

void UOriginKingdomLevelSetup::SetupNavigationOnly()
{
    UpdateStatus(TEXT("Setting up navigation..."), 0.0f);

    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World) return;

    // Remove existing nav mesh bounds
    TArray<AActor*> NavActors;
    UGameplayStatics::GetAllActorsOfClass(World, ANavMeshBoundsVolume::StaticClass(), NavActors);
    for (AActor* Actor : NavActors) UEditorLevelLibrary::DestroyActor(Actor);

    // Create new nav mesh bounds covering the world
    ANavMeshBoundsVolume* NavVolume = World->SpawnActor<ANavMeshBoundsVolume>(FVector(0, 0, 0), FRotator(0, 0, 0));
    NavVolume->SetActorLabel(TEXT("NavMeshBounds"));
    NavVolume->SetActorScale3D(FVector(10.0f, 10.0f, 5.0f));

    UpdateStatus(TEXT("Navigation bounds created"), 1.0f);
}

void UOriginKingdomLevelSetup::SpawnMissionActors()
{
    UpdateStatus(TEXT("Spawning mission actors..."), 0.0f);

    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World) return;

    // Key locations
    FVector VillageCenter(-1500.0f, 200.0f, 100.0f);
    FVector RuinsEntrance(2500.0f, 500.0f, 150.0f);
    FVector EnemyCamp(3500.0f, 0.0f, 150.0f);
    FVector PlayerStart(-1500.0f, 200.0f, 150.0f);

    // Player Start
    APlayerStart* PS = World->SpawnActor<APlayerStart>(PlayerStart, FRotator(0, 0, 0));
    PS->SetActorLabel(TEXT("PlayerStart_Village"));

    UpdateStatus(TEXT("Player start created"), 0.1f);

    // Basic Enemies (5)
    TArray<FVector> EnemyPositions = {
        FVector(3000.0f, 200.0f, 150.0f),
        FVector(3100.0f, 150.0f, 150.0f),
        FVector(2900.0f, 250.0f, 150.0f),
        FVector(3200.0f, 100.0f, 150.0f),
        FVector(2800.0f, 300.0f, 150.0f)
    };

    for (int32 i = 0; i < EnemyPositions.Num(); ++i)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        ABaseEnemy* Enemy = World->SpawnActor<ABaseEnemy>(ABaseEnemy::StaticClass(), EnemyPositions[i], FRotator(0, FMath::FRandRange(0, 360), 0), SpawnParams);
        if (Enemy)
        {
            Enemy->SetActorLabel(FString::Printf(TEXT("BasicEnemy_%d"), i));
            Enemy->MaxHealth = 80.0f;
            Enemy->CurrentHealth = 80.0f;
            Enemy->PatrolSpeed = 200.0f;
            Enemy->ChaseSpeed = 400.0f;
            Enemy->AttackRange = 180.0f;
            Enemy->CombatRange = 600.0f;
        }
    }

    UpdateStatus(TEXT("Basic enemies spawned"), 0.5f);

    // Heavy Enemy (Boss)
    FActorSpawnParameters BossSpawnParams;
    BossSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AHeavyEnemy* Boss = World->SpawnActor<AHeavyEnemy>(AHeavyEnemy::StaticClass(), EnemyCamp, FRotator(0, 180, 0), BossSpawnParams);
    if (Boss)
    {
        Boss->SetActorLabel(TEXT("HeavyEnemy_Boss"));
        Boss->MaxHealth = 400.0f;
        Boss->CurrentHealth = 400.0f;
        Boss->PatrolSpeed = 150.0f;
        Boss->ChaseSpeed = 300.0f;
        Boss->AttackRange = 350.0f;
        Boss->CombatRange = 1000.0f;
        Boss->GroundSlamRadius = 450.0f;
        Boss->GroundSlamDamage = 60.0f;
    }

    UpdateStatus(TEXT("Heavy enemy (boss) spawned"), 0.7f);

    // Mission Triggers
    ATriggerBox* Trigger1 = World->SpawnActor<ATriggerBox>(RuinsEntrance, FRotator(0, 0, 0));
    Trigger1->SetActorLabel(TEXT("Trigger_Obj1_RuinsEntrance"));
    Trigger1->GetBrushComponent()->SetBoxExtent(FVector(300, 300, 100));

    ATriggerBox* Trigger2 = World->SpawnActor<ATriggerBox>(FVector(3000, 200, 150), FRotator(0, 0, 0));
    Trigger2->SetActorLabel(TEXT("Trigger_Obj2_EnemyEncounter"));
    Trigger2->GetBrushComponent()->SetBoxExtent(FVector(400, 400, 100));

    ATriggerBox* Trigger3 = World->SpawnActor<ATriggerBox>(EnemyCamp, FRotator(0, 0, 0));
    Trigger3->SetActorLabel(TEXT("Trigger_Obj3_DefeatLeader"));
    Trigger3->GetBrushComponent()->SetBoxExtent(FVector(200, 200, 100));

    ATriggerBox* Trigger4 = World->SpawnActor<ATriggerBox>(VillageCenter, FRotator(0, 0, 0));
    Trigger4->SetActorLabel(TEXT("Trigger_Obj4_ReturnVillage"));
    Trigger4->GetBrushComponent()->SetBoxExtent(FVector(300, 300, 100));

    UpdateStatus(TEXT("Mission triggers created"), 0.9f);

    // Ambience Actors
    AAmbientSound* ForestAmb = World->SpawnActor<AAmbientSound>(FVector(-1000, 0, 200), FRotator(0, 0, 0));
    ForestAmb->SetActorLabel(TEXT("Ambience_Forest"));

    AAmbientSound* VillageAmb = World->SpawnActor<AAmbientSound>(VillageCenter, FRotator(0, 0, 0));
    VillageAmb->SetActorLabel(TEXT("Ambience_Village"));

    AAmbientSound* RiverAmb = World->SpawnActor<AAmbientSound>(FVector(1000, -1000, 100), FRotator(0, 0, 0));
    RiverAmb->SetActorLabel(TEXT("Ambience_River"));

    AAmbientSound* RuinsAmb = World->SpawnActor<AAmbientSound>(RuinsEntrance, FRotator(0, 0, 0));
    RuinsAmb->SetActorLabel(TEXT("Ambience_Ruins"));

    UpdateStatus(TEXT("Mission actors spawned!"), 1.0f);
}

void UOriginKingdomLevelSetup::GenerateMission001()
{
    UpdateStatus(TEXT("Generating Mission 001..."), 0.0f);

    ClearLevel();
    UpdateStatus(TEXT("Level cleared, setting up lighting..."), 0.1f);

    SetupLightingOnly();
    UpdateStatus(TEXT("Lighting done, setting up navigation..."), 0.3f);

    SetupNavigationOnly();
    UpdateStatus(TEXT("Navigation done, spawning mission actors..."), 0.5f);

    SpawnMissionActors();
    UpdateStatus(TEXT("Mission 001 generation complete!"), 1.0f);

    // Save level
    FString LevelName = LevelNameBox ? LevelNameBox->GetText().ToString() : TEXT("/Game/Levels/Mission001/Mission001_FirstThreat");
    UEditorLoadingAndSavingUtils::SaveMap(LevelName);
}