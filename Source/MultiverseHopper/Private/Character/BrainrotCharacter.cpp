#include "Character/BrainrotCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "Weapons/StaffWeapon.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"

ABrainrotCharacter::ABrainrotCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 300.0f;
    CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 50.0f);
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 10.0f;
    CameraBoom->bDoCollisionTest = true;
    CameraBoom->ProbeSize = 12.0f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
    FollowCamera->FieldOfView = 90.0f;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 600.0f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 300.0f;
    GetCharacterMovement()->MaxWalkSpeedCrouched = 150.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;
    GetCharacterMovement()->GroundFriction = 3.0f;
    GetCharacterMovement()->BrakingFrictionFactor = 1.0f;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    MaxStamina = 100.0f;
    CurrentStamina = MaxStamina;
    StaminaRegenRate = 15.0f;
    StaminaRegenDelay = 1.0f;

    WalkSpeed = 300.0f;
    SprintSpeed = 600.0f;
    CrouchSpeed = 150.0f;
    RotationRate = 540.0f;

    CameraDistance = 300.0f;
    CameraHeight = 50.0f;
    CameraLagSpeed = 10.0f;

    DodgeDistance = 400.0f;
    DodgeInvincibilityTime = 0.3f;
    RollDistance = 600.0f;
    RollInvincibilityTime = 0.5f;

    StaminaCostLightAttack = 10.0f;
    StaminaCostHeavyAttack = 25.0f;
    StaminaCostDodge = 20.0f;
    StaminaCostRoll = 30.0f;

    CombatState = ECombatState::Idle;
    MovementState = EMovementState::Idle;
    CurrentComboIndex = 0;
    bComboInputQueued = false;
    bIsInvincible = false;
    LastStaminaUseTime = 0.0f;

    LightAttackCombo.SetNum(3);
    for (int32 i = 0; i < 3; ++i)
    {
        LightAttackCombo[i].Damage = 15.0f + (i * 5.0f);
        LightAttackCombo[i].KnockbackForce = 200.0f + (i * 50.0f);
        LightAttackCombo[i].AttackRange = 180.0f;
        LightAttackCombo[i].AttackAngle = 60.0f;
        LightAttackCombo[i].HitNotifyName = FName(*FString::Printf(TEXT("HitNotify_%d"), i + 1));
        LightAttackCombo[i].ComboWindowStart = 0.3f;
        LightAttackCombo[i].ComboWindowEnd = 0.7f;
    }

    HeavyAttackData.Damage = 40.0f;
    HeavyAttackData.KnockbackForce = 800.0f;
    HeavyAttackData.AttackRange = 220.0f;
    HeavyAttackData.AttackAngle = 90.0f;
    HeavyAttackData.HitNotifyName = TEXT("HeavyHitNotify");
    HeavyAttackData.ComboWindowStart = 0.0f;
    HeavyAttackData.ComboWindowEnd = 0.0f;
}

void ABrainrotCharacter::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponents();
    InitializeAttributes();
    InitializeInput();
    InitializeCombat();
    InitializeCamera();

    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ABrainrotCharacter::InitializeComponents()
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }
}

void ABrainrotCharacter::InitializeAttributes()
{
    CurrentHealth = MaxHealth;
    CurrentStamina = MaxStamina;
}

void ABrainrotCharacter::InitializeInput()
{
}

void ABrainrotCharacter::InitializeCombat()
{
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    EquippedStaff = GetWorld()->SpawnActor<AStaffWeapon>(AStaffWeapon::StaticClass(), FTransform::Identity, SpawnParams);
    if (EquippedStaff)
    {
        EquippedStaff->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("StaffSocket"));
        EquippedStaff->SetOwner(this);
    }
}

void ABrainrotCharacter::InitializeCamera()
{
    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = CameraDistance;
        CameraBoom->SocketOffset.Z = CameraHeight;
        CameraBoom->CameraLagSpeed = CameraLagSpeed;
    }
}

void ABrainrotCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABrainrotCharacter::Move);
        }
        if (LookAction)
        {
            EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABrainrotCharacter::Look);
        }
        if (JumpAction)
        {
            EnhancedInput->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
            EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }
        if (SprintAction)
        {
            EnhancedInput->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ABrainrotCharacter::SprintPressed);
            EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ABrainrotCharacter::SprintReleased);
        }
        if (CrouchAction)
        {
            EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ABrainrotCharacter::CrouchPressed);
            EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ABrainrotCharacter::CrouchReleased);
        }
        if (LightAttackAction)
        {
            EnhancedInput->BindAction(LightAttackAction, ETriggerEvent::Triggered, this, &ABrainrotCharacter::LightAttack);
        }
        if (HeavyAttackAction)
        {
            EnhancedInput->BindAction(HeavyAttackAction, ETriggerEvent::Triggered, this, &ABrainrotCharacter::HeavyAttack);
        }
        if (DodgeAction)
        {
            EnhancedInput->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ABrainrotCharacter::Dodge);
        }
        if (RollAction)
        {
            EnhancedInput->BindAction(RollAction, ETriggerEvent::Triggered, this, &ABrainrotCharacter::Roll);
        }
        if (PauseAction)
        {
            EnhancedInput->BindAction(PauseAction, ETriggerEvent::Triggered, this, &ABrainrotCharacter::OnPausePressed);
        }
    }
}

void ABrainrotCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateMovementState();
    UpdateCombatState();
    UpdateCamera(DeltaTime);

    if (CurrentStamina < MaxStamina)
    {
        if (GetWorld()->GetTimeSeconds() - LastStaminaUseTime >= StaminaRegenDelay)
        {
            CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + StaminaRegenRate * DeltaTime);
        }
    }
}

void ABrainrotCharacter::Move(const FInputActionValue& Value)
{
    if (CombatState == ECombatState::Dead || CombatState == ECombatState::Attacking || CombatState == ECombatState::HeavyAttack ||
        CombatState == ECombatState::Dodging || CombatState == ECombatState::Rolling)
    {
        return;
    }

    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller && (MovementVector.X != 0.0f || MovementVector.Y != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);

        LastMovementInput = FVector(MovementVector.X, MovementVector.Y, 0.0f).GetSafeNormal();
    }
}

void ABrainrotCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller && (LookAxisVector.X != 0.0f || LookAxisVector.Y != 0.0f))
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ABrainrotCharacter::SprintPressed()
{
    if (CombatState != ECombatState::Dead && CurrentStamina > 10.0f && GetCharacterMovement()->IsMovingOnGround())
    {
        GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
        MovementState = EMovementState::Sprinting;
    }
}

void ABrainrotCharacter::SprintReleased()
{
    if (CombatState != ECombatState::Dead)
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        if (MovementState == EMovementState::Sprinting)
        {
            MovementState = EMovementState::Walking;
        }
    }
}

void ABrainrotCharacter::CrouchPressed()
{
    if (CombatState != ECombatState::Dead && CombatState != ECombatState::Attacking && CombatState != ECombatState::HeavyAttack)
    {
        Crouch();
    }
}

void ABrainrotCharacter::CrouchReleased()
{
    if (CombatState != ECombatState::Dead)
    {
        UnCrouch();
    }
}

void ABrainrotCharacter::LightAttack()
{
    if (CombatState == ECombatState::Dead || CombatState == ECombatState::HeavyAttack ||
        CombatState == ECombatState::Dodging || CombatState == ECombatState::Rolling ||
        CurrentStamina < StaminaCostLightAttack)
    {
        return;
    }

    if (CombatState == ECombatState::Attacking && bComboInputQueued)
    {
        bComboInputQueued = false;
        return;
    }

    PerformLightAttack();
}

void ABrainrotCharacter::HeavyAttack()
{
    if (CombatState == ECombatState::Dead || CombatState == ECombatState::Attacking ||
        CombatState == ECombatState::Dodging || CombatState == ECombatState::Rolling ||
        CurrentStamina < StaminaCostHeavyAttack)
    {
        return;
    }

    PerformHeavyAttack();
}

void ABrainrotCharacter::Dodge()
{
    if (CombatState == ECombatState::Dead || CombatState == ECombatState::Attacking ||
        CombatState == ECombatState::HeavyAttack || CombatState == ECombatState::Rolling ||
        CurrentStamina < StaminaCostDodge || !GetCharacterMovement()->IsMovingOnGround())
    {
        return;
    }

    PerformDodge();
}

