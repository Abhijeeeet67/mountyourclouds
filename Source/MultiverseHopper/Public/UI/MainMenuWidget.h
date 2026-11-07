#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "MainMenuWidget.generated.h"

class AMultiverseGameModeBase;

UCLASS()
class MULTIVERSEHOPPER_API UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnStartGameClicked();

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnSettingsClicked();

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnQuitClicked();

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnContinueClicked();

protected:
    UPROPERTY(meta = (BindWidget))
    UButton* StartGameButton;

    UPROPERTY(meta = (BindWidget))
    UButton* ContinueButton;

    UPROPERTY(meta = (BindWidget))
    UButton* SettingsButton;

    UPROPERTY(meta = (BindWidget))
    UButton* QuitButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* GameTitleText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* SubtitleText;

    UPROPERTY(meta = (BindWidget))
    UImage* BackgroundImage;

    UPROPERTY(meta = (BindWidget))
    UVerticalBox* MainMenuContainer;

    UPROPERTY(meta = (BindWidget))
    UVerticalBox* SettingsContainer;

    UPROPERTY()
    AMultiverseGameModeBase* GameMode;

    void SetupButtons();
    void ShowMainMenu();
    void ShowSettings();
};