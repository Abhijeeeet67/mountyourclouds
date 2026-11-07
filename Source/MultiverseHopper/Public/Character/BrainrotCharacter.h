#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "BrainrotCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UAnimMontage;
class UInputAction;
class UInputMappingContext;
class UAttributeSet;
class UAbilitySystemComponent;
class AStaffWeapon;
class UCurveFloat;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle,
    Attacking,
    ComboWindow,
    HeavyAttack,
    Dodging,
    Rolling,
    HitReaction,
    Dead
};

UENUM(BlueprintType)
enum class EMovementState : uint8
{
    Idle,
    Walking,
    Sprinting,
    Crouching,
    Jumping,
    Falling,
    Landing
};

USTRUCT(BlueprintType)
struct FComboAttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Damage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float KnockbackForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FName HitNotifyName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ComboWindowStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ComboWindowEnd;
};

UCLASS()
class MULTIVERSEHOPPER_API ABrainrotCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ABrainrotCharacter();

    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void LightAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void HeavyAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Dodge();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Roll();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage(float DamageAmount, FVector HitDirection, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnAttackHitNotify();

    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void Die();

    UFUNCTION(BlueprintCallable, Category = "Health")
    void Respawn();

    UFUNCTION(BlueprintPure, Category = "State")
    float GetHealthPercent() const { return MaxHealth > 0 ? CurrentHealth / MaxHealth : 0.0f; }

    UFUNCTION(BlueprintPure, Category = "State")
    ECombatState GetCombatState() const { return CombatState; }

    UFUNCTION(BlueprintPure, Category = "State")
    EMovementState GetMovementState() const { return MovementState; }

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraMode(bool bOverShoulder);

protected:
    virtual void PossessedBy(AController* NewController) override;

private:
    void InitializeComponents();
    void InitializeAttributes();
    void InitializeInput();
    void InitializeCombat();
    void InitializeCamera();

    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void SprintPressed();
    void SprintReleased();
    void CrouchPressed();
    void CrouchReleased();

    void UpdateMovementState();
    void UpdateCombatState();
    void UpdateCamera(float DeltaTime);

    void PerformLightAttack();
    void PerformHeavyAttack();
    void PerformDodge();
    void PerformRoll();
    void CheckComboInput();
    void ResetCombo();
    void OnComboWindowOpen();
    void OnComboWindowClose();
    void ApplyAttackDamage(int32 AttackIndex);
    void SpawnHitEffect(FVector Location, FVector Normal);

    void OnLanded(const FHitResult& Hit);
    void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode);

    UFUNCTION()
    void OnHealthChanged(float OldValue, float NewValue);

    UFUNCTION()
    void OnStaminaChanged(float OldValue, float NewValue);

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    TArray<FComboAttackData> LightAttackCombo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    FComboAttackData HeavyAttackData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float DodgeDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float DodgeInvincibilityTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float RollDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float RollInvincibilityTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float StaminaCostLightAttack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float StaminaCostHeavyAttack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float StaminaCostDodge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float StaminaCostRoll;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
    float MaxStamina;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
    float CurrentStamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
    float StaminaRegenRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
    float StaminaRegenDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float SprintSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float CrouchSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float RotationRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    float CameraDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    float CameraHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    float CameraLagSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    UNiagaraSystem* HitEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    UNiagaraSystem* DodgeEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    UNiagaraSystem* RollEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    USoundBase* SwingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    USoundBase* HitSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    USoundBase* DodgeSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    USoundBase* HurtSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    USoundBase* DeathSound;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    ECombatState CombatState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    EMovementState MovementState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    int32 CurrentComboIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    bool bComboInputQueued;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsInvincible;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float LastStaminaUseTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    AStaffWeapon* EquippedStaff;

    UPROPERTY()
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY()
    UInputAction* MoveAction;

    UPROPERTY()
    UInputAction* LookAction;

    UPROPERTY()
    UInputAction* JumpAction;

    UPROPERTY()
    UInputAction* SprintAction;

    UPROPERTY()
    UInputAction* CrouchAction;

    UPROPERTY()
    UInputAction* LightAttackAction;

    UPROPERTY()
    UInputAction* HeavyAttackAction;

    UPROPERTY()
    UInputAction* DodgeAction;

    UPROPERTY()
    UInputAction* RollAction;

    UPROPERTY()
    UInputAction* InteractAction;

    UPROPERTY()
    UInputAction* PauseAction;

    FTimerHandle ComboWindowTimer;
    FTimerHandle StaminaRegenTimer;
    FTimerHandle InvincibilityTimer;
    FVector LastMovementInput;
};