void ABrainrotCharacter::Roll()
{
    if (CombatState == ECombatState::Dead || CombatState == ECombatState::Attacking ||
        CombatState == ECombatState::HeavyAttack || CombatState == ECombatState::Dodging ||
        CurrentStamina < StaminaCostRoll || !GetCharacterMovement()->IsMovingOnGround())
    {
        return;
    }

    PerformRoll();
}

void ABrainrotCharacter::PerformLightAttack()
{
    if (CurrentComboIndex >= LightAttackCombo.Num())
    {
        CurrentComboIndex = 0;
    }

    const FComboAttackData& AttackData = LightAttackCombo[CurrentComboIndex];

    if (AttackData.AttackMontage)
    {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(AttackData.AttackMontage);
        }
    }

    CombatState = ECombatState::Attacking;
    CurrentStamina -= StaminaCostLightAttack;
    LastStaminaUseTime = GetWorld()->GetTimeSeconds();

    GetWorldTimerManager().SetTimer(ComboWindowTimer, this, &ABrainrotCharacter::OnComboWindowOpen, AttackData.ComboWindowStart, false);
    GetWorldTimerManager().SetTimer(ComboWindowTimer, this, &ABrainrotCharacter::OnComboWindowClose, AttackData.ComboWindowEnd, false);

    if (SwingSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, SwingSound, GetActorLocation());
    }
}

void ABrainrotCharacter::PerformHeavyAttack()
{
    if (HeavyAttackData.AttackMontage)
    {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(HeavyAttackData.AttackMontage);
        }
    }

    CombatState = ECombatState::HeavyAttack;
    CurrentStamina -= StaminaCostHeavyAttack;
    LastStaminaUseTime = GetWorld()->GetTimeSeconds();

    if (SwingSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, SwingSound, GetActorLocation());
    }
}

void ABrainrotCharacter::PerformDodge()
{
    CombatState = ECombatState::Dodging;
    bIsInvincible = true;
    CurrentStamina -= StaminaCostDodge;
    LastStaminaUseTime = GetWorld()->GetTimeSeconds();

    FVector DodgeDirection = LastMovementInput.IsNearlyZero() ? GetActorForwardVector() : LastMovementInput;
    LaunchCharacter(DodgeDirection * DodgeDistance, true, true);

    if (DodgeEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DodgeEffect, GetActorLocation());
    }
    if (DodgeSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DodgeSound, GetActorLocation());
    }

    GetWorldTimerManager().SetTimer(InvincibilityTimer, this, [this]()
    {
        bIsInvincible = false;
        if (CombatState == ECombatState::Dodging)
        {
            CombatState = ECombatState::Idle;
        }
    }, DodgeInvincibilityTime, false);
}

void ABrainrotCharacter::PerformRoll()
{
    CombatState = ECombatState::Rolling;
    bIsInvincible = true;
    CurrentStamina -= StaminaCostRoll;
    LastStaminaUseTime = GetWorld()->GetTimeSeconds();

    FVector RollDirection = LastMovementInput.IsNearlyZero() ? GetActorForwardVector() : LastMovementInput;
    LaunchCharacter(RollDirection * RollDistance, true, true);

    if (RollEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), RollEffect, GetActorLocation());
    }
    if (DodgeSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DodgeSound, GetActorLocation());
    }

    GetWorldTimerManager().SetTimer(InvincibilityTimer, this, [this]()
    {
        bIsInvincible = false;
        if (CombatState == ECombatState::Rolling)
        {
            CombatState = ECombatState::Idle;
        }
    }, RollInvincibilityTime, false);
}

void ABrainrotCharacter::OnComboWindowOpen()
{
    bComboInputQueued = true;
}

void ABrainrotCharacter::OnComboWindowClose()
{
    if (bComboInputQueued)
    {
        CurrentComboIndex = (CurrentComboIndex + 1) % LightAttackCombo.Num();
        bComboInputQueued = false;
    }
    else
    {
        ResetCombo();
    }
}

void ABrainrotCharacter::ResetCombo()
{
    CurrentComboIndex = 0;
    bComboInputQueued = false;
    if (CombatState == ECombatState::Attacking || CombatState == ECombatState::HeavyAttack)
    {
        CombatState = ECombatState::Idle;
    }
}

void ABrainrotCharacter::OnAttackHitNotify()
{
    int32 AttackIndex = (CombatState == ECombatState::HeavyAttack) ? -1 : CurrentComboIndex;
    ApplyAttackDamage(AttackIndex);
}

