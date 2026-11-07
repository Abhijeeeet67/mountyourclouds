#include "UI/PauseMenuWidget.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/MultiverseGameMode.h"
#include "Core/MultiverseGameInstance.h"

void UPauseMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    GameMode = Cast<AMultiverseGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

    SetupButtons();
    LoadSettings();
    ShowPauseMenu();
}

void UPauseMenuWidget::SetupButtons()
{
    if (ResumeButton)
    {
        ResumeButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
    }
    if (RestartButton)
    {
        RestartButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnRestartClicked);
    }
    if (SettingsButton)
    {
        SettingsButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSettingsClicked);
    }
    if (MainMenuButton)
    {
        MainMenuButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnMainMenuClicked);
    }
    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnQuitClicked);
    }
    if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::ShowPauseMenu);
    }

    if (MasterVolumeSlider)
    {
        MasterVolumeSlider->OnValueChanged.AddDynamic(this, &UPauseMenuWidget::OnMasterVolumeChanged);
    }
    if (MusicVolumeSlider)
    {
        MusicVolumeSlider->OnValueChanged.AddDynamic(this, &UPauseMenuWidget::OnMusicVolumeChanged);
    }
    if (SFXVolumeSlider)
    {
        SFXVolumeSlider->OnValueChanged.AddDynamic(this, &UPauseMenuWidget::OnSFXVolumeChanged);
    }
}

void UPauseMenuWidget::LoadSettings()
{
    if (UMultiverseGameInstance* GI = Cast<UMultiverseGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
    {
        if (MasterVolumeSlider)
        {
            MasterVolumeSlider->SetValue(GI->GetMasterVolume());
        }
        if (MasterVolumeText)
        {
            MasterVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), GI->GetMasterVolume() * 100)));
        }
    }
}

void UPauseMenuWidget::OnResumeClicked()
{
    if (GameMode)
    {
        GameMode->SetGameState(EGameState::Playing);
    }
}

void UPauseMenuWidget::OnRestartClicked()
{
    if (GameMode)
    {
        GameMode->StartMission(GameMode->CurrentMissionID);
    }
}

void UPauseMenuWidget::OnSettingsClicked()
{
    ShowSettings();
}

void UPauseMenuWidget::OnMainMenuClicked()
{
    if (GameMode)
    {
        GameMode->SetGameState(EGameState::MainMenu);
    }
}

void UPauseMenuWidget::OnQuitClicked()
{
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        PC->ConsoleCommand(TEXT("quit"));
    }
}

void UPauseMenuWidget::OnMasterVolumeChanged(float Value)
{
    if (UMultiverseGameInstance* GI = Cast<UMultiverseGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
    {
        GI->SetMasterVolume(Value);
    }
    if (MasterVolumeText)
    {
        MasterVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), Value * 100)));
    }
}

void UPauseMenuWidget::OnMusicVolumeChanged(float Value)
{
    if (MusicVolumeText)
    {
        MusicVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), Value * 100)));
    }
}

void UPauseMenuWidget::OnSFXVolumeChanged(float Value)
{
    if (SFXVolumeText)
    {
        SFXVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), Value * 100)));
    }
}

void UPauseMenuWidget::OnGraphicsQualityChanged(int32 QualityIndex)
{
    if (UMultiverseGameInstance* GI = Cast<UMultiverseGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
    {
        GI->SetGraphicsQuality(QualityIndex);
    }
}

void UPauseMenuWidget::ShowPauseMenu()
{
    if (PauseMenuContainer)
    {
        PauseMenuContainer->SetVisibility(ESlateVisibility::Visible);
    }
    if (SettingsContainer)
    {
        SettingsContainer->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UPauseMenuWidget::ShowSettings()
{
    if (PauseMenuContainer)
    {
        PauseMenuContainer->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (SettingsContainer)
    {
        SettingsContainer->SetVisibility(ESlateVisibility::Visible);
    }
}