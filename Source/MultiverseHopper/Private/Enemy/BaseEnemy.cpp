#include "Enemy/BaseEnemy.h"
#include "Perception/PawnSensingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "BrainrotCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"

ABaseEnemy::ABaseEnemy()
{
    PrimaryActorTick.bCanEverTick = true;

    PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
    PawnSensing->SightRadius = 1500.0f;
    PawnSensing->LoseSightRadius = 2000.0f;
    PawnSensing->PeripheralVisionAngle = 90.0f;
    PawnSensing->HearingThreshold = 600.0f;
    PawnSensing->LOSHearingThreshold = 1200.0f;
    PawnSensing->SetPeripheralVisionAngle(90.0f);

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = 200.0f;
    GetCharacterMovement()->JumpZVelocity = 400.0f;
    GetCharacterMovement()->AirControl = 0.2f;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

    EnemyType = EEnemyType::BasicMelee;
    SightRadius = 1500.0f;
    LoseSightRadius = 2000.0f;
    PeripheralVisionAngle = 90.0f;
    HearingRange = 600.0f;
    PatrolWaitTime = 3.0f;
    ChaseSpeed = 400.0f;
    PatrolSpeed = 200.0f;
    AttackRange = 200.0f;
    CombatRange = 500.0f;
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;

    EnemyState = EEnemyState::Idle;
    CurrentTarget = nullptr;
    CurrentPatrolIndex = 0;
    bIsAttacking = false;
    LastAttackTime = 0.0f;
    TeamId = FGenericTeamId(1);
}

void ABaseEnemy::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponents();
    InitializeAI();
    InitializeAttributes();

    CurrentHealth = MaxHealth;

    if (PawnSensing)
    {
        PawnSensing->OnSeePawn.AddDynamic(this, &ABaseEnemy::OnSeePawn);
        PawnSensing->OnHearNoise.AddDynamic(this, &ABaseEnemy::OnHearNoise);
    }

    if (PatrolPoints.Num() > 0)
    {
        SetEnemyState(EEnemyState::Patrol);
        MoveToPatrolPoint();
    }
}

void ABaseEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateAI(DeltaTime);
}

void ABaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ABaseEnemy::InitializeComponents()
{
}

void ABaseEnemy::InitializeAI()
{
    if (PawnSensing)
    {
        PawnSensing->SightRadius = SightRadius;
        PawnSensing->LoseSightRadius = LoseSightRadius;
        PawnSensing->PeripheralVisionAngle = PeripheralVisionAngle;
        PawnSensing->HearingThreshold = HearingRange;
    }

    GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
}

void ABaseEnemy::InitializeAttributes()
{
    CurrentHealth = MaxHealth;
}

void ABaseEnemy::UpdateAI(float DeltaTime)
{
    HandleStateTransitions();

    switch (EnemyState)
    {
        case EEnemyState::Patrol:
            break;
        case EEnemyState::Chasing:
            ChaseTarget();
            break;
        case EEnemyState::Attacking:
            CheckAttackConditions();
            break;
        case EEnemyState::Returning:
            break;
        default:
            break;
    }
}

