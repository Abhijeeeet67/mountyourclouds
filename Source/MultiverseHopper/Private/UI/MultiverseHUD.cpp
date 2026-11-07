#include "UI/MultiverseHUD.h"
#include "Character/BrainrotCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Kismet/KismetMathLibrary.h"

void UMultiverseHUD::NativeConstruct()
{
    Super::NativeConstruct();

    DamageIndicatorDuration = 0.5f;

    if (InteractionPromptBox)
    {
        InteractionPromptBox->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (DamageIndicator)
    {
        DamageIndicator->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UMultiverseHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (PlayerCharacter)
    {
        UpdateHealthBar();
        UpdateStaminaBar();
    }

    UpdateDamageIndicators(InDeltaTime);
}

void UMultiverseHUD::SetPlayerCharacter(ABrainrotCharacter* NewPlayer)
{
    PlayerCharacter = NewPlayer;

    if (PlayerCharacter)
    {
        UpdateHealthBar();
        UpdateStaminaBar();
    }
}

void UMultiverseHUD::UpdateHealth(float CurrentHealth, float MaxHealth)
{
    if (HealthBar)
    {
        HealthBar->SetPercent(MaxHealth > 0 ? CurrentHealth / MaxHealth : 0.0f);
    }
    if (HealthText)
    {
        HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurrentHealth, MaxHealth)));
    }
}

void UMultiverseHUD::UpdateStamina(float CurrentStamina, float MaxStamina)
{
    if (StaminaBar)
    {
        StaminaBar->SetPercent(MaxStamina > 0 ? CurrentStamina / MaxStamina : 0.0f);
    }
    if (StaminaText)
    {
        StaminaText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurrentStamina, MaxStamina)));
    }
}

void UMultiverseHUD::UpdateHealthBar()
{
    if (PlayerCharacter)
    {
        UpdateHealth(PlayerCharacter->CurrentHealth, PlayerCharacter->MaxHealth);
    }
}

void UMultiverseHUD::UpdateStaminaBar()
{
    if (PlayerCharacter)
    {
        UpdateStamina(PlayerCharacter->CurrentStamina, PlayerCharacter->MaxStamina);
    }
}

void UMultiverseHUD::SetObjectiveText(const FText& Objective)
{
    if (ObjectiveText)
    {
        ObjectiveText->SetText(Objective);
    }
    if (ObjectiveContainer)
    {
        ObjectiveContainer->SetVisibility(ESlateVisibility::Visible);
    }
}

void UMultiverseHUD::ShowInteractionPrompt(const FText& Prompt)
{
    if (InteractionPromptText)
    {
        InteractionPromptText->SetText(Prompt);
    }
    if (InteractionPromptBox)
    {
        InteractionPromptBox->SetVisibility(ESlateVisibility::Visible);
    }
}

void UMultiverseHUD::HideInteractionPrompt()
{
    if (InteractionPromptBox)
    {
        InteractionPromptBox->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UMultiverseHUD::ShowDamageIndicator(FVector WorldLocation)
{
    DamageIndicators.Add(WorldLocation);
    if (DamageIndicator)
    {
        DamageIndicator->SetVisibility(ESlateVisibility::Visible);
        DamageIndicator->SetRenderOpacity(1.0f);
    }
}

void UMultiverseHUD::UpdateDamageIndicators(float DeltaTime)
{
    if (DamageIndicators.Num() > 0 && DamageIndicator)
    {
        DamageIndicatorDuration -= DeltaTime;
        if (DamageIndicatorDuration <= 0.0f)
        {
            DamageIndicator->SetRenderOpacity(FMath::Max(0.0f, DamageIndicatorDuration + 0.5f));
            if (DamageIndicatorDuration <= -0.5f)
            {
                DamageIndicators.Empty();
                DamageIndicatorDuration = 0.5f;
                DamageIndicator->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
    }
}