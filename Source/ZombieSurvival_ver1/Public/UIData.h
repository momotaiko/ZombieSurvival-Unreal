#pragma once
#include "CoreMinimal.h"
#include "UIData.generated.h"

UCLASS(BlueprintType)
class ZOMBIESURVIVAL_VER1_API UUIData : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "UI") float MaxHealth;
    UPROPERTY(BlueprintReadWrite, Category = "UI") float CurrentHealth;
    UPROPERTY(BlueprintReadWrite, Category = "UI") int32 Red;
    UPROPERTY(BlueprintReadWrite, Category = "UI") int32 Green;
    UPROPERTY(BlueprintReadWrite, Category = "UI") int32 Blue;
    UPROPERTY(BlueprintReadWrite, Category = "UI") int32 Kills;
    UPROPERTY(BlueprintReadWrite, Category = "UI") float Stamina;
    UPROPERTY(BlueprintReadWrite, Category = "UI") float MaxStamina;
    
    UPROPERTY(BlueprintReadWrite, Category = "Upgrades") int32 Gun;
    UPROPERTY(BlueprintReadWrite, Category = "Upgrades") int32 GrenadeLauncher;
    UPROPERTY(BlueprintReadWrite, Category = "Upgrades") int32 Sword;
    UPROPERTY(BlueprintReadWrite, Category = "Upgrades") int32 MoveSpeed;
    UPROPERTY(BlueprintReadWrite, Category = "Upgrades") int32 AdditionalMaxHealth;
    
    UPROPERTY(BlueprintReadWrite, Category = "Special Upgrades") bool DoubleJump;
    UPROPERTY(BlueprintReadWrite, Category = "Special Upgrades") bool SwordRangeIncrease;
    UPROPERTY(BlueprintReadWrite, Category = "Special Upgrades") bool GunReloadSpeedIncrease;
    UPROPERTY(BlueprintReadWrite, Category = "Special Upgrades") bool GunMaxAmmoIncrease;
    UPROPERTY(BlueprintReadWrite, Category = "Special Upgrades") bool GrenadeExplosionRangeIncrease;
    UPROPERTY(BlueprintReadWrite, Category = "Special Upgrades") bool ZombieDropMoreCrystals;
    UPROPERTY(BlueprintReadWrite, Category = "Special Upgrades") bool TurretRangeIncrease;
    UPROPERTY(BlueprintReadWrite, Category = "Special Upgrades") bool SelfHealingAbility;
    UPROPERTY(BlueprintReadWrite, Category = "Special Upgrades") bool TurretCostReduction;
    UPROPERTY(BlueprintReadWrite, Category = "Special Upgrades") bool CrystalPickupRangeIncrease;
};