void ABrainrotCharacter::ApplyAttackDamage(int32 AttackIndex)
{
    const FComboAttackData& AttackData = (AttackIndex == -1) ? HeavyAttackData : LightAttackCombo[FMath::Clamp(AttackIndex, 0, LightAttackCombo.Num() - 1)];

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
            FVector HitDirection = (HitActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            UGameplayStatics::ApplyPointDamage(HitActor, AttackData.Damage, HitDirection, Hit, GetController(), this, nullptr);

            if (HitEffect)
            {
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, Hit.Location, HitDirection.Rotation());
            }
            if (HitSound)
            {
                UGameplayStatics::PlaySoundAtLocation(this, HitSound, Hit.Location);
            }
        }
    }
}

void ABrainrotCharacter::TakeDamage(float DamageAmount, FVector HitDirection, AActor* DamageCauser)
{
    if (CombatState == ECombatState::Dead || bIsInvincible)
    {
        return;
    }

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

    if (HurtSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, HurtSound, GetActorLocation());
    }

    LaunchCharacter(HitDirection * 300.0f + FVector(0, 0, 100.0f), true, true);

    CombatState = ECombatState::HitReaction;

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    else
    {
        GetWorldTimerManager().SetTimer(InvincibilityTimer, this, [this]()
        {
            if (CombatState == ECombatState::HitReaction)
            {
                CombatState = ECombatState::Idle;
            }
        }, 0.5f, false);
    }
}

void ABrainrotCharacter::Heal(float Amount)
{
    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Amount);
}

void ABrainrotCharacter::Die()
{
    CombatState = ECombatState::Dead;
    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (DeathSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
    }

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
    }

    GetWorldTimerManager().SetTimer(InvincibilityTimer, this, &ABrainrotCharacter::Respawn, 3.0f, false);
}

void ABrainrotCharacter::Respawn()
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (AActor* PlayerStart = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass()))
        {
            SetActorLocation(PlayerStart->GetActorLocation());
            SetActorRotation(PlayerStart->GetActorRotation());
        }
    }

    CurrentHealth = MaxHealth;
    CurrentStamina = MaxStamina;
    CombatState = ECombatState::Idle;
    MovementState = EMovementState::Idle;
    CurrentComboIndex = 0;
    bComboInputQueued = false;
    bIsInvincible = false;

    GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ABrainrotCharacter::UpdateMovementState()
{
    if (CombatState == ECombatState::Dead)
    {
        MovementState = EMovementState::Idle;
        return;
    }

    if (!GetCharacterMovement()->IsMovingOnGround())
    {
        if (GetCharacterMovement()->Velocity.Z > 0)
        {
            MovementState = EMovementState::Jumping;
        }
        else
        {
            MovementState = EMovementState::Falling;
        }
    }
    else if (GetCharacterMovement()->Velocity.Size2D() > 10.0f)
    {
        if (GetCharacterMovement()->MaxWalkSpeed > WalkSpeed + 50.0f)
        {
            MovementState = EMovementState::Sprinting;
        }
        else if (bIsCrouched)
        {
            MovementState = EMovementState::Crouching;
        }
        else
        {
            MovementState = EMovementState::Walking;
        }
    }
    else
    {
        MovementState = EMovementState::Idle;
    }
}

void ABrainrotCharacter::UpdateCombatState()
{
}

void ABrainrotCharacter::UpdateCamera(float DeltaTime)
{
}

void ABrainrotCharacter::OnLanded(const FHitResult& Hit)
{
    Super::OnLanded(Hit);
    MovementState = EMovementState::Landing;
}

void ABrainrotCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
    Super::OnMovementModeChanged(PrevMovementMode, PrevCustomMode);
}

void ABrainrotCharacter::OnHealthChanged(float OldValue, float NewValue)
{
}

void ABrainrotCharacter::OnStaminaChanged(float OldValue, float NewValue)
{
}

void ABrainrotCharacter::SetCameraMode(bool bOverShoulder)
{
    if (CameraBoom)
    {
        CameraBoom->bUsePawnControlRotation = bOverShoulder;
    }
}

void ABrainrotCharacter::OnPausePressed()
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->SetPause(true);
    }
}