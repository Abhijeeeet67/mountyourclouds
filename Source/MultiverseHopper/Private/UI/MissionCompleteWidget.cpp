#include "UI/MissionCompleteWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/MultiverseGameModeBase.h"
#include "Core/MultiverseGameInstance.h"
#include "Gameplay/MissionSystem.h"

void UMissionCompleteWidget::NativeConstruct()
{
    Super::NativeConstruct();

    GameMode = Cast<AMultiverseGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

    SetupButtons();
    UpdateStats();

    if (MissionTitleText)
    {
        MissionTitleText->SetText(FText::FromString(TEXT("MISSION COMPLETE")));
    }
    if (CompletionText)
    {
        CompletionText->SetText(FText::FromString(TEXT("THE FIRST THREAT HAS BEEN ELIMINATED")));
    }
    if (CompletionBar)
    {
        CompletionBar->SetPercent(1.0f);
    }
}

void UMissionCompleteWidget::SetupButtons()
{
    if (ContinueButton)
    {
        ContinueButton->OnClicked.AddDynamic(this, &UMissionCompleteWidget::OnContinueClicked);
    }
    if (MainMenuButton)
    {
        MainMenuButton->OnClicked.AddDynamic(this, &UMissionCompleteWidget::OnMainMenuClicked);
    }
    if (ReplayButton)
    {
        ReplayButton->OnClicked.AddDynamic(this, &UMissionCompleteWidget::OnReplayClicked);
    }
}

void UMissionCompleteWidget::UpdateStats()
{
    if (StatsText && GameMode)
    {
        StatsText->SetText(FText::FromString(FString::Printf(TEXT("Enemies Defeated: 6\nTime: %.1fs\nDamage Taken: %.0f"), 300.0f, 25.0f)));
    }
}

void UMissionCompleteWidget::OnContinueClicked()
{
    if (GameMode)
    {
        if (UMultiverseGameInstance* GI = Cast<UMultiverseGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
        {
            GI->UpdateDimensionProgress(TEXT("Dimension_Void"), EDimensionState::Unlocked);
        }
        GameMode->SetGameState(EGameState::MainMenu);
    }
}

void UMissionCompleteWidget::OnMainMenuClicked()
{
    if (GameMode)
    {
        GameMode->SetGameState(EGameState::MainMenu);
    }
}

void UMissionCompleteWidget::OnReplayClicked()
{
    if (GameMode)
    {
        GameMode->StartMission(GameMode->CurrentMissionID);
    }
}