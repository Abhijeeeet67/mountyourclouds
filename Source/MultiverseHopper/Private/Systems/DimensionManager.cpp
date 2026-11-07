#include "Systems/DimensionManager.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"

UDimensionManager::UDimensionManager()
{
    World = nullptr;
    CurrentDimension = EDimensionID::Prime;
    PreviousDimension = EDimensionID::Prime;
    TargetDimension = EDimensionID::Prime;
    bIsShifting = false;
    ShiftProgress = 0.0f;
    ShiftDuration = 2.5f;
    MaxAnchorsPerDimension = 3;
    AnchorLifetime = 300.0f;
}

void UDimensionManager::Initialize(UWorld* InWorld)
{
    World = InWorld;
    LoadDimensionDatabase();
}

void UDimensionManager::LoadDimensionDatabase()
{
    FDimensionData Prime;
    Prime.DimensionID = EDimensionID::Prime;
    Prime.DimensionName = TEXT("Dimension_Prime");
    Prime.DisplayName = FText::FromString(TEXT("Prime Reality"));
    Prime.Description = FText::FromString(TEXT("The origin world - a mysterious fantasy kingdom with ancient forests and ruins."));
    Prime.AmbientColor = FLinearColor(1.0f, 0.95f, 0.85f);
    Prime.FogColor = FLinearColor(0.6f, 0.5f, 0.4f);
    Prime.FogDensity = 0.001f;
    Prime.GravityScale = 1.0f;
    Prime.TimeDilation = 1.0f;
    Prime.bIsUnlocked = true;
    DimensionDatabase.Add(Prime.DimensionID, Prime);

    FDimensionData Void;
    Void.DimensionID = EDimensionID::Void;
    Void.DimensionName = TEXT("Dimension_Void");
    Void.DisplayName = FText::FromString(TEXT("The Void Between"));
    Void.Description = FText::FromString(TEXT("A dimension of floating islands and endless darkness. Gravity is unstable."));
    Void.AmbientColor = FLinearColor(0.2f, 0.1f, 0.3f);
    Void.FogColor = FLinearColor(0.1f, 0.05f, 0.15f);
    Void.FogDensity = 0.003f;
    Void.GravityScale = 0.5f;
    Void.TimeDilation = 1.2f;
    Void.UnlockRequirements = { TEXT("Mission_001_FirstThreat") };
    Void.bIsUnlocked = false;
    DimensionDatabase.Add(Void.DimensionID, Void);

    FDimensionData Ember;
    Ember.DimensionID = EDimensionID::Ember;
    Ember.DimensionName = TEXT("Dimension_Ember");
    Ember.DisplayName = FText::FromString(TEXT("Ember Wastes"));
    Ember.Description = FText::FromString(TEXT("A scorching realm of volcanic rock and rivers of lava. Heat damages the unwary."));
    Ember.AmbientColor = FLinearColor(1.0f, 0.4f, 0.1f);
    Ember.FogColor = FLinearColor(0.8f, 0.3f, 0.1f);
    Ember.FogDensity = 0.002f;
    Ember.GravityScale = 1.1f;
    Ember.TimeDilation = 0.9f;
    Ember.UnlockRequirements = { TEXT("Mission_002_EmberTrial") };
    Ember.bIsUnlocked = false;
    DimensionDatabase.Add(Ember.DimensionID, Ember);

    FDimensionData Frost;
    Frost.DimensionID = EDimensionID::Frost;
    Frost.DimensionName = TEXT("Dimension_Frost");
    Frost.DisplayName = FText::FromString(TEXT("Frost Peaks"));
    Frost.Description = FText::FromString(TEXT("An eternal winter of crystalline ice and howling winds. Cold slows movement."));
    Frost.AmbientColor = FLinearColor(0.7f, 0.85f, 1.0f);
    Frost.FogColor = FLinearColor(0.5f, 0.7f, 0.9f);
    Frost.FogDensity = 0.0025f;
    Frost.GravityScale = 1.0f;
    Frost.TimeDilation = 1.0f;
    Frost.UnlockRequirements = { TEXT("Mission_003_FrostTrial") };
    Frost.bIsUnlocked = false;
    DimensionDatabase.Add(Frost.DimensionID, Frost);

    FDimensionData Verdant;
    Verdant.DimensionID = EDimensionID::Verdant;
    Verdant.DimensionName = TEXT("Dimension_Verdant");
    Verdant.DisplayName = FText::FromString(TEXT("Verdant Wilds"));
    Verdant.Description = FText::FromString(TEXT("An overgrown jungle where nature reclaims all. Poison and vines hinder progress."));
    Verdant.AmbientColor = FLinearColor(0.5f, 0.9f, 0.4f);
    Verdant.FogColor = FLinearColor(0.3f, 0.6f, 0.2f);
    Verdant.FogDensity = 0.003f;
    Verdant.GravityScale = 0.9f;
    Verdant.TimeDilation = 1.1f;
    Verdant.UnlockRequirements = { TEXT("Mission_004_VerdantTrial") };
    Verdant.bIsUnlocked = false;
    DimensionDatabase.Add(Verdant.DimensionID, Verdant);

    FDimensionData Aether;
    Aether.DimensionID = EDimensionID::Aether;
    Aether.DimensionName = TEXT("Dimension_Aether");
    Aether.DisplayName = FText::FromString(TEXT("Aether Spires"));
    Aether.Description = FText::FromString(TEXT("Floating citadels in an endless sky. Wind currents enable flight."));
    Aether.AmbientColor = FLinearColor(0.9f, 0.9f, 1.0f);
    Aether.FogColor = FLinearColor(0.7f, 0.7f, 0.9f);
    Aether.FogDensity = 0.001f;
    Aether.GravityScale = 0.3f;
    Aether.TimeDilation = 1.3f;
    Aether.UnlockRequirements = { TEXT("Mission_005_AetherTrial") };
    Aether.bIsUnlocked = false;
    DimensionDatabase.Add(Aether.DimensionID, Aether);

    FDimensionData Umbra;
    Umbra.DimensionID = EDimensionID::Umbra;
    Umbra.DimensionName = TEXT("Dimension_Umbra");
    Umbra.DisplayName = FText::FromString(TEXT("Umbra Depths"));
    Umbra.Description = FText::FromString(TEXT("A lightless abyss where shadows have form. Vision is limited, sound is key."));
    Umbra.AmbientColor = FLinearColor(0.1f, 0.1f, 0.15f);
    Umbra.FogColor = FLinearColor(0.05f, 0.05f, 0.1f);
    Umbra.FogDensity = 0.005f;
    Umbra.GravityScale = 1.2f;
    Umbra.TimeDilation = 0.8f;
    Umbra.UnlockRequirements = { TEXT("Mission_006_UmbraTrial") };
    Umbra.bIsUnlocked = false;
    DimensionDatabase.Add(Umbra.DimensionID, Umbra);
}

