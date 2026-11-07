#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "OriginKingdomLevelSetup.generated.h"

class UButton;
class UTextBlock;
class UProgressBar;
class UEditableTextBox;
class UComboBoxString;
class UCheckBox;

UCLASS()
class MULTIVERSEHOPPER_API UOriginKingdomLevelSetup : public UEditorUtilityWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "Level Setup")
    void GenerateMission001();

    UFUNCTION(BlueprintCallable, Category = "Level Setup")
    void ClearLevel();

    UFUNCTION(BlueprintCallable, Category = "Level Setup")
    void SetupLightingOnly();

    UFUNCTION(BlueprintCallable, Category = "Level Setup")
    void SetupNavigationOnly();

    UFUNCTION(BlueprintCallable, Category = "Level Setup")
    void SpawnMissionActors();

protected:
    UPROPERTY(meta = (BindWidget))
    UButton* GenerateButton;

    UPROPERTY(meta = (BindWidget))
    UButton* ClearButton;

    UPROPERTY(meta = (BindWidget))
    UButton* LightingButton;

    UPROPERTY(meta = (BindWidget))
    UButton* NavigationButton;

    UPROPERTY(meta = (BindWidget))
    UButton* SpawnActorsButton;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* ProgressBar;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* StatusText;

    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* LevelNameBox;

    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* SeedBox;

    UPROPERTY(meta = (BindWidget))
    UComboBoxString* QualityPresetCombo;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* GenerateFoliageCheck;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* GenerateStructuresCheck;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* GenerateEnemiesCheck;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* GenerateMissionCheck;

    void UpdateStatus(const FString& Message, float Progress);
    void SetupDefaultValues();
};