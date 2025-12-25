// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZombieSurvivalCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class AGun;

UCLASS()
class ZOMBIESURVIVAL_VER1_API AZombieSurvivalCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AZombieSurvivalCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 CurrentWeaponMode = 1; // 1: 근접, 2: 원거리, 3: 중거리 (근접 1 원거리 2 중거리)

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AGrenade> GrenadeClass;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

	// 입력 핸들러
	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void Jump() override;
	void Landed(const FHitResult& Hit) override;
	void PerformAirDash();
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);
	void OnResetVR();

	// 공격 핸들러
	void FireWeapon();
	void StartAiming();
	void StopAiming();

	FTimerHandle FireTimerHandle;       // 연사 타이머 핸들
	float FireRate = 0.1f;              // 연사 비율 (기본 10배)

	void StartFiring();                 // 연사 시작
	void StopFiring();

	void SwitchWeaponMode(int32 NewMode);
	void HandleMouseWheelSwitch(float Value);
	void SelectGun();
	void SelectLauncher();
	void SelectMelee();

	// 게임 정지/재개
	void TogglePause();

	// 총
	int32 MaxAmmo = 30;
	int32 CurrentAmmo = 30;
	bool bIsReloading = false;

	// 유탄발사기
	int32 MaxGrenadeAmmo = 5;
	int32 CurrentGrenadeAmmo = 5;
	bool bIsGrenadeReloading = false;

	float GrenadeReloadDuration = 2.0f;
	FTimerHandle GrenadeReloadTimer;


	// 원거리 쿨다운
	bool bCanLaunchGrenade = true;
	FTimerHandle GrenadeCooldownTimer;
	float GrenadeFireCooldown = 0.7f;



	FTimerHandle ReloadTimerHandle;
	float ReloadDuration = 1.85f; // 재장전 시간 (초)

	void Reload();                  // 재장전 시작
	void FinishReloading();         // 재장전 종료
	void ReloadGrenade();
	void FinishReloadingGrenade();

	// 근접 공격 함수
	void PerformMeleeAttack();
	void DrawMeleeAttackRange(const FVector& Center, const FVector& ForwardVector);

	// 사운드
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundBase* ReloadSound;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundBase* FireSound;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundBase* SwitchWeaponSound;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundBase* HitSound;

	// 반동 조절
	UPROPERTY(EditDefaultsOnly, Category = Recoil)
	float RecoilAmount = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = Recoil)
	float RecoilRecoverySpeed = 8.f;

	FRotator TargetControlRotation;
	bool bIsRecoiling = false;

