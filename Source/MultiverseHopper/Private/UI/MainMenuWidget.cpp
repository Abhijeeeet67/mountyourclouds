#include "UI/MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/MultiverseGameModeBase.h"

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    GameMode = Cast<AMultiverseGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

    SetupButtons();

    if (GameTitleText)
    {
        GameTitleText->SetText(FText::FromString(TEXT("BRAINROT WARS")));
    }
    if (SubtitleText)
    {
        SubtitleText->SetText(FText::FromString(TEXT("THE LAST HEARTBEAT")));
    }

    ShowMainMenu();
}

void UMainMenuWidget::SetupButtons()
{
    if (StartGameButton)
    {
        StartGameButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnStartGameClicked);
    }
    if (ContinueButton)
    {
        ContinueButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnContinueClicked);
    }
    if (SettingsButton)
    {
        SettingsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSettingsClicked);
    }
    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
    }

    if (ContinueButton)
    {
        bool bHasSave = false;
        if (GameMode)
        {
            if (UMultiverseGameInstance* GI = Cast<UMultiverseGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
            {
                bHasSave = GI->DoesSaveExist(0);
            }
        }
        ContinueButton->SetVisibility(bHasSave ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UMainMenuWidget::OnStartGameClicked()
{
    if (GameMode)
    {
        GameMode->StartMission(GameMode->StartingMissionID);
    }
}

void UMainMenuWidget::OnContinueClicked()
{
    if (GameMode)
    {
        if (UMultiverseGameInstance* GI = Cast<UMultiverseGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
        {
            GI->LoadGame(0);
            GameMode->StartMission(GI->CurrentDimensionID);
        }
    }
}

void UMainMenuWidget::OnSettingsClicked()
{
    ShowSettings();
}

void UMainMenuWidget::OnQuitClicked()
{
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        PC->ConsoleCommand(TEXT("quit"));
    }
}

void UMainMenuWidget::ShowMainMenu()
{
    if (MainMenuContainer)
    {
        MainMenuContainer->SetVisibility(ESlateVisibility::Visible);
    }
    if (SettingsContainer)
    {
        SettingsContainer->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UMainMenuWidget::ShowSettings()
{
    if (MainMenuContainer)
    {
        MainMenuContainer->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (SettingsContainer)
    {
        SettingsContainer->SetVisibility(ESlateVisibility::Visible);
    }
}