#include "UI/GameOverWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/MultiverseGameModeBase.h"
#include "Character/BrainrotCharacter.h"

void UGameOverWidget::NativeConstruct()
{
    Super::NativeConstruct();

    GameMode = Cast<AMultiverseGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

    SetupButtons();

    if (GameOverText)
    {
        GameOverText->SetText(FText::FromString(TEXT("YOU HAVE FALLEN")));
    }

    if (StatsText && GameMode)
    {
        ABrainrotCharacter* Player = Cast<ABrainrotCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
        if (Player)
        {
            StatsText->SetText(FText::FromString(FString::Printf(TEXT("Enemies Defeated: %d\nTime Survived: %.1fs"), 0, 0.0f)));
        }
    }
}

void UGameOverWidget::SetupButtons()
{
    if (RetryButton)
    {
        RetryButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnRetryClicked);
    }
    if (MainMenuButton)
    {
        MainMenuButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnMainMenuClicked);
    }
    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnQuitClicked);
    }
}

void UGameOverWidget::OnRetryClicked()
{
    if (GameMode)
    {
        GameMode->StartMission(GameMode->CurrentMissionID);
    }
}

void UGameOverWidget::OnMainMenuClicked()
{
    if (GameMode)
    {
        GameMode->SetGameState(EGameState::MainMenu);
    }
}

void UGameOverWidget::OnQuitClicked()
{
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        PC->ConsoleCommand(TEXT("quit"));
    }
}