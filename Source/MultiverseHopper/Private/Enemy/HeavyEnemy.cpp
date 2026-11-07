#include "Enemy/HeavyEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "Character/BrainrotCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

AHeavyEnemy::AHeavyEnemy()
{
    EnemyType = EEnemyType::HeavyMelee;
    MaxHealth = 300.0f;
    CurrentHealth = MaxHealth;
    PatrolSpeed = 150.0f;
    ChaseSpeed = 300.0f;
    AttackRange = 300.0f;
    CombatRange = 800.0f;

    GroundSlamRadius = 400.0f;
    GroundSlamDamage = 50.0f;
    ChargeDistance = 1000.0f;
    ChargeSpeed = 1500.0f;

    bIsCharging = false;

    GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
    GetCapsuleComponent()->SetCapsuleRadius(60.0f);
    GetCapsuleComponent()->SetCapsuleHalfHeight(120.0f);
}

void AHeavyEnemy::BeginPlay()
{
    Super::BeginPlay();
    InitializeHeavyEnemy();
}

void AHeavyEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateHeavyAttacks(DeltaTime);
}

void AHeavyEnemy::InitializeHeavyEnemy()
{
    if (Attacks.Num() < 3)
    {
        Attacks.SetNum(3);
    }

    Attacks[0].Damage = 35.0f;
    Attacks[0].AttackRange = 300.0f;
    Attacks[0].AttackCooldown = 3.0f;
    Attacks[0].WindupTime = 1.0f;

    Attacks[1].Damage = 50.0f;
    Attacks[1].AttackRange = 400.0f;
    Attacks[1].AttackCooldown = 5.0f;
    Attacks[1].WindupTime = 1.5f;

    Attacks[2].Damage = 60.0f;
    Attacks[2].AttackRange = 300.0f;
    Attacks[2].AttackCooldown = 8.0f;
    Attacks[2].WindupTime = 2.0f;
}

void AHeavyEnemy::UpdateHeavyAttacks(float DeltaTime)
{
    if (bIsCharging)
    {
        FVector CurrentLocation = GetActorLocation();
        FVector Direction = (ChargeTargetLocation - CurrentLocation).GetSafeNormal();
        FVector NewLocation = CurrentLocation + Direction * ChargeSpeed * DeltaTime;

        float DistanceTraveled = FVector::Dist(ChargeStartLocation, NewLocation);
        float TotalDistance = FVector::Dist(ChargeStartLocation, ChargeTargetLocation);

        if (DistanceTraveled >= TotalDistance || FVector::Dist(NewLocation, ChargeTargetLocation) < 50.0f)
        {
            SetActorLocation(ChargeTargetLocation);
            bIsCharging = false;
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            SetEnemyState(EEnemyState::Attacking);
        }
        else
        {
            SetActorLocation(NewLocation);
        }
    }
}

void AHeavyEnemy::PerformAttack(int32 AttackIndex)
{
    if (EnemyState == EEnemyState::Dead || bIsAttacking)
    {
        return;
    }

    if (AttackIndex == 1 && CurrentTarget)
    {
        float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
        if (Distance > 500.0f && Distance < 2000.0f)
        {
            PerformChargeAttack();
            return;
        }
    }
    else if (AttackIndex == 2 && CurrentTarget)
    {
        float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
        if (Distance <= 500.0f)
        {
            PerformGroundSlam();
            return;
        }
    }

    Super::PerformAttack(AttackIndex);
}

void AHeavyEnemy::PerformGroundSlam()
{
    if (!GroundSlamEffect || !GroundSlamSound)
    {
        return;
    }

    bIsAttacking = true;
    GetCharacterMovement()->MaxWalkSpeed = 0.0f;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && Attacks.Num() > 2 && Attacks[2].AttackMontage)
    {
        AnimInstance->Montage_Play(Attacks[2].AttackMontage);
    }

    FTimerDelegate SlamDelegate;
    SlamDelegate.BindLambda([this]()
    {
        ExecuteGroundSlam();
    });
    GetWorldTimerManager().SetTimer(AttackCooldownTimer, SlamDelegate, Attacks[2].WindupTime, false);
}

void AHeavyEnemy::ExecuteGroundSlam()
{
    TArray<FHitResult> HitResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    bool bHit = GetWorld()->SweepMultiByChannel(HitResults, GetActorLocation(), GetActorLocation(), FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(GroundSlamRadius), QueryParams);

    for (const FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();
        if (HitActor && HitActor != this)
        {
            ABrainrotCharacter* Player = Cast<ABrainrotCharacter>(HitActor);
            if (Player)
            {
                FVector HitDirection = (HitActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
                Player->TakeDamage(GroundSlamDamage, HitDirection, this);
            }
            else
            {
                FVector HitDirection = (HitActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
                UGameplayStatics::ApplyPointDamage(HitActor, GroundSlamDamage, HitDirection, Hit, GetController(), this, nullptr);
            }
        }
    }

    if (GroundSlamEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), GroundSlamEffect, GetActorLocation());
    }
    if (GroundSlamSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, GroundSlamSound, GetActorLocation());
    }

    FTimerDelegate RecoveryDelegate;
    RecoveryDelegate.BindLambda([this]()
    {
        bIsAttacking = false;
        bIsCharging = false;
        GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
        if (CurrentTarget)
        {
            SetEnemyState(EEnemyState::Chasing);
        }
        else
        {
            SetEnemyState(EEnemyState::Returning);
        }
    });
    GetWorldTimerManager().SetTimer(AttackCooldownTimer, RecoveryDelegate, Attacks[2].AttackCooldown, false);
}

void AHeavyEnemy::PerformChargeAttack()
{
    if (!CurrentTarget || !ChargeSound)
    {
        return;
    }

    bIsAttacking = true;
    bIsCharging = true;
    GetCharacterMovement()->SetMovementMode(MOVE_Flying);
    GetCharacterMovement()->MaxWalkSpeed = ChargeSpeed;

    ChargeStartLocation = GetActorLocation();
    FVector Direction = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    ChargeTargetLocation = GetActorLocation() + Direction * FMath::Min(ChargeDistance, FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation()));
    ChargeTargetLocation.Z = GetActorLocation().Z;

    SetActorRotation(Direction.Rotation());

    if (ChargeSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ChargeSound, GetActorLocation());
    }

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && Attacks.Num() > 1 && Attacks[1].AttackMontage)
    {
        AnimInstance->Montage_Play(Attacks[1].AttackMontage);
    }
}

void AHeavyEnemy::OnAttackHitNotify()
{
    if (bIsCharging)
    {
        return;
    }
    Super::OnAttackHitNotify();
}