void UDimensionManager::ShiftDimension(EDimensionID TargetDimension)
{
    if (!CanShiftTo(TargetDimension) || bIsShifting)
    {
        return;
    }

    StartShift(TargetDimension);
}

bool UDimensionManager::CanShiftTo(EDimensionID TargetDimension) const
{
    if (TargetDimension == CurrentDimension)
    {
        return false;
    }

    if (const FDimensionData* Data = DimensionDatabase.Find(TargetDimension))
    {
        return Data->bIsUnlocked;
    }

    return false;
}

void UDimensionManager::StartShift(EDimensionID NewDimension)
{
    PreviousDimension = CurrentDimension;
    TargetDimension = NewDimension;
    bIsShifting = true;
    ShiftProgress = 0.0f;

    OnDimensionShiftStarted.Broadcast(PreviousDimension, TargetDimension);

    SpawnTransitionEffect();
    PlayTransitionSound();

    ApplyDimensionEffects(TargetDimension);
}

void UDimensionManager::UpdateShift(float DeltaTime)
{
    if (!bIsShifting)
    {
        return;
    }

    ShiftProgress += DeltaTime / ShiftDuration;
    ShiftProgress = FMath::Clamp(ShiftProgress, 0.0f, 1.0f);

    if (ShiftProgress >= 1.0f)
    {
        CompleteShift();
    }
}

void UDimensionManager::CompleteShift()
{
    CurrentDimension = TargetDimension;
    bIsShifting = false;
    ShiftProgress = 1.0f;

    OnDimensionShiftCompleted.Broadcast(CurrentDimension);

    if (UMultiverseGameInstance* GameInstance = Cast<UMultiverseGameInstance>(GEngine->GameInstance))
    {
        GameInstance->UpdateDimensionProgress(
            *DimensionDatabase[CurrentDimension].DimensionName.ToString(),
            EDimensionState::Current
        );
    }
}

