#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameOverWidget.generated.h"

class AMultiverseGameModeBase;

UCLASS()
class MULTIVERSEHOPPER_API UGameOverWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnRetryClicked();

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnMainMenuClicked();

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnQuitClicked();

protected:
    UPROPERTY(meta = (BindWidget))
    UButton* RetryButton;

    UPROPERTY(meta = (BindWidget))
    UButton* MainMenuButton;

    UPROPERTY(meta = (BindWidget))
    UButton* QuitButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* GameOverText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* StatsText;

    UPROPERTY()
    AMultiverseGameModeBase* GameMode;

    void SetupButtons();
};