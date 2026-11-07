#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "MissionCompleteWidget.generated.h"

class AMultiverseGameModeBase;

UCLASS()
class MULTIVERSEHOPPER_API UMissionCompleteWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnContinueClicked();

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnMainMenuClicked();

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnReplayClicked();

protected:
    UPROPERTY(meta = (BindWidget))
    UButton* ContinueButton;

    UPROPERTY(meta = (BindWidget))
    UButton* MainMenuButton;

    UPROPERTY(meta = (BindWidget))
    UButton* ReplayButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* MissionTitleText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* CompletionText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* StatsText;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* CompletionBar;

    UPROPERTY()
    AMultiverseGameModeBase* GameMode;

    void SetupButtons();
    void UpdateStats();
};