void ABaseEnemy::HandleStateTransitions()
{
    if (EnemyState == EEnemyState::Dead || EnemyState == EEnemyState::HitReaction || EnemyState == EEnemyState::Attacking)
    {
        return;
    }

    float DistanceToTarget = 0.0f;
    if (CurrentTarget)
    {
        DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    }

    switch (EnemyState)
    {
        case EEnemyState::Idle:
            if (CurrentTarget && DistanceToTarget <= CombatRange)
            {
                SetEnemyState(EEnemyState::Chasing);
            }
            else if (PatrolPoints.Num() > 0)
            {
                SetEnemyState(EEnemyState::Patrol);
                MoveToPatrolPoint();
            }
            break;

        case EEnemyState::Patrol:
            if (CurrentTarget && DistanceToTarget <= CombatRange)
            {
                SetEnemyState(EEnemyState::Chasing);
            }
            break;

        case EEnemyState::Chasing:
            if (!CurrentTarget)
            {
                SetEnemyState(EEnemyState::Returning);
            }
            else if (DistanceToTarget > LoseSightRadius)
            {
                SetEnemyState(EEnemyState::Returning);
            }
            else if (DistanceToTarget <= AttackRange)
            {
                SetEnemyState(EEnemyState::Attacking);
            }
            break;

        case EEnemyState::Attacking:
            if (!CurrentTarget || DistanceToTarget > AttackRange * 1.5f)
            {
                SetEnemyState(EEnemyState::Chasing);
            }
            break;

        case EEnemyState::Returning:
            if (PatrolPoints.Num() > 0)
            {
                float DistanceToPatrol = FVector::Dist(GetActorLocation(), PatrolPoints[CurrentPatrolIndex]->GetActorLocation());
                if (DistanceToPatrol <= 100.0f)
                {
                    SetEnemyState(EEnemyState::Patrol);
                    GetWorldTimerManager().SetTimer(PatrolTimer, this, &ABaseEnemy::MoveToPatrolPoint, PatrolWaitTime, false);
                }
            }
            else
            {
                SetEnemyState(EEnemyState::Idle);
            }
            break;
    }
}

void ABaseEnemy::MoveToPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return;
    }

    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController && PatrolPoints[CurrentPatrolIndex])
    {
        AIController->MoveToActor(PatrolPoints[CurrentPatrolIndex], 50.0f);
    }
}

void ABaseEnemy::ChaseTarget()
{
    if (!CurrentTarget)
    {
        return;
    }

    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController)
    {
        GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
        AIController->MoveToActor(CurrentTarget, AttackRange * 0.8f);
    }
}

void ABaseEnemy::CheckAttackConditions()
{
    if (!CurrentTarget || bIsAttacking)
    {
        return;
    }

    float DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    if (DistanceToTarget <= AttackRange)
    {
        float TimeSinceLastAttack = GetWorld()->GetTimeSeconds() - LastAttackTime;
        if (Attacks.Num() > 0 && TimeSinceLastAttack >= Attacks[0].AttackCooldown)
        {
            PerformAttack(0);
        }
    }
}

void ABaseEnemy::OnSeePawn(APawn* SeenPawn)
{
    if (!SeenPawn || SeenPawn == this)
    {
        return;
    }

    ABrainrotCharacter* Player = Cast<ABrainrotCharacter>(SeenPawn);
    if (Player && Player->GetCombatState() != ECombatState::Dead)
    {
        if (!CurrentTarget)
        {
            if (SpotSound)
            {
                UGameplayStatics::PlaySoundAtLocation(this, SpotSound, GetActorLocation());
            }
        }
        SetTarget(SeenPawn);
    }
}

void ABaseEnemy::OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
    if (!NoiseInstigator || NoiseInstigator == this)
    {
        return;
    }

    ABrainrotCharacter* Player = Cast<ABrainrotCharacter>(NoiseInstigator);
    if (Player && Player->GetCombatState() != ECombatState::Dead)
    {
        if (!CurrentTarget)
        {
            SetTarget(NoiseInstigator);
            SetEnemyState(EEnemyState::Investigating);
        }
    }
}

void ABaseEnemy::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    if (CurrentTarget && EnemyState != EEnemyState::Dead)
    {
        SetEnemyState(EEnemyState::Chasing);
    }
}

AActor* ABaseEnemy::GetTarget() const
{
    return CurrentTarget;
}

void ABaseEnemy::SetPatrolPoints(const TArray<AActor*>& NewPatrolPoints)
{
    PatrolPoints = NewPatrolPoints;
    CurrentPatrolIndex = 0;
}

void ABaseEnemy::ReturnToPatrol()
{
    CurrentTarget = nullptr;
    SetEnemyState(EEnemyState::Returning);
}

