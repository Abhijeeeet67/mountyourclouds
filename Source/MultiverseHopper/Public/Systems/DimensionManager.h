#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DimensionManager.generated.h"

class UWorld;
class AActor;
class UNiagaraSystem;
class USoundBase;
class UCurveFloat;

UENUM(BlueprintType)
enum class EDimensionID : uint8
{
    Prime,
    Void,
    Ember,
    Frost,
    Verdant,
    Aether,
    Umbra,
    Count
};

USTRUCT(BlueprintType)
struct FDimensionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDimensionID DimensionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName DimensionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AmbientColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GravityScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeDilation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UnlockRequirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UNiagaraSystem* TransitionEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundBase* TransitionSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UCurveFloat* TransitionCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsUnlocked;
};

USTRUCT(BlueprintType)
struct FDimensionAnchor
{
    GENERATED_BODY()

    UPROPERTY()
    FVector Location;

    UPROPERTY()
    FRotator Rotation;

    UPROPERTY()
    EDimensionID SourceDimension;

    UPROPERTY()
    EDimensionID TargetDimension;

    UPROPERTY()
    float CreationTime;

    UPROPERTY()
    float Lifetime;

    UPROPERTY()
    bool bIsActive;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDimensionShift, EDimensionID, FromDimension, EDimensionID, ToDimension);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDimensionShiftComplete, EDimensionID, NewDimension);

UCLASS(BlueprintType, Blueprintable)
class MULTIVERSEHOPPER_API UDimensionManager : public UObject
{
    GENERATED_BODY()

public:
    UDimensionManager();

    UFUNCTION(BlueprintCallable, Category = "Dimension")
    void Initialize(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Dimension")
    void ShiftDimension(EDimensionID TargetDimension);

    UFUNCTION(BlueprintCallable, Category = "Dimension")
    bool CanShiftTo(EDimensionID TargetDimension) const;

    UFUNCTION(BlueprintCallable, Category = "Dimension")
    void CreateAnchor(EDimensionID TargetDimension);

    UFUNCTION(BlueprintCallable, Category = "Dimension")
    void RemoveAnchor(EDimensionID DimensionID);

    UFUNCTION(BlueprintCallable, Category = "Dimension")
    FDimensionAnchor GetAnchor(EDimensionID DimensionID) const;

    UFUNCTION(BlueprintCallable, Category = "Dimension")
    TArray<FDimensionAnchor> GetActiveAnchors() const;

    UFUNCTION(BlueprintPure, Category = "Dimension")
    EDimensionID GetCurrentDimension() const { return CurrentDimension; }

    UFUNCTION(BlueprintPure, Category = "Dimension")
    EDimensionID GetPreviousDimension() const { return PreviousDimension; }

    UFUNCTION(BlueprintPure, Category = "Dimension")
    bool IsShifting() const { return bIsShifting; }

    UFUNCTION(BlueprintPure, Category = "Dimension")
    float GetShiftProgress() const { return ShiftProgress; }

    UFUNCTION(BlueprintCallable, Category = "Dimension")
    FDimensionData GetDimensionData(EDimensionID DimensionID) const;

    UFUNCTION(BlueprintCallable, Category = "Dimension")
    void UnlockDimension(EDimensionID DimensionID);

    UFUNCTION(BlueprintCallable, Category = "Dimension")
    bool IsDimensionUnlocked(EDimensionID DimensionID) const;

    UPROPERTY(BlueprintAssignable, Category = "Dimension")
    FOnDimensionShift OnDimensionShiftStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dimension")
    FOnDimensionShiftComplete OnDimensionShiftCompleted;

protected:
    UPROPERTY()
    UWorld* World;

    UPROPERTY()
    EDimensionID CurrentDimension;

    UPROPERTY()
    EDimensionID PreviousDimension;

    UPROPERTY()
    EDimensionID TargetDimension;

    UPROPERTY()
    bool bIsShifting;

    UPROPERTY()
    float ShiftProgress;

    UPROPERTY()
    float ShiftDuration;

    UPROPERTY()
    TMap<EDimensionID, FDimensionData> DimensionDatabase;

    UPROPERTY()
    TMap<EDimensionID, FDimensionAnchor> ActiveAnchors;

    UPROPERTY()
    float MaxAnchorsPerDimension;

    UPROPERTY()
    float AnchorLifetime;

    void LoadDimensionDatabase();
    void StartShift(EDimensionID NewDimension);
    void UpdateShift(float DeltaTime);
    void CompleteShift();
    void ApplyDimensionEffects(EDimensionID DimensionID);
    void SpawnTransitionEffect();
    void PlayTransitionSound();
};