public:
	// 카메라 액세서리
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	// 줌 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Zoom)
	float DefaultFOV = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Zoom)
	float ZoomedFOV = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Zoom)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Zoom)
	bool bIsAiming = false;

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float DefaultWalkSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float AimingWalkSpeed = 300.f;

	// 무기 클래스
	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	TSubclassOf<AGun> GunClass;

	UPROPERTY()
	AGun* EquippedGun;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	TSubclassOf<class AGrenadeLauncher> LauncherClass;

	UPROPERTY()
	AGrenadeLauncher* EquippedLauncher;

	UPROPERTY(EditAnywhere)
	float GunDamage = 10.f;

	// 유탄발사기 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float GrenadeDamage = 50.f;

	// 근접 무기 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
	float MeleeDamage = 25.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
	float MeleeRange = 200.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
	float MeleeAngle = 120.f; // 부채꼴 각도
	
	bool bCanMeleeAttack = true;
	FTimerHandle MeleeAttackCooldownTimer;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
	float MeleeAttackCooldown = 0.8f;

	// 포탑 빌더 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret")
	class UTurretBuilderComponent* TurretBuilder;

	// 업그레이드 매니저 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Upgrade")
	class UUpgradeManager* UpgradeManager;

	// 각 무기별 포탑 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Turret")
	TSubclassOf<AActor> GunTurretClass;

	UPROPERTY(EditDefaultsOnly, Category = "Turret")
	TSubclassOf<AActor> GrenadeTurretClass;

	UPROPERTY(EditDefaultsOnly, Category = "Turret")
	TSubclassOf<AActor> MeleeTurretClass;

	// 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHealth = 80.0f;

	// 크리스탈
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crystal")
	int32 RedCrystalCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crystal")
	int32 BlueCrystalCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crystal")
	int32 GreenCrystalCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Kills = 0;

	
	UFUNCTION(BlueprintCallable)
	bool IsAlive() const { return CurrentHealth > 0.f; }

	UFUNCTION(BlueprintCallable)
	void TakeDamage(float Damage); // 피해 적용

	// 원거리 쿨다운
	bool bCanAirDash = true;
	bool haveWing = false;

	// === 달리기 시스템 ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sprint")
	bool bIsSprinting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint") 
	float SprintSpeedMultiplier = 1.3f; // 달리기 속도 배율

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sprint")
	float CurrentStamina = 100.0f; // 현재 스태미나

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
	float MaxStamina = 100.0f; // 최대 스태미나

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
	float StaminaDrainRate = 20.0f; // 초당 스태미나 소모량 (5초에 100 소모)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
	float StaminaRegenRate = 10.0f; // 초당 스태미나 회복량 (10초에 100 회복)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
	float MinStaminaToSprint = 10.0f; // 달리기 시작 최소 스태미나

	// === 달리기 함수 ===
	void StartSprinting();
	void StopSprinting();
	void UpdateStamina(float DeltaTime);

	// 포탑 스폰 모드
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret")
	bool bIsTurretSpawnMode = false;

	void ToggleTurretSpawnMode(); // F키로 포탑 스폰 모드 토글

	// 포탑 설치 완료 콜백
	UFUNCTION()
	void OnTurretPlacementComplete();

	// 업그레이드 시스템을 위한 Getter/Setter 함수들
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	float GetGunDamage() const { return GunDamage; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void SetGunDamage(float NewDamage) { GunDamage = NewDamage; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	float GetMeleeDamage() const { return MeleeDamage; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void SetMeleeDamage(float NewDamage) { MeleeDamage = NewDamage; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	float GetMeleeRange() const { return MeleeRange; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void SetMeleeRange(float NewRange) { MeleeRange = NewRange; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	float GetReloadDuration() const { return ReloadDuration; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void SetReloadDuration(float NewDuration) { ReloadDuration = NewDuration; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	int32 GetMaxAmmo() const { return MaxAmmo; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void SetMaxAmmo(int32 NewMaxAmmo) { MaxAmmo = NewMaxAmmo; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	float GetGrenadeFireCooldown() const { return GrenadeFireCooldown; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void SetGrenadeFireCooldown(float NewCooldown) { GrenadeFireCooldown = NewCooldown; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	float GetDefaultWalkSpeed() const { return DefaultWalkSpeed; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void SetDefaultWalkSpeed(float NewSpeed) { DefaultWalkSpeed = NewSpeed; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	float GetAimingWalkSpeed() const { return AimingWalkSpeed; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void SetAimingWalkSpeed(float NewSpeed) { AimingWalkSpeed = NewSpeed; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	bool GetHaveWing() const { return haveWing; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void SetHaveWing(bool bNewHaveWing) { haveWing = bNewHaveWing; }

	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	float GetGrenadeDamage() const { return GrenadeDamage; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void SetGrenadeDamage(float NewDamage) { GrenadeDamage = NewDamage; }

	// 달리기/스태미나 관련 업그레이드 함수들
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	float GetMaxStamina() const { return MaxStamina; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void SetMaxStamina(float NewMaxStamina) { MaxStamina = NewMaxStamina; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	float GetCurrentStamina() const { return CurrentStamina; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	float GetSprintSpeedMultiplier() const { return SprintSpeedMultiplier; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void SetSprintSpeedMultiplier(float NewMultiplier) { SprintSpeedMultiplier = NewMultiplier; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	float GetStaminaDrainRate() const { return StaminaDrainRate; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void SetStaminaDrainRate(float NewRate) { StaminaDrainRate = NewRate; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	float GetStaminaRegenRate() const { return StaminaRegenRate; }
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void SetStaminaRegenRate(float NewRate) { StaminaRegenRate = NewRate; }
};
