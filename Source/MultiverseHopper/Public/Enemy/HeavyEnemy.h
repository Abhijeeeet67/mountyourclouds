#pragma once

#include "CoreMinimal.h"
#include "Enemy/BaseEnemy.h"
#include "HeavyEnemy.generated.h"

UCLASS()
class MULTIVERSEHOPPER_API AHeavyEnemy : public ABaseEnemy
{
    GENERATED_BODY()

public:
    AHeavyEnemy();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Enemy|Heavy")
    void PerformGroundSlam();

    UFUNCTION(BlueprintCallable, Category = "Enemy|Heavy")
    void PerformChargeAttack();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Heavy")
    float GroundSlamRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Heavy")
    float GroundSlamDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Heavy")
    float ChargeDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Heavy")
    float ChargeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Heavy")
    UNiagaraSystem* GroundSlamEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Heavy")
    USoundBase* GroundSlamSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Heavy")
    USoundBase* ChargeSound;

    UPROPERTY()
    bool bIsCharging;

    UPROPERTY()
    FVector ChargeStartLocation;

    UPROPERTY()
    FVector ChargeTargetLocation;

    void InitializeHeavyEnemy();
    void UpdateHeavyAttacks(float DeltaTime);
    void ExecuteGroundSlam();
    void ExecuteChargeAttack();
};