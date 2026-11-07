#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StaffWeapon.generated.h"

class USkeletalMeshComponent;
class UBoxComponent;
class UNiagaraSystem;
class USoundBase;

UCLASS()
class MULTIVERSEHOPPER_API AStaffWeapon : public AActor
{
    GENERATED_BODY()

public:
    AStaffWeapon();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void EnableCollision();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void DisableCollision();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void SetOwnerCharacter(AActor* NewOwner);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    USkeletalMeshComponent* WeaponMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    UBoxComponent* HitBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
    UNiagaraSystem* TrailEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
    UNiagaraSystem* HitSparkEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Audio")
    USoundBase* SwingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Audio")
    USoundBase* ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Settings")
    float DamageMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Settings")
    float KnockbackMultiplier;

protected:
    UPROPERTY()
    AActor* OwnerCharacter;

    UPROPERTY()
    UNiagaraComponent* ActiveTrail;

    UFUNCTION()
    void OnHitBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    void InitializeComponents();
    void SpawnTrailEffect();
    void DespawnTrailEffect();
};