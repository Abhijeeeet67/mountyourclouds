#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "BaseEnemy.generated.h"

class UBehaviorTree;
class UBlackboardData;
class UPawnSensingComponent;
class UAttributeSet;
class UAbilitySystemComponent;
class UAnimMontage;
class UNiagaraSystem;
class USoundBase;
class AActor;

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
    Idle,
    Patrol,
    Investigating,
    Chasing,
    Attacking,
    HitReaction,
    Dead,
    Returning
};

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
    BasicMelee,
    HeavyMelee,
    Ranged,
    Elite,
    Boss
};

USTRUCT(BlueprintType)
struct FEnemyAttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Damage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float WindupTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FName HitNotifyName;
};

UCLASS()
class MULTIVERSEHOPPER_API ABaseEnemy : public ACharacter, public IGenericTeamAgentInterface
{
    GENERATED_BODY()

public:
    ABaseEnemy();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintCallable, Category = "Enemy|AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Enemy|AI")
    AActor* GetTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Enemy|AI")
    void SetPatrolPoints(const TArray<AActor*>& NewPatrolPoints);

    UFUNCTION(BlueprintCallable, Category = "Enemy|AI")
    void ReturnToPatrol();

    UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
    void PerformAttack(int32 AttackIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
    void OnAttackHitNotify();

    UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
    void ApplyAttackDamage(int32 AttackIndex);

    UFUNCTION(BlueprintCallable, Category = "Enemy|State")
    void SetEnemyState(EEnemyState NewState);

    UFUNCTION(BlueprintCallable, Category = "Enemy|State")
    EEnemyState GetEnemyState() const { return EnemyState; }

    UFUNCTION(BlueprintCallable, Category = "Enemy|Health")
    void Heal(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Enemy|Health")
    void Die();

    UFUNCTION(BlueprintPure, Category = "Enemy|Health")
    float GetHealthPercent() const { return MaxHealth > 0 ? CurrentHealth / MaxHealth : 0.0f; }

    virtual FGenericTeamId GetGenericTeamId() const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Type")
    EEnemyType EnemyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
    UBlackboardData* BlackboardAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
    float LoseSightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
    float PeripheralVisionAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
    float HearingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
    float PatrolWaitTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
    float ChaseSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
    float CombatRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
    TArray<FEnemyAttackData> Attacks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Health")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Health")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|VFX")
    UNiagaraSystem* HitEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|VFX")
    UNiagaraSystem* DeathEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Audio")
    USoundBase* HitSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Audio")
    USoundBase* DeathSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Audio")
    USoundBase* AttackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Audio")
    USoundBase* SpotSound;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
    UPawnSensingComponent* PawnSensing;

    UPROPERTY()
    AActor* CurrentTarget;

    UPROPERTY()
    TArray<AActor*> PatrolPoints;

    UPROPERTY()
    int32 CurrentPatrolIndex;

    UPROPERTY()
    EEnemyState EnemyState;

    UPROPERTY()
    bool bIsAttacking;

    UPROPERTY()
    float LastAttackTime;

    UPROPERTY()
    FGenericTeamId TeamId;

    UFUNCTION()
    void OnSeePawn(APawn* SeenPawn);

    UFUNCTION()
    void OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume);

    void InitializeComponents();
    void InitializeAI();
    void InitializeAttributes();
    void UpdateAI(float DeltaTime);
    void HandleStateTransitions();
    void MoveToPatrolPoint();
    void ChaseTarget();
    void CheckAttackConditions();
    void OnDeath();

    FTimerHandle PatrolTimer;
    FTimerHandle AttackCooldownTimer;
};