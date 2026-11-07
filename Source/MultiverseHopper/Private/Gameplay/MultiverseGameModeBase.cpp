#include "Gameplay/MultiverseGameModeBase.h"
#include "Character/BrainrotCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"

AMultiverseGameModeBase::AMultiverseGameModeBase()
{
    PrimaryActorTick.bCanEverTick = true;

    DefaultPawnClass = nullptr;
    PlayerControllerClass = APlayerController::StaticClass();
    CurrentGameState = EGameState::MainMenu;
    StartingMissionID = TEXT("Mission_001_FirstThreat");
}

void AMultiverseGameModeBase::BeginPlay()
{
    Super::BeginPlay();
    InitializeWidgets();
    ShowMainMenu();
}

void AMultiverseGameModeBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateHUD();
}

void AMultiverseGameModeBase::InitializeWidgets()
{
    CreateWidgetIfNeeded(MainMenuWidgetClass, MainMenuWidget);
    CreateWidgetIfNeeded(HUDWidgetClass, HUDWidget);
    CreateWidgetIfNeeded(PauseMenuWidgetClass, PauseMenuWidget);
    CreateWidgetIfNeeded(GameOverWidgetClass, GameOverWidget);
    CreateWidgetIfNeeded(MissionCompleteWidgetClass, MissionCompleteWidget);
}

void AMultiverseGameModeBase::CreateWidgetIfNeeded(TSubclassOf<UUserWidget> WidgetClass, UUserWidget*& WidgetRef)
{
    if (WidgetClass && !WidgetRef)
    {
        WidgetRef = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
        if (WidgetRef)
        {
            WidgetRef->AddToViewport();
            WidgetRef->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void AMultiverseGameModeBase::SetGameState(EGameState NewState)
{
    if (CurrentGameState == NewState)
    {
        return;
    }

    CurrentGameState = NewState;

    switch (NewState)
    {
        case EGameState::MainMenu:
            ShowMainMenu();
            break;
        case EGameState::Playing:
            HideMainMenu();
            HidePauseMenu();
            if (HUDWidget)
            {
                HUDWidget->SetVisibility(ESlateVisibility::Visible);
            }
            break;
        case EGameState::Paused:
            ShowPauseMenu();
            break;
        case EGameState::GameOver:
            ShowGameOver();
            break;
        case EGameState::MissionComplete:
            ShowMissionComplete();
            break;
        case EGameState::Transitioning:
            break;
    }
}

void AMultiverseGameModeBase::StartMission(FName MissionID)
{
    CurrentMissionID = MissionID;
    SetGameState(EGameState::Playing);

    if (PlayerCharacterClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
        if (PC)
        {
            AActor* PlayerStart = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass());
            FTransform SpawnTransform = PlayerStart ? PlayerStart->GetActorTransform() : FTransform::Identity;

            PlayerCharacter = GetWorld()->SpawnActor<ABrainrotCharacter>(PlayerCharacterClass, SpawnTransform, SpawnParams);
            if (PlayerCharacter)
            {
                PC->Possess(PlayerCharacter);
                PC->SetInputMode(FInputModeGameOnly());
                PC->bShowMouseCursor = false;
            }
        }
    }
}

void AMultiverseGameModeBase::CompleteMission()
{
    SetGameState(EGameState::MissionComplete);

    if (UMultiverseGameInstance* GameInstance = Cast<UMultiverseGameInstance>(GetGameInstance()))
    {
        GameInstance->UpdateDimensionProgress(CurrentMissionID, EDimensionState::Completed, 100.0f);
    }
}

void AMultiverseGameModeBase::FailMission()
{
    SetGameState(EGameState::GameOver);
}

void AMultiverseGameModeBase::OnPlayerDeath(ABrainrotCharacter* Player)
{
    if (Player == PlayerCharacter)
    {
        GetWorldTimerManager().SetTimerForNextTick(this, [this]()
        {
            SetGameState(EGameState::GameOver);
        });
    }
}

void AMultiverseGameModeBase::OnPlayerRespawn(ABrainrotCharacter* Player)
{
    if (Player == PlayerCharacter && CurrentGameState == EGameState::GameOver)
    {
        SetGameState(EGameState::Playing);
    }
}

void AMultiverseGameModeBase::ShowMainMenu()
{
    if (MainMenuWidget)
    {
        MainMenuWidget->SetVisibility(ESlateVisibility::Visible);
    }
    if (HUDWidget)
    {
        HUDWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (PauseMenuWidget)
    {
        PauseMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (GameOverWidget)
    {
        GameOverWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (MissionCompleteWidget)
    {
        MissionCompleteWidget->SetVisibility(ESlateVisibility::Collapsed);
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        PC->SetInputMode(FInputModeUIOnly());
        PC->bShowMouseCursor = true;
    }
}

void AMultiverseGameModeBase::HideMainMenu()
{
    if (MainMenuWidget)
    {
        MainMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void AMultiverseGameModeBase::ShowPauseMenu()
{
    if (PauseMenuWidget)
    {
        PauseMenuWidget->SetVisibility(ESlateVisibility::Visible);
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        PC->SetInputMode(FInputModeUIOnly());
        PC->bShowMouseCursor = true;
        PC->SetPause(true);
    }
}

void AMultiverseGameModeBase::HidePauseMenu()
{
    if (PauseMenuWidget)
    {
        PauseMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
        PC->SetPause(false);
    }
}

void AMultiverseGameModeBase::ShowGameOver()
{
    if (GameOverWidget)
    {
        GameOverWidget->SetVisibility(ESlateVisibility::Visible);
    }
    if (HUDWidget)
    {
        HUDWidget->SetVisibility(ESlateVisibility::Collapsed);
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        PC->SetInputMode(FInputModeUIOnly());
        PC->bShowMouseCursor = true;
    }
}

void AMultiverseGameModeBase::ShowMissionComplete()
{
    if (MissionCompleteWidget)
    {
        MissionCompleteWidget->SetVisibility(ESlateVisibility::Visible);
    }
    if (HUDWidget)
    {
        HUDWidget->SetVisibility(ESlateVisibility::Collapsed);
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        PC->SetInputMode(FInputModeUIOnly());
        PC->bShowMouseCursor = true;
    }
}

void AMultiverseGameModeBase::UpdateHUD()
{
}