void ABaseEnemy::PerformAttack(int32 AttackIndex)
{
    if (Attacks.Num() == 0 || AttackIndex >= Attacks.Num())
    {
        return;
    }

    const FEnemyAttackData& AttackData = Attacks[AttackIndex];
    if (!AttackData.AttackMontage)
    {
        return;
    }

    bIsAttacking = true;
    LastAttackTime = GetWorld()->GetTimeSeconds();

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Play(AttackData.AttackMontage);
    }

    if (AttackSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
    }

    FTimerDelegate CooldownDelegate;
    CooldownDelegate.BindLambda([this]()
    {
        bIsAttacking = false;
    });
    GetWorldTimerManager().SetTimer(AttackCooldownTimer, CooldownDelegate, Attacks[0].AttackCooldown, false);
}

void ABaseEnemy::OnAttackHitNotify()
{
    ApplyAttackDamage(0);
}

void ABaseEnemy::ApplyAttackDamage(int32 AttackIndex)
{
    if (Attacks.Num() == 0 || AttackIndex >= Attacks.Num())
    {
        return;
    }

    const FEnemyAttackData& AttackData = Attacks[AttackIndex];

    FVector Start = GetActorLocation() + GetActorForwardVector() * 50.0f + FVector(0, 0, 50.0f);
    FVector End = Start + GetActorForwardVector() * AttackData.AttackRange;

    TArray<FHitResult> HitResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.bReturnPhysicalMaterial = false;

    bool bHit = GetWorld()->SweepMultiByChannel(HitResults, Start, End, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(AttackData.AttackRange * 0.5f), QueryParams);

    for (const FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();
        if (HitActor && HitActor != this)
        {
            ABrainrotCharacter* Player = Cast<ABrainrotCharacter>(HitActor);
            if (Player)
            {
                FVector HitDirection = (HitActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
                Player->TakeDamage(AttackData.Damage, HitDirection, this);
            }
            else
            {
                FVector HitDirection = (HitActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
                UGameplayStatics::ApplyPointDamage(HitActor, AttackData.Damage, HitDirection, Hit, GetController(), this, nullptr);
            }

            if (HitEffect)
            {
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, Hit.Location, HitDirection.Rotation());
            }
        }
    }
}

void ABaseEnemy::SetEnemyState(EEnemyState NewState)
{
    if (EnemyState == NewState)
    {
        return;
    }

    EnemyState = NewState;

    switch (NewState)
    {
        case EEnemyState::Patrol:
            GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
            break;
        case EEnemyState::Chasing:
        case EEnemyState::Investigating:
            GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
            break;
        case EEnemyState::Attacking:
            GetCharacterMovement()->MaxWalkSpeed = 0.0f;
            break;
        case EEnemyState::Returning:
            GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
            break;
        case EEnemyState::Dead:
            GetCharacterMovement()->DisableMovement();
            GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            OnDeath();
            break;
        default:
            break;
    }
}

void ABaseEnemy::Heal(float Amount)
{
    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Amount);
}

float ABaseEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (EnemyState == EEnemyState::Dead)
    {
        return 0.0f;
    }

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    if (HitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
    }

    if (HitEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, GetActorLocation());
    }

    if (DamageCauser)
    {
        FVector HitDirection = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
        LaunchCharacter(HitDirection * 200.0f + FVector(0, 0, 100.0f), true, true);
    }

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    else
    {
        SetEnemyState(EEnemyState::HitReaction);
        GetWorldTimerManager().SetTimer(AttackCooldownTimer, [this]()
        {
            if (EnemyState == EEnemyState::HitReaction)
            {
                if (CurrentTarget)
                {
                    SetEnemyState(EEnemyState::Chasing);
                }
                else
                {
                    SetEnemyState(EEnemyState::Returning);
                }
            }
        }, 0.5f, false);
    }

    return ActualDamage;
}

void ABaseEnemy::Die()
{
    if (EnemyState == EEnemyState::Dead)
    {
        return;
    }

    SetEnemyState(EEnemyState::Dead);

    if (DeathSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
    }

    if (DeathEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DeathEffect, GetActorLocation());
    }

    GetWorldTimerManager().SetTimer(AttackCooldownTimer, [this]()
    {
        Destroy();
    }, 5.0f, false);
}

void ABaseEnemy::OnDeath()
{
}

FGenericTeamId ABaseEnemy::GetGenericTeamId() const
{
    return TeamId;
}