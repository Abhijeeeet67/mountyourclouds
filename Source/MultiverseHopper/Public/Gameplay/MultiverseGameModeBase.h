#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MultiverseGameModeBase.generated.h"

class ABrainrotCharacter;
class APlayerController;
class UUserWidget;

UENUM(BlueprintType)
enum class EGameState : uint8
{
    MainMenu,
    Playing,
    Paused,
    GameOver,
    MissionComplete,
    Transitioning
};

UCLASS()
class MULTIVERSEHOPPER_API AMultiverseGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMultiverseGameModeBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Game|State")
    void SetGameState(EGameState NewState);

    UFUNCTION(BlueprintCallable, Category = "Game|State")
    EGameState GetGameState() const { return CurrentGameState; }

    UFUNCTION(BlueprintCallable, Category = "Game|Mission")
    void StartMission(FName MissionID);

    UFUNCTION(BlueprintCallable, Category = "Game|Mission")
    void CompleteMission();

    UFUNCTION(BlueprintCallable, Category = "Game|Mission")
    void FailMission();

    UFUNCTION(BlueprintCallable, Category = "Game|Player")
    void OnPlayerDeath(ABrainrotCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Game|Player")
    void OnPlayerRespawn(ABrainrotCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Game|UI")
    void ShowMainMenu();

    UFUNCTION(BlueprintCallable, Category = "Game|UI")
    void HideMainMenu();

    UFUNCTION(BlueprintCallable, Category = "Game|UI")
    void ShowPauseMenu();

    UFUNCTION(BlueprintCallable, Category = "Game|UI")
    void HidePauseMenu();

    UFUNCTION(BlueprintCallable, Category = "Game|UI")
    void ShowGameOver();

    UFUNCTION(BlueprintCallable, Category = "Game|UI")
    void ShowMissionComplete();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game|Classes")
    TSubclassOf<ABrainrotCharacter> PlayerCharacterClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game|Classes")
    TSubclassOf<APlayerController> PlayerControllerClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game|UI")
    TSubclassOf<UUserWidget> MainMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game|UI")
    TSubclassOf<UUserWidget> HUDWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game|UI")
    TSubclassOf<UUserWidget> PauseMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game|UI")
    TSubclassOf<UUserWidget> GameOverWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game|UI")
    TSubclassOf<UUserWidget> MissionCompleteWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game|Mission")
    FName StartingMissionID;

protected:
    UPROPERTY()
    EGameState CurrentGameState;

    UPROPERTY()
    UUserWidget* MainMenuWidget;

    UPROPERTY()
    UUserWidget* HUDWidget;

    UPROPERTY()
    UUserWidget* PauseMenuWidget;

    UPROPERTY()
    UUserWidget* GameOverWidget;

    UPROPERTY()
    UUserWidget* MissionCompleteWidget;

    UPROPERTY()
    FName CurrentMissionID;

    UPROPERTY()
    ABrainrotCharacter* PlayerCharacter;

    void InitializeWidgets();
    void CreateWidgetIfNeeded(TSubclassOf<UUserWidget> WidgetClass, UUserWidget*& WidgetRef);
    void UpdateHUD();
};