void UDimensionManager::ApplyDimensionEffects(EDimensionID DimensionID)
{
    if (!World) return;

    const FDimensionData* Data = DimensionDatabase.Find(DimensionID);
    if (!Data) return;

    if (AWorldSettings* WorldSettings = World->GetWorldSettings())
    {
        WorldSettings->SetTimeDilation(Data->TimeDilation);
    }

    for (TActorIterator<ADirectionalLight> It(World); It; ++It)
    {
        ADirectionalLight* Light = *It;
        Light->SetLightColor(Data->AmbientColor);
    }

    if (AExponentialHeightFog* Fog = World->GetWorldSettings()->GetDefaultFogActor())
    {
        Fog->SetFogColor(Data->FogColor);
        Fog->FogDensity = Data->FogDensity;
    }

    if (UGameplayStatics::GetGlobalGravityZ(World) != 0)
    {
        UGameplayStatics::SetGlobalGravityZ(World, -980.0f * Data->GravityScale);
    }
}

void UDimensionManager::SpawnTransitionEffect()
{
    if (!World) return;

    const FDimensionData* FromData = DimensionDatabase.Find(PreviousDimension);
    const FDimensionData* ToData = DimensionDatabase.Find(TargetDimension);

    UNiagaraSystem* Effect = ToData ? ToData->TransitionEffect : nullptr;
    if (!Effect && FromData)
    {
        Effect = FromData->TransitionEffect;
    }

    if (Effect && UGameplayStatics::GetPlayerController(World, 0))
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
        if (PlayerPawn)
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(Effect, PlayerPawn->GetRootComponent(), NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
        }
    }
}

void UDimensionManager::PlayTransitionSound()
{
    if (!World) return;

    const FDimensionData* ToData = DimensionDatabase.Find(TargetDimension);
    if (ToData && ToData->TransitionSound)
    {
        UGameplayStatics::PlaySound2D(World, ToData->TransitionSound);
    }
}

void UDimensionManager::CreateAnchor(EDimensionID TargetDimension)
{
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    int32 AnchorCount = 0;
    for (const auto& Pair : ActiveAnchors)
    {
        if (Pair.Value.SourceDimension == CurrentDimension)
        {
            AnchorCount++;
        }
    }

    if (AnchorCount >= MaxAnchorsPerDimension)
    {
        return;
    }

    FDimensionAnchor Anchor;
    Anchor.Location = PlayerPawn->GetActorLocation();
    Anchor.Rotation = PlayerPawn->GetActorRotation();
    Anchor.SourceDimension = CurrentDimension;
    Anchor.TargetDimension = TargetDimension;
    Anchor.CreationTime = World->GetTimeSeconds();
    Anchor.Lifetime = AnchorLifetime;
    Anchor.bIsActive = true;

    ActiveAnchors.Add(TargetDimension, Anchor);
}

void UDimensionManager::RemoveAnchor(EDimensionID DimensionID)
{
    ActiveAnchors.Remove(DimensionID);
}

FDimensionAnchor UDimensionManager::GetAnchor(EDimensionID DimensionID) const
{
    if (const FDimensionAnchor* Anchor = ActiveAnchors.Find(DimensionID))
    {
        return *Anchor;
    }
    return FDimensionAnchor();
}

TArray<FDimensionAnchor> UDimensionManager::GetActiveAnchors() const
{
    TArray<FDimensionAnchor> Result;
    for (const auto& Pair : ActiveAnchors)
    {
        if (Pair.Value.bIsActive && World->GetTimeSeconds() - Pair.Value.CreationTime < Pair.Value.Lifetime)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

FDimensionData UDimensionManager::GetDimensionData(EDimensionID DimensionID) const
{
    if (const FDimensionData* Data = DimensionDatabase.Find(DimensionID))
    {
        return *Data;
    }
    return FDimensionData();
}

void UDimensionManager::UnlockDimension(EDimensionID DimensionID)
{
    if (FDimensionData* Data = DimensionDatabase.Find(DimensionID))
    {
        Data->bIsUnlocked = true;
    }
}

bool UDimensionManager::IsDimensionUnlocked(EDimensionID DimensionID) const
{
    if (const FDimensionData* Data = DimensionDatabase.Find(DimensionID))
    {
        return Data->bIsUnlocked;
    }
    return false;
}