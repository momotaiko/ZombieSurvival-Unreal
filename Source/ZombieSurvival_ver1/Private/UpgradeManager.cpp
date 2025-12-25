#include "UpgradeManager.h"
#include "ZombieSurvivalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"

UUpgradeManager::UUpgradeManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    PlayerCharacter = nullptr;
}

void UUpgradeManager::BeginPlay()
{
    Super::BeginPlay();
    
    PlayerCharacter = Cast<AZombieSurvivalCharacter>(GetOwner());
    if (!PlayerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("UpgradeManager: Owner is not AZombieSurvivalCharacter"));
        return;
    }

    // 기본값들 저장 (업그레이드 계산을 위해)
    BaseGunDamage = PlayerCharacter->GetGunDamage();
    BaseGrenadeDamage = PlayerCharacter->GetGrenadeDamage();
    BaseMeleeDamage = PlayerCharacter->GetMeleeDamage();
    BaseMovementSpeed = PlayerCharacter->GetCharacterMovement() ? PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed : 600.0f;
    BaseAimingSpeed = PlayerCharacter->GetAimingWalkSpeed();
    BaseMaxHealth = PlayerCharacter->MaxHealth;

    UE_LOG(LogTemp, Warning, TEXT("Base values saved - Gun: %.1f, Grenade: %.1f, Melee: %.1f, Speed: %.1f, Health: %.1f"), 
        BaseGunDamage, BaseGrenadeDamage, BaseMeleeDamage, BaseMovementSpeed, BaseMaxHealth);

    // 초기 업그레이드 레벨 설정
    UpgradeLevels.Add(EUpgradeType::Weapon1DamageIncrease, 0);
    UpgradeLevels.Add(EUpgradeType::Weapon2DamageIncrease, 0);
    UpgradeLevels.Add(EUpgradeType::Weapon3DamageIncrease, 0);
    UpgradeLevels.Add(EUpgradeType::MovementSpeedIncrease, 0);
    UpgradeLevels.Add(EUpgradeType::MaxHealthIncrease, 0);
}

FUpgradeApplyResult UUpgradeManager::ApplyUpgrade(const FUpgradeData& UpgradeData)
{
    FUpgradeApplyResult Result;
    
    if (!PlayerCharacter)
    {
        Result.bSuccess = false;
        Result.ErrorMessage = TEXT("Player character not found");
        return Result;
    }

    // 특수 업그레이드가 아닌 경우 레벨 제한 확인
    if (UpgradeData.UpgradeType != EUpgradeType::SpecialUpgrade)
    {
        int32 CurrentLevel = GetUpgradeLevel(UpgradeData.UpgradeType);
        if (CurrentLevel >= UpgradeData.MaxUpgradeLevel)
        {
            Result.bSuccess = false;
            Result.ErrorMessage = TEXT("Maximum upgrade level reached");
            Result.CurrentLevel = CurrentLevel;
            return Result;
        }
    }

    // 업그레이드 타입에 따라 적용
    switch (UpgradeData.UpgradeType)
    {
        case EUpgradeType::Weapon1DamageIncrease:
            ApplyWeaponDamageUpgrade(1, UpgradeData.UpgradeValue);
            UpgradeLevels[EUpgradeType::Weapon1DamageIncrease]++;
            break;
            
        case EUpgradeType::Weapon2DamageIncrease:
            ApplyWeaponDamageUpgrade(2, UpgradeData.UpgradeValue);
            UpgradeLevels[EUpgradeType::Weapon2DamageIncrease]++;
            break;
            
        case EUpgradeType::Weapon3DamageIncrease:
            ApplyWeaponDamageUpgrade(3, UpgradeData.UpgradeValue);
            UpgradeLevels[EUpgradeType::Weapon3DamageIncrease]++;
            break;
            
        case EUpgradeType::MovementSpeedIncrease:
            ApplyMovementSpeedUpgrade(UpgradeData.UpgradeValue);
            UpgradeLevels[EUpgradeType::MovementSpeedIncrease]++;
            break;
            
        case EUpgradeType::MaxHealthIncrease:
            ApplyMaxHealthUpgrade(UpgradeData.UpgradeValue);
            UpgradeLevels[EUpgradeType::MaxHealthIncrease]++;
            break;
            
        case EUpgradeType::SpecialUpgrade:
            ApplySpecialUpgrade(UpgradeData.SpecialUpgradeID, UpgradeData.UpgradeValue);
            break;
            
        default:
            Result.bSuccess = false;
            Result.ErrorMessage = TEXT("Unknown upgrade type");
            return Result;
    }

    Result.bSuccess = true;
    Result.CurrentLevel = GetUpgradeLevel(UpgradeData.UpgradeType);
    
    UE_LOG(LogTemp, Warning, TEXT("Upgrade applied successfully: %s"), *UpgradeData.UpgradeName.ToString());
    
    return Result;
}

