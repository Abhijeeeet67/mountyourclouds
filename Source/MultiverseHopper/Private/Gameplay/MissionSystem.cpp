#include "Gameplay/MissionSystem.h"
#include "Character/BrainrotCharacter.h"
#include "Enemy/BaseEnemy.h"
#include "Kismet/GameplayStatics.h"

UMissionSystem::UMissionSystem()
{
    PlayerCharacter = nullptr;
    bHasActiveMission = false;
    CurrentMission.State = EMissionState::Inactive;
    CurrentMission.CurrentObjectiveIndex = 0;
}

void UMissionSystem::Initialize(ABrainrotCharacter* InPlayerCharacter)
{
    PlayerCharacter = InPlayerCharacter;
    LoadMissionDatabase();
}

void UMissionSystem::LoadMissionDatabase()
{
    FMissionData Mission001;
    Mission001.MissionID = TEXT("Mission_001_FirstThreat");
    Mission001.Title = FText::FromString(TEXT("MISSION 001: THE FIRST THREAT"));
    Mission001.Description = FText::FromString(TEXT("A mysterious enemy group has appeared near the abandoned ruins. Investigate and eliminate the threat."));
    Mission001.State = EMissionState::Inactive;
    Mission001.CurrentObjectiveIndex = 0;
    Mission001.TimeLimit = 0.0f;
    Mission001.StartLocation = FVector(-1500.0f, 200.0f, 100.0f);

    FMissionObjective Obj1;
    Obj1.ObjectiveID = TEXT("Obj_ExploreForest");
    Obj1.Description = FText::FromString(TEXT("Explore the forest and find the abandoned ruins"));
    Obj1.Type = EObjectiveType::ReachLocation;
    Obj1.RequiredCount = 1;
    Obj1.CurrentCount = 0;
    Obj1.bIsOptional = false;
    Obj1.TargetLocation = FVector(1200.0f, -800.0f, 120.0f);
    Obj1.TargetRadius = 300.0f;

    FMissionObjective Obj2;
    Obj2.ObjectiveID = TEXT("Obj_DiscoverRuins");
    Obj2.Description = FText::FromString(TEXT("Discover the abandoned ruins"));
    Obj2.Type = EObjectiveType::ReachLocation;
    Obj2.RequiredCount = 1;
    Obj2.CurrentCount = 0;
    Obj2.bIsOptional = false;
    Obj2.TargetLocation = FVector(2500.0f, 500.0f, 150.0f);
    Obj2.TargetRadius = 200.0f;

    FMissionObjective Obj3;
    Obj3.ObjectiveID = TEXT("Obj_EncounterEnemies");
    Obj3.Description = FText::FromString(TEXT("Encounter hostile enemies near the ruins"));
    Obj3.Type = EObjectiveType::ReachLocation;
    Obj3.RequiredCount = 1;
    Obj3.CurrentCount = 0;
    Obj3.bIsOptional = false;
    Obj3.TargetLocation = FVector(3000.0f, 200.0f, 150.0f);
    Obj3.TargetRadius = 150.0f;

    FMissionObjective Obj4;
    Obj4.ObjectiveID = TEXT("Obj_DefeatEnemies");
    Obj4.Description = FText::FromString(TEXT("Defeat the enemy patrol (0/5)"));
    Obj4.Type = EObjectiveType::KillEnemies;
    Obj4.RequiredCount = 5;
    Obj4.CurrentCount = 0;
    Obj4.bIsOptional = false;

    FMissionObjective Obj5;
    Obj5.ObjectiveID = TEXT("Obj_DefeatLeader");
    Obj5.Description = FText::FromString(TEXT("Defeat the enemy leader"));
    Obj5.Type = EObjectiveType::DefeatBoss;
    Obj5.RequiredCount = 1;
    Obj5.CurrentCount = 0;
    Obj5.bIsOptional = false;
    Obj5.TargetLocation = FVector(3500.0f, 0.0f, 150.0f);
    Obj5.TargetRadius = 100.0f;

    FMissionObjective Obj6;
    Obj6.ObjectiveID = TEXT("Obj_ReturnToVillage");
    Obj6.Description = FText::FromString(TEXT("Return to Tung's village"));
    Obj6.Type = EObjectiveType::ReachLocation;
    Obj6.RequiredCount = 1;
    Obj6.CurrentCount = 0;
    Obj6.bIsOptional = false;
    Obj6.TargetLocation = FVector(-1500.0f, 200.0f, 100.0f);
    Obj6.TargetRadius = 200.0f;

    Mission001.Objectives = { Obj1, Obj2, Obj3, Obj4, Obj5, Obj6 };
    MissionDatabase.Add(Mission001.MissionID, Mission001);
}

void UMissionSystem::StartMission(FName MissionID)
{
    if (FMissionData* MissionData = MissionDatabase.Find(MissionID))
    {
        CurrentMission = *MissionData;
        CurrentMission.State = EMissionState::Active;
        CurrentMission.CurrentObjectiveIndex = 0;
        bHasActiveMission = true;

        for (FMissionObjective& Obj : CurrentMission.Objectives)
        {
            Obj.CurrentCount = 0;
        }

        BroadcastMissionUpdate();
    }
}

