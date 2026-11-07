#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "PauseMenuWidget.generated.h"

class AMultiverseGameModeBase;

UCLASS()
class MULTIVERSEHOPPER_API UPauseMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnResumeClicked();

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnRestartClicked();

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnSettingsClicked();

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnMainMenuClicked();

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnQuitClicked();

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void OnMasterVolumeChanged(float Value);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void OnMusicVolumeChanged(float Value);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void OnSFXVolumeChanged(float Value);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void OnGraphicsQualityChanged(int32 QualityIndex);

protected:
    UPROPERTY(meta = (BindWidget))
    UButton* ResumeButton;

    UPROPERTY(meta = (BindWidget))
    UButton* RestartButton;

    UPROPERTY(meta = (BindWidget))
    UButton* SettingsButton;

    UPROPERTY(meta = (BindWidget))
    UButton* MainMenuButton;

    UPROPERTY(meta = (BindWidget))
    UButton* QuitButton;

    UPROPERTY(meta = (BindWidget))
    UButton* BackButton;

    UPROPERTY(meta = (BindWidget))
    USlider* MasterVolumeSlider;

    UPROPERTY(meta = (BindWidget))
    USlider* MusicVolumeSlider;

    UPROPERTY(meta = (BindWidget))
    USlider* SFXVolumeSlider;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* MasterVolumeText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* MusicVolumeText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* SFXVolumeText;

    UPROPERTY(meta = (BindWidget))
    UVerticalBox* PauseMenuContainer;

    UPROPERTY(meta = (BindWidget))
    UVerticalBox* SettingsContainer;

    UPROPERTY()
    AMultiverseGameModeBase* GameMode;

    void SetupButtons();
    void LoadSettings();
    void ShowPauseMenu();
    void ShowSettings();
};