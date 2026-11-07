#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "MultiverseHUD.generated.h"

class ABrainrotCharacter;

UCLASS()
class MULTIVERSEHOPPER_API UMultiverseHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetPlayerCharacter(ABrainrotCharacter* NewPlayer);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateHealth(float CurrentHealth, float MaxHealth);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateStamina(float CurrentStamina, float MaxStamina);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetObjectiveText(const FText& Objective);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowInteractionPrompt(const FText& Prompt);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void HideInteractionPrompt();

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowDamageIndicator(FVector WorldLocation);

protected:
    UPROPERTY(meta = (BindWidget))
    UProgressBar* HealthBar;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* StaminaBar;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* HealthText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* StaminaText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ObjectiveText;

    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* InteractionPromptBox;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* InteractionPromptText;

    UPROPERTY(meta = (BindWidget))
    UImage* DamageIndicator;

    UPROPERTY(meta = (BindWidget))
    UVerticalBox* ObjectiveContainer;

    UPROPERTY()
    ABrainrotCharacter* PlayerCharacter;

    UPROPERTY()
    TArray<FVector> DamageIndicators;

    UPROPERTY()
    float DamageIndicatorDuration;

    void UpdateHealthBar();
    void UpdateStaminaBar();
    void UpdateDamageIndicators(float DeltaTime);
};