void UMissionSystem::UpdateObjective(FName ObjectiveID, int32 ProgressAmount)
{
    if (!bHasActiveMission || CurrentMission.State != EMissionState::Active)
    {
        return;
    }

    for (FMissionObjective& Obj : CurrentMission.Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID)
        {
            Obj.CurrentCount = FMath::Min(Obj.RequiredCount, Obj.CurrentCount + ProgressAmount);

            if (Obj.Type == EObjectiveType::KillEnemies)
            {
                Obj.Description = FText::FromString(FString::Printf(TEXT("Defeat the enemy patrol (%d/%d)"), Obj.CurrentCount, Obj.RequiredCount));
            }

            CheckObjectiveCompletion();
            BroadcastMissionUpdate();
            break;
        }
    }
}

void UMissionSystem::CompleteObjective(FName ObjectiveID)
{
    if (!bHasActiveMission)
    {
        return;
    }

    for (FMissionObjective& Obj : CurrentMission.Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID)
        {
            Obj.CurrentCount = Obj.RequiredCount;
            OnObjectiveCompleted.Broadcast(ObjectiveID);
            CheckObjectiveCompletion();
            BroadcastMissionUpdate();
            break;
        }
    }
}

void UMissionSystem::CheckObjectiveCompletion()
{
    if (!bHasActiveMission)
    {
        return;
    }

    bool bAllObjectivesComplete = true;
    for (const FMissionObjective& Obj : CurrentMission.Objectives)
    {
        if (!Obj.bIsOptional && Obj.CurrentCount < Obj.RequiredCount)
        {
            bAllObjectivesComplete = false;
            break;
        }
    }

    if (bAllObjectivesComplete)
    {
        CompleteMission();
    }
    else
    {
        AdvanceToNextObjective();
    }
}

void UMissionSystem::AdvanceToNextObjective()
{
    for (int32 i = CurrentMission.CurrentObjectiveIndex; i < CurrentMission.Objectives.Num(); ++i)
    {
        const FMissionObjective& Obj = CurrentMission.Objectives[i];
        if (!Obj.bIsOptional && Obj.CurrentCount < Obj.RequiredCount)
        {
            CurrentMission.CurrentObjectiveIndex = i;
            break;
        }
    }
}

void UMissionSystem::CompleteMission()
{
    CurrentMission.State = EMissionState::Completed;
    bHasActiveMission = false;
    OnMissionCompleted.Broadcast(CurrentMission.MissionID);
    BroadcastMissionUpdate();
}

void UMissionSystem::FailMission()
{
    CurrentMission.State = EMissionState::Failed;
    bHasActiveMission = false;
    BroadcastMissionUpdate();
}

void UMissionSystem::RegisterEnemyKill(AActor* Enemy)
{
    if (!Enemy) return;

    ABaseEnemy* BaseEnemy = Cast<ABaseEnemy>(Enemy);
    if (BaseEnemy)
    {
        UpdateObjective(TEXT("Obj_DefeatEnemies"), 1);

        if (BaseEnemy->EnemyType == EEnemyType::HeavyMelee || BaseEnemy->EnemyType == EEnemyType::Boss)
        {
            CompleteObjective(TEXT("Obj_DefeatLeader"));
        }
    }
}

void UMissionSystem::RegisterLocationReached(FVector Location)
{
    if (!bHasActiveMission) return;

    FMissionObjective& CurrentObj = CurrentMission.Objectives[CurrentMission.CurrentObjectiveIndex];
    if (CurrentObj.Type == EObjectiveType::ReachLocation)
    {
        float Distance = FVector::Dist(Location, CurrentObj.TargetLocation);
        if (Distance <= CurrentObj.TargetRadius)
        {
            CompleteObjective(CurrentObj.ObjectiveID);
        }
    }
}

void UMissionSystem::RegisterInteraction(AActor* InteractedActor)
{
    if (!bHasActiveMission || !InteractedActor) return;

    FMissionObjective& CurrentObj = CurrentMission.Objectives[CurrentMission.CurrentObjectiveIndex];
    if (CurrentObj.Type == EObjectiveType::InteractWithObject)
    {
        if (CurrentObj.TargetActor == InteractedActor || (CurrentObj.TargetClass && InteractedActor->IsA(CurrentObj.TargetClass)))
        {
            CompleteObjective(CurrentObj.ObjectiveID);
        }
    }
}

FMissionData UMissionSystem::GetCurrentMission() const
{
    return CurrentMission;
}

FMissionObjective UMissionSystem::GetCurrentObjective() const
{
    if (CurrentMission.Objectives.IsValidIndex(CurrentMission.CurrentObjectiveIndex))
    {
        return CurrentMission.Objectives[CurrentMission.CurrentObjectiveIndex];
    }
    return FMissionObjective();
}

bool UMissionSystem::IsMissionActive(FName MissionID) const
{
    return bHasActiveMission && CurrentMission.MissionID == MissionID;
}

void UMissionSystem::BroadcastMissionUpdate()
{
    OnMissionUpdated.Broadcast(CurrentMission);
}