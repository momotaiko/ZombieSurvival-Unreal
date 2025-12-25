#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UpgradeDataTypes.generated.h"

UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
    Weapon1DamageIncrease             UMETA(DisplayName = "총 공격력 업그레이드"),
    Weapon2DamageIncrease             UMETA(DisplayName = "유탄발사기 공격력 업그레이드"), 
    Weapon3DamageIncrease             UMETA(DisplayName = "칼 공격력 업그레이드"), 
    MovementSpeedIncrease             UMETA(DisplayName = "이동속도 업그레이드"),
    MaxHealthIncrease                 UMETA(DisplayName = "최대 체력 업그레이드"),
    SpecialUpgrade                    UMETA(DisplayName = "개별 특수 업그레이드")
};

USTRUCT(BlueprintType)
struct FUpgradeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    FText UpgradeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    EUpgradeType UpgradeType;

    // 업그레이드 강화 수치 (퍼센트 또는 절댓값)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    float UpgradeValue;

    // 현재 업그레이드 레벨 (같은 타입의 업그레이드를 여러 번 적용할 때)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    int32 UpgradeLevel;

    // 최대 업그레이드 레벨
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    int32 MaxUpgradeLevel;

    // 특수 업그레이드의 경우 고유 식별자
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    FName SpecialUpgradeID;

    // 등장 조건
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    int32 MinKillCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    int32 MaxKillCount;

    // 희귀도 (높을수록 덜 등장)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade", meta = (ClampMin = "1", ClampMax = "100"))
    int32 Rarity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    TArray<FName> RequiredUpgrades;

    FUpgradeData()
    {
        UpgradeValue = 0.0f;
        UpgradeLevel = 1;
        MaxUpgradeLevel = 5;
        MinKillCount = 0;
        MaxKillCount = 100000;
        Rarity = 50;
    }
};