int32 UUpgradeManager::GetUpgradeLevel(EUpgradeType UpgradeType) const
{
    if (const int32* Level = UpgradeLevels.Find(UpgradeType))
    {
        return *Level;
    }
    return 0;
}

bool UUpgradeManager::HasSpecialUpgrade(FName SpecialUpgradeID) const
{
    return SpecialUpgrades.Contains(SpecialUpgradeID);
}

void UUpgradeManager::ApplyWeaponDamageUpgrade(int32 WeaponType, float UpgradeValue)
{
    switch (WeaponType)
    {
        case 1: // 총
        {
            // 기본값에서 새로운 업그레이드 적용 (교체 방식)
            float NewDamage = BaseGunDamage * (1.0f + UpgradeValue / 100.0f);
            PlayerCharacter->SetGunDamage(NewDamage);
            UE_LOG(LogTemp, Warning, TEXT("Gun damage upgraded: Base(%.1f) -> New(%.1f) [+%.1f%%]"), 
                BaseGunDamage, NewDamage, UpgradeValue);
            break;
        }
        case 2: // 유탄발사기
        {
            // 기본값에서 새로운 업그레이드 적용 (교체 방식)
            float NewDamage = BaseGrenadeDamage * (1.0f + UpgradeValue / 100.0f);
            PlayerCharacter->SetGrenadeDamage(NewDamage);
            UE_LOG(LogTemp, Warning, TEXT("Grenade damage upgraded: Base(%.1f) -> New(%.1f) [+%.1f%%]"), 
                BaseGrenadeDamage, NewDamage, UpgradeValue);
            break;
        }
        case 3: // 칼
        {
            // 기본값에서 새로운 업그레이드 적용 (교체 방식)
            float NewDamage = BaseMeleeDamage * (1.0f + UpgradeValue / 100.0f);
            PlayerCharacter->SetMeleeDamage(NewDamage);
            UE_LOG(LogTemp, Warning, TEXT("Melee damage upgraded: Base(%.1f) -> New(%.1f) [+%.1f%%]"), 
                BaseMeleeDamage, NewDamage, UpgradeValue);
            break;
        }
    }
}

void UUpgradeManager::ApplyMovementSpeedUpgrade(float UpgradeValue)
{
    if (PlayerCharacter && PlayerCharacter->GetCharacterMovement())
    {
        // 기본값에서 새로운 업그레이드 적용 (교체 방식)
        float NewSpeed = BaseMovementSpeed * (1.0f + UpgradeValue / 100.0f);
        float NewAimingSpeed = BaseAimingSpeed * (1.0f + UpgradeValue / 100.0f);
        
        PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
        PlayerCharacter->SetAimingWalkSpeed(NewAimingSpeed);
        PlayerCharacter->SetDefaultWalkSpeed(NewSpeed);
        
        UE_LOG(LogTemp, Warning, TEXT("Movement speed upgraded: Base(%.1f) -> New(%.1f) [+%.1f%%]"), 
            BaseMovementSpeed, NewSpeed, UpgradeValue);
    }
}

