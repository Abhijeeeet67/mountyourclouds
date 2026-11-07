#include "Weapons/StaffWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Character.h"
#include "Character/BrainrotCharacter.h"

AStaffWeapon::AStaffWeapon()
{
    PrimaryActorTick.bCanEverTick = true;

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    WeaponMesh->SetGenerateOverlapEvents(false);

    HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
    HitBox->SetupAttachment(WeaponMesh);
    HitBox->SetBoxExtent(FVector(15.0f, 15.0f, 60.0f));
    HitBox->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
    HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    HitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    HitBox->SetGenerateOverlapEvents(true);
    HitBox->OnComponentBeginOverlap.AddDynamic(this, &AStaffWeapon::OnHitBoxOverlap);

    DamageMultiplier = 1.0f;
    KnockbackMultiplier = 1.0f;
}

void AStaffWeapon::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponents();
    DisableCollision();
}

void AStaffWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AStaffWeapon::InitializeComponents()
{
    if (WeaponMesh)
    {
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void AStaffWeapon::EnableCollision()
{
    if (HitBox)
    {
        HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
    SpawnTrailEffect();
}

void AStaffWeapon::DisableCollision()
{
    if (HitBox)
    {
        HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    DespawnTrailEffect();
}

void AStaffWeapon::SetOwnerCharacter(AActor* NewOwner)
{
    OwnerCharacter = NewOwner;
}

void AStaffWeapon::OnHitBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this || OtherActor == OwnerCharacter)
    {
        return;
    }

    ABrainrotCharacter* OwnerChar = Cast<ABrainrotCharacter>(OwnerCharacter);
    if (!OwnerChar)
    {
        return;
    }

    FVector HitDirection = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    float Damage = 15.0f * DamageMultiplier;
    float Knockback = 300.0f * KnockbackMultiplier;

    UGameplayStatics::ApplyPointDamage(OtherActor, Damage, HitDirection, SweepResult, OwnerChar->GetController(), OwnerCharacter, nullptr);

    if (HitSparkEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitSparkEffect, SweepResult.Location, HitDirection.Rotation());
    }
    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, SweepResult.Location);
    }
}

void AStaffWeapon::SpawnTrailEffect()
{
    if (TrailEffect && WeaponMesh)
    {
        ActiveTrail = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailEffect, WeaponMesh, TEXT("TrailSocket"), FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
    }
}

void AStaffWeapon::DespawnTrailEffect()
{
    if (ActiveTrail)
    {
        ActiveTrail->Deactivate();
        ActiveTrail = nullptr;
    }
}