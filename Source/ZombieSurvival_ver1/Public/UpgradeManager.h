#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UpgradeDataTypes.h"
#include "UpgradeManager.generated.h"

// 업그레이드 적용 결과를 나타내는 구조체
USTRUCT(BlueprintType)
struct FUpgradeApplyResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bSuccess;

    UPROPERTY(BlueprintReadOnly)
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly)
    int32 CurrentLevel;

    FUpgradeApplyResult()
    {
        bSuccess = false;
        CurrentLevel = 0;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ZOMBIESURVIVAL_VER1_API UUpgradeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UUpgradeManager();

protected:
    virtual void BeginPlay() override;

public:
    // 업그레이드 적용
    UFUNCTION(BlueprintCallable, Category = "Upgrade")
    FUpgradeApplyResult ApplyUpgrade(const FUpgradeData& UpgradeData);

    // 특정 타입의 현재 업그레이드 레벨 조회
    UFUNCTION(BlueprintCallable, Category = "Upgrade")
    int32 GetUpgradeLevel(EUpgradeType UpgradeType) const;

    // 특정 특수 업그레이드 보유 여부 확인
    UFUNCTION(BlueprintCallable, Category = "Upgrade")
    bool HasSpecialUpgrade(FName SpecialUpgradeID) const;

    // 모든 업그레이드 정보 반환
    UFUNCTION(BlueprintCallable, Category = "Upgrade")
    TMap<EUpgradeType, int32> GetAllUpgradeLevels() const { return UpgradeLevels; }

    // 모든 특수 업그레이드 목록 반환
    UFUNCTION(BlueprintCallable, Category = "Upgrade")
    TArray<FName> GetAllSpecialUpgrades() const { return SpecialUpgrades; }

protected:
    // 각 업그레이드 타입별 현재 레벨
    UPROPERTY(BlueprintReadOnly, Category = "Upgrade")
    TMap<EUpgradeType, int32> UpgradeLevels;

    // 획득한 특수 업그레이드 목록
    UPROPERTY(BlueprintReadOnly, Category = "Upgrade")
    TArray<FName> SpecialUpgrades;

    // 기본값들 저장 (업그레이드 계산용)
    UPROPERTY(BlueprintReadOnly, Category = "Upgrade")
    float BaseGunDamage;

    UPROPERTY(BlueprintReadOnly, Category = "Upgrade")
    float BaseGrenadeDamage;

    UPROPERTY(BlueprintReadOnly, Category = "Upgrade")
    float BaseMeleeDamage;

    UPROPERTY(BlueprintReadOnly, Category = "Upgrade")
    float BaseMovementSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Upgrade")
    float BaseAimingSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Upgrade")
    float BaseMaxHealth;

private:
    // 각 업그레이드 타입별 적용 함수
    void ApplyWeaponDamageUpgrade(int32 WeaponType, float UpgradeValue);
    void ApplyMovementSpeedUpgrade(float UpgradeValue);
    void ApplyMaxHealthUpgrade(float UpgradeValue);
    void ApplySpecialUpgrade(FName SpecialUpgradeID, float UpgradeValue);

    // 플레이어 캐릭터 레퍼런스
    class AZombieSurvivalCharacter* PlayerCharacter;
}; 