void UUpgradeManager::ApplyMaxHealthUpgrade(float UpgradeValue)
{
    // 현재 체력 비율 계산 (업그레이드 후에도 비율 유지)
    float HealthRatio = PlayerCharacter->CurrentHealth / PlayerCharacter->MaxHealth;
    
    // 기본값에서 새로운 업그레이드 적용 (교체 방식)
    float NewMaxHealth = BaseMaxHealth * (1.0f + UpgradeValue / 100.0f);
    
    PlayerCharacter->MaxHealth = NewMaxHealth;
    PlayerCharacter->CurrentHealth = NewMaxHealth * HealthRatio;
    
    UE_LOG(LogTemp, Warning, TEXT("Max health upgraded: Base(%.1f) -> New(%.1f) [+%.1f%%] (Current: %.1f)"), 
        BaseMaxHealth, NewMaxHealth, UpgradeValue, PlayerCharacter->CurrentHealth);
}

void UUpgradeManager::ApplySpecialUpgrade(FName SpecialUpgradeID, float UpgradeValue)
{
    // 이미 보유한 특수 업그레이드는 적용하지 않음
    if (HasSpecialUpgrade(SpecialUpgradeID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Special upgrade already owned: %s"), *SpecialUpgradeID.ToString());
        return;
    }

    // 특수 업그레이드 적용
    FString UpgradeIDString = SpecialUpgradeID.ToString();
    
    if (UpgradeIDString == TEXT("DoubleJump"))
    {
        PlayerCharacter->SetHaveWing(true);
        UE_LOG(LogTemp, Warning, TEXT("Double jump enabled!"));
    }
    else if (UpgradeIDString == TEXT("SwordRangeIncrease"))
    {
        // TODO: 검 사정거리 증가 구현
        UE_LOG(LogTemp, Warning, TEXT("Sword range increased!"));
    }
    else if (UpgradeIDString == TEXT("GunReloadSpeedIncrease"))
    {
        // TODO: 총 재장전 속도 증가 구현
        UE_LOG(LogTemp, Warning, TEXT("Gun reload speed increased!"));
    }
    else if (UpgradeIDString == TEXT("GunMaxAmmoIncrease"))
    {
        // TODO: 총 최대 탄약 증가 구현
        UE_LOG(LogTemp, Warning, TEXT("Gun max ammo increased!"));
    }
    else if (UpgradeIDString == TEXT("GrenadeExplosionRangeIncrease"))
    {
        // TODO: 유탄 폭발 범위 증가 구현
        UE_LOG(LogTemp, Warning, TEXT("Grenade explosion range increased!"));
    }
    else if (UpgradeIDString == TEXT("ZombieDropMoreCrystals"))
    {
        // TODO: 좀비가 수정을 더 드랍하는 기능 구현
        UE_LOG(LogTemp, Warning, TEXT("Zombies now drop more crystals!"));
    }
    else if (UpgradeIDString == TEXT("TurretRangeIncrease"))
    {
        // TODO: 터렛 사정거리 증가 구현
        UE_LOG(LogTemp, Warning, TEXT("Turret range increased!"));
    }
    else if (UpgradeIDString == TEXT("SelfHealingAbility"))
    {
        // TODO: 자가 치유 능력 구현
        UE_LOG(LogTemp, Warning, TEXT("Self healing ability acquired!"));
    }
    else if (UpgradeIDString == TEXT("TurretCostReduction"))
    {
        // TODO: 터렛 설치 비용 감소 구현
        UE_LOG(LogTemp, Warning, TEXT("Turret cost reduced!"));
    }
    else if (UpgradeIDString == TEXT("CrystalPickupRangeIncrease"))
    {
        // TODO: 수정 획득 가능 거리 증가 구현
        UE_LOG(LogTemp, Warning, TEXT("Crystal pickup range increased!"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Unknown special upgrade: %s"), *UpgradeIDString);
    }

    // 특수 업그레이드 목록에 추가
    SpecialUpgrades.Add(SpecialUpgradeID);
} 