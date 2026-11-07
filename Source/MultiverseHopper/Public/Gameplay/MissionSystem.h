#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MissionSystem.generated.h"

class ABrainrotCharacter;

UENUM(BlueprintType)
enum class EMissionState : uint8
{
    Inactive,
    Active,
    Completed,
    Failed
};

UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
    ReachLocation,
    KillEnemies,
    InteractWithObject,
    SurviveTime,
    FindItem,
    TalkToNPC,
    DefeatBoss
};

USTRUCT(BlueprintType)
struct FMissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EObjectiveType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AActor> TargetClass;
};

USTRUCT(BlueprintType)
struct FMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMissionObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentObjectiveIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMissionState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector StartLocation;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionUpdated, const FMissionData&, MissionData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompleted, FName, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionCompleted, FName, MissionID);

UCLASS(BlueprintType, Blueprintable)
class MULTIVERSEHOPPER_API UMissionSystem : public UObject
{
    GENERATED_BODY()

public:
    UMissionSystem();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void Initialize(ABrainrotCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void StartMission(FName MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void UpdateObjective(FName ObjectiveID, int32 ProgressAmount = 1);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void CompleteObjective(FName ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void FailMission();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    FMissionData GetCurrentMission() const;

    UFUNCTION(BlueprintCallable, Category = "Mission")
    FMissionObjective GetCurrentObjective() const;

    UFUNCTION(BlueprintCallable, Category = "Mission")
    bool IsMissionActive(FName MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void RegisterEnemyKill(AActor* Enemy);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void RegisterLocationReached(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void RegisterInteraction(AActor* InteractedActor);

    UPROPERTY(BlueprintAssignable, Category = "Mission")
    FOnMissionUpdated OnMissionUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Mission")
    FOnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Mission")
    FOnMissionCompleted OnMissionCompleted;

protected:
    UPROPERTY()
    ABrainrotCharacter* PlayerCharacter;

    UPROPERTY()
    TMap<FName, FMissionData> MissionDatabase;

    UPROPERTY()
    FMissionData CurrentMission;

    UPROPERTY()
    bool bHasActiveMission;

    void LoadMissionDatabase();
    void CheckObjectiveCompletion();
    void AdvanceToNextObjective();
    void CompleteMission();
    void BroadcastMissionUpdate();
};