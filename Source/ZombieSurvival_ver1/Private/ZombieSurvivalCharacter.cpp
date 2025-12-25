// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieSurvivalCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"
#include "Gun.h"
#include "Kismet/GameplayStatics.h"
#include "Grenade.h"
#include "GrenadeLauncher.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "EnemyBase.h" 
#include "TurretBuilderComponent.h"
#include "UpgradeManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Engine.h"
#include "ZombieSurvivalGameMode.h"

// Sets default values
AZombieSurvivalCharacter::AZombieSurvivalCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 450.f;
	GetCharacterMovement()->AirControl = 0.2f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 75.f, 10.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// 포탑 빌더 컴포넌트 생성
	TurretBuilder = CreateDefaultSubobject<UTurretBuilderComponent>(TEXT("TurretBuilder"));

	// 업그레이드 매니저 컴포넌트 생성
	UpgradeManager = CreateDefaultSubobject<UUpgradeManager>(TEXT("UpgradeManager"));

	// 플레이어 태그 추가 (AI가 플레이어를 인식할 수 있도록)
	Tags.Add(FName("Player"));
}

// Called when the game starts or when spawned
void AZombieSurvivalCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth;
	CurrentStamina = MaxStamina; // 스태미나 초기화

	if (GunClass)
	{
		EquippedGun = GetWorld()->SpawnActor<AGun>(GunClass);
		if (EquippedGun)
		{
			EquippedGun->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("RightHandSocket"));
			EquippedGun->SetOwner(this);
			EquippedGun->SetActorHiddenInGame(false);
		}
	}

	// 포탑 설치 완료 이벤트 구독
	if (TurretBuilder)
	{
		TurretBuilder->OnTurretPlaced.AddDynamic(this, &AZombieSurvivalCharacter::OnTurretPlacementComplete);
	}
}

void AZombieSurvivalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AZombieSurvivalCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AZombieSurvivalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AZombieSurvivalCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AZombieSurvivalCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AZombieSurvivalCharacter::LookUpAtRate);

	PlayerInputComponent->BindTouch(IE_Pressed, this, &AZombieSurvivalCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AZombieSurvivalCharacter::TouchStopped);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AZombieSurvivalCharacter::OnResetVR);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AZombieSurvivalCharacter::StartFiring);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AZombieSurvivalCharacter::StopFiring);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AZombieSurvivalCharacter::StartAiming);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AZombieSurvivalCharacter::StopAiming);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AZombieSurvivalCharacter::Reload);

	PlayerInputComponent->BindAction("ToggleTurretMode", IE_Pressed, this, &AZombieSurvivalCharacter::ToggleTurretSpawnMode);

	// 달리기 입력 바인딩
	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &AZombieSurvivalCharacter::StartSprinting);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &AZombieSurvivalCharacter::StopSprinting);

	PlayerInputComponent->BindAxis("MouseScroll", this, &AZombieSurvivalCharacter::HandleMouseWheelSwitch);

	PlayerInputComponent->BindAction("Selectweapon1", IE_Pressed, this, &AZombieSurvivalCharacter::SelectGun);
	PlayerInputComponent->BindAction("Selectweapon2", IE_Pressed, this, &AZombieSurvivalCharacter::SelectLauncher);
	PlayerInputComponent->BindAction("Selectweapon3", IE_Pressed, this, &AZombieSurvivalCharacter::SelectMelee);

	// P키로 게임 정지/재개
	PlayerInputComponent->BindAction("GameStop", IE_Pressed, this, &AZombieSurvivalCharacter::TogglePause);
}

void AZombieSurvivalCharacter::FireWeapon()
{
	// 포탑 스폰 모드일 때는 포탑 설치
	if (bIsTurretSpawnMode)
	{
		if (TurretBuilder)
		{
			TurretBuilder->ConfirmPlacement();
		}
		return;
	}

	// 1번 무기 (권총) 사용 시
	if (CurrentWeaponMode == 1)
	{
		// 탄약이 없거나 재장전 중이면
		if (bIsReloading || CurrentAmmo <= 0)
		{
			// 탄약이 0이고 아직 재장전 중이 아니면 자동으로 재장전 시작
			if (CurrentAmmo <= 0 && !bIsReloading)
			{
				Reload();
			}
			return;
		}

		CurrentAmmo--; // 한 발 소모

		// 발사 사운드 재생
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}

		// 카메라 기준으로 시선 방향 추적 (조준 위치 구함)
		FVector CamLoc = FollowCamera->GetComponentLocation();
		FRotator CamRot = FollowCamera->GetComponentRotation();
		FVector CamDir = CamRot.Vector();
		FVector TraceEnd = CamLoc + CamDir * 10000.f;

		// 조준 위치에서 첫 충돌 지점 찾기
		FHitResult SightHit;
		FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, this);
		TraceParams.AddIgnoredActor(this);

		bool bSightHit = GetWorld()->LineTraceSingleByChannel(
			SightHit, CamLoc, TraceEnd, ECC_Visibility, TraceParams
		);

		FVector TargetLocation = bSightHit ? SightHit.ImpactPoint : TraceEnd;

		// 총구 위치 → 조준 지점 방향으로 두 번째 LineTrace 수행
		FVector MuzzleLoc = EquippedGun && EquippedGun->MuzzleLocation
			? EquippedGun->MuzzleLocation->GetComponentLocation()
			: FollowCamera->GetComponentLocation();

		FVector FinalDir = (TargetLocation - MuzzleLoc).GetSafeNormal();
		FVector End = MuzzleLoc + FinalDir * 10000.f;

		FHitResult ShotHit;
		FCollisionQueryParams ShotParams(SCENE_QUERY_STAT(Shot), true, this);
		ShotParams.AddIgnoredActor(this);

		bool bShotHit = GetWorld()->LineTraceSingleByChannel(
			ShotHit, MuzzleLoc, End, ECC_Visibility, ShotParams
		);

		FVector FinalImpact = bShotHit ? ShotHit.ImpactPoint : End;

		// 디버그용 빨간 선 표시
		DrawDebugLine(GetWorld(), MuzzleLoc, FinalImpact, FColor::Red, false, 1.0f, 0, 2.0f);

		// 적에게 맞았으면 데미지 처리
		if (bShotHit)
		{
			AEnemyBase* HitEnemy = Cast<AEnemyBase>(ShotHit.GetActor());
			if (HitEnemy)
			{
				HitEnemy->TakeDamage(GunDamage);
			}
		}

		// 반동 처리
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			TargetControlRotation = PC->GetControlRotation();
			PC->AddPitchInput(-RecoilAmount);
			bIsRecoiling = true;
		}
	}

	// 2번 무기 (유탄 발사기) 사용 시
	else if (CurrentWeaponMode == 2)
	{
		if (!GrenadeClass) return;

		// 탄약 조건 및 재장전 확인
		if (CurrentGrenadeAmmo <= 0 || bIsGrenadeReloading || !bCanLaunchGrenade)
		{
			if (CurrentGrenadeAmmo <= 0 && !bIsGrenadeReloading)
			{
				ReloadGrenade();
			}
			return;
		}

		CurrentGrenadeAmmo--;
		bCanLaunchGrenade = false;

		// 쿨다운 설정
		GetWorld()->GetTimerManager().SetTimer(
			GrenadeCooldownTimer,
			[this]() { bCanLaunchGrenade = true; },
			GrenadeFireCooldown,
			false
		);

		// 카메라 기준 시선 방향으로 조준점 계산
		FVector CamLoc = FollowCamera->GetComponentLocation();
		FRotator CamRot = FollowCamera->GetComponentRotation();
		FVector TraceEnd = CamLoc + CamRot.Vector() * 10000.f;

		FHitResult SightHit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		bool bHit = GetWorld()->LineTraceSingleByChannel(SightHit, CamLoc, TraceEnd, ECC_Visibility, Params);
		FVector TargetLoc = bHit ? SightHit.ImpactPoint : TraceEnd;

		// 유탄 스폰 위치 보정 (카메라 기준 약간 앞쪽으로)
		FVector MuzzleLoc = EquippedLauncher && EquippedLauncher->MuzzleLocation
			? EquippedLauncher->MuzzleLocation->GetComponentLocation()
			: FollowCamera->GetComponentLocation();

		MuzzleLoc += FollowCamera->GetForwardVector() * 50.f;

		// 목표 방향 계산 + 위로 15도 보정
		FVector LaunchDir = (TargetLoc - MuzzleLoc).GetSafeNormal();
		FRotator LaunchRot = LaunchDir.Rotation();
		LaunchRot.Pitch += 15.f;

		FVector FinalDir = LaunchRot.Vector();

		// 유탄 생성
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		AGrenade* Grenade = GetWorld()->SpawnActor<AGrenade>(
			GrenadeClass, MuzzleLoc, LaunchRot, SpawnParams
		);

		if (Grenade && Grenade->ProjectileMovement)
		{
			// 플레이어의 유탄 데미지 적용
			Grenade->ExplosionDamage = GrenadeDamage;
			
			Grenade->ProjectileMovement->Velocity = FinalDir * 1500.f;
			Grenade->ProjectileMovement->UpdateComponentVelocity();
		}
	}

	// 근접 무기
	else if (CurrentWeaponMode == 3)
	{
		PerformMeleeAttack();
	}
}


void AZombieSurvivalCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AZombieSurvivalCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AZombieSurvivalCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AZombieSurvivalCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AZombieSurvivalCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AZombieSurvivalCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0.f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRot(0.f, Rotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AZombieSurvivalCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0.f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRot(0.f, Rotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AZombieSurvivalCharacter::Jump()
{
	// 공중에 있을 때만 이단 점프 가능 (윙 부츠 보유 시)
	if (GetCharacterMovement()->IsFalling() && haveWing == true)
	{
		if (bCanAirDash)
		{
			PerformAirDash(); // 공중 대시 형태의 이단 점프
			bCanAirDash = false; // 이단 점프는 1회만 허용
		}
	}
	else
	{
		// 지면 위에서는 일반 점프
		Super::Jump();
	}
}


void AZombieSurvivalCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	bCanAirDash = true; // 착지 시 이단 점프 회복
}


void AZombieSurvivalCharacter::PerformAirDash()
{
	// 현재 속도를 복사
	FVector NewVelocity = GetCharacterMovement()->Velocity;

	// 수직 속도 제거 (하강 속도 제거)
	NewVelocity.Z = 0.f;

	// 수직 점프 힘 부여
	NewVelocity.Z += 450.f; // 점프 높이 조절 가능

	// 최종 속도 적용
	GetCharacterMovement()->Velocity = NewVelocity;
}


void AZombieSurvivalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 스태미나 업데이트
	UpdateStamina(DeltaTime);

	// 우클릭시 FOV(시야각)를 점진적으로 좁힘 (줌 효과)
	float TargetFOV = bIsAiming ? ZoomedFOV : DefaultFOV;
	float NewFOV = FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);
	FollowCamera->SetFieldOfView(NewFOV);
}

void AZombieSurvivalCharacter::StartAiming()
{
	bIsAiming = true; // 조준 중 상태로 설정
	
	// 조준 중에는 달리기 자동 중지
	if (bIsSprinting)
	{
		StopSprinting();
	}
	
	GetCharacterMovement()->MaxWalkSpeed = AimingWalkSpeed; // 조준 중에는 이동속도 감소
}

void AZombieSurvivalCharacter::StopAiming()
{
	bIsAiming = false; // 조준 해제
	
	// 조준 해제 시 달리기 상태가 아니면 기본 속도로 복구
	if (!bIsSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed; // 기본 이동속도로 복구
	}
}

void AZombieSurvivalCharacter::StartFiring()
{
	FireWeapon(); // 즉시 1회 발사

	// 연사 타이머 시작 (FireRate 간격마다 FireWeapon 반복 호출)
	GetWorld()->GetTimerManager().SetTimer(
		FireTimerHandle,
		this,
		&AZombieSurvivalCharacter::FireWeapon,
		FireRate,
		true
	);
}

void AZombieSurvivalCharacter::StopFiring()
{
	// 연사 중단 (타이머 제거)
	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
}

void AZombieSurvivalCharacter::Reload()
{
	if (bIsReloading || CurrentAmmo == MaxAmmo) return;

	bIsReloading = true;

	if (ReloadSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
	}

	// 일정 시간 후 재장전 완료
	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&AZombieSurvivalCharacter::FinishReloading,
		ReloadDuration,
		false
	);
}


void AZombieSurvivalCharacter::FinishReloading()
{
	bIsReloading = false;
	CurrentAmmo = MaxAmmo; // 탄약 가득 채움
}

void AZombieSurvivalCharacter::SwitchWeaponMode(int32 NewMode)
{
	// 무기 종류에 따라 외형 토글
	if (EquippedGun)
		EquippedGun->SetActorHiddenInGame(NewMode != 1);

	if (NewMode == 2)
	{
		if (!EquippedLauncher && LauncherClass)
		{
			EquippedLauncher = GetWorld()->SpawnActor<AGrenadeLauncher>(LauncherClass);
			if (EquippedLauncher)
			{
				EquippedLauncher->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("RightHandSocket"));
				EquippedLauncher->SetOwner(this);
			}
		}
		if (EquippedLauncher)
			EquippedLauncher->SetActorHiddenInGame(false);
	}
	else if (EquippedLauncher)
	{
		EquippedLauncher->SetActorHiddenInGame(true);
	}

	if (SwitchWeaponSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SwitchWeaponSound, GetActorLocation());
	}

	CurrentWeaponMode = NewMode;

	// 포탑 스폰 모드일 때 포탑 클래스도 업데이트
	if (bIsTurretSpawnMode && TurretBuilder)
	{
		TSubclassOf<AActor> SelectedTurretClass = nullptr;
		switch (CurrentWeaponMode)
		{
			case 1: // 권총
				SelectedTurretClass = GunTurretClass;
				UE_LOG(LogTemp, Warning, TEXT("포탑 모드에서 권총 포탑으로 변경"));
				break;
			case 2: // 유탄발사기
				SelectedTurretClass = GrenadeTurretClass;
				UE_LOG(LogTemp, Warning, TEXT("포탑 모드에서 유탄 포탑으로 변경"));
				break;
			case 3: // 근접무기
				SelectedTurretClass = MeleeTurretClass;
				UE_LOG(LogTemp, Warning, TEXT("포탑 모드에서 근접 포탑으로 변경"));
				break;
		}
		
		if (SelectedTurretClass)
		{
			TurretBuilder->TurretClass = SelectedTurretClass;
		}
	}
}


void AZombieSurvivalCharacter::HandleMouseWheelSwitch(float Value)
{
	// 휠 입력 없으면 무시
	if (FMath::IsNearlyZero(Value))
		return;

	int32 NewMode = CurrentWeaponMode;

	if (Value > 0.f)
	{
		// 위로 굴리면 +1, 3 넘어가면 1로 순환
		NewMode = CurrentWeaponMode + 1;
		if (NewMode > 3)
			NewMode = 1;
	}
	else // Value < 0
	{
		// 아래로 굴리면 -1, 1 아래로 내려가면 3으로 순환
		NewMode = CurrentWeaponMode - 1;
		if (NewMode < 1)
			NewMode = 3;
	}

	if (NewMode != CurrentWeaponMode)
	{
		SwitchWeaponMode(NewMode);
	}
}


void AZombieSurvivalCharacter::SelectGun()
{
	SwitchWeaponMode(1);
}

void AZombieSurvivalCharacter::SelectLauncher()
{
	SwitchWeaponMode(2);
}

void AZombieSurvivalCharacter::SelectMelee()
{
	SwitchWeaponMode(3);
}


// 재장전 함수
void AZombieSurvivalCharacter::ReloadGrenade()
{
	if (bIsGrenadeReloading || CurrentGrenadeAmmo == MaxGrenadeAmmo) return;

	bIsGrenadeReloading = true;
	if (ReloadSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
	}

	GetWorld()->GetTimerManager().SetTimer(
		GrenadeReloadTimer, this,
		&AZombieSurvivalCharacter::FinishReloadingGrenade,
		GrenadeReloadDuration, false
	);
}

void AZombieSurvivalCharacter::FinishReloadingGrenade()
{
	CurrentGrenadeAmmo = MaxGrenadeAmmo;
	bIsGrenadeReloading = false;
}

void AZombieSurvivalCharacter::TakeDamage(float Damage)
{
	if (!IsAlive()) return;

	CurrentHealth -= Damage;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.f, MaxHealth);

	UE_LOG(LogTemp, Warning, TEXT("Player took %.1f damage. Current Health: %.1f"), Damage, CurrentHealth);

	// 피격 사운드 재생
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}

	if (!IsAlive())
	{
		UE_LOG(LogTemp, Warning, TEXT("You Die."));
		Destroy(); // 사망 처리
	}
}

void AZombieSurvivalCharacter::ToggleTurretSpawnMode()
{
	UE_LOG(LogTemp, Warning, TEXT("ToggleTurretSpawnMode 함수 호출됨!"));
	
	// TurretBuilder 컴포넌트 존재 확인
	if (!TurretBuilder)
	{
		UE_LOG(LogTemp, Error, TEXT("TurretBuilder 컴포넌트가 null입니다!"));
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("TurretBuilder 컴포넌트 존재 확인됨"));
	
	// 포탑 클래스들 설정 상태 확인
	UE_LOG(LogTemp, Warning, TEXT("포탑 클래스 상태 - Gun: %s, Grenade: %s, Melee: %s"), 
		GunTurretClass ? TEXT("설정됨") : TEXT("null"),
		GrenadeTurretClass ? TEXT("설정됨") : TEXT("null"),
		MeleeTurretClass ? TEXT("설정됨") : TEXT("null"));
	
	bIsTurretSpawnMode = !bIsTurretSpawnMode;
	
	if (bIsTurretSpawnMode)
	{
		// 포탑 스폰 모드 진입
		UE_LOG(LogTemp, Warning, TEXT("포탑 스폰 모드 활성화 - 무기 모드: %d"), CurrentWeaponMode);
		
		// 현재 무기에 따라 포탑 클래스 설정
		TSubclassOf<AActor> SelectedTurretClass = nullptr;
		switch (CurrentWeaponMode)
		{
			case 1: // 권총
				SelectedTurretClass = GunTurretClass;
				UE_LOG(LogTemp, Warning, TEXT("권총 포탑 선택"));
				break;
			case 2: // 유탄발사기
				SelectedTurretClass = GrenadeTurretClass;
				UE_LOG(LogTemp, Warning, TEXT("유탄 포탑 선택"));
				break;
			case 3: // 근접무기
				SelectedTurretClass = MeleeTurretClass;
				UE_LOG(LogTemp, Warning, TEXT("근접 포탑 선택"));
				break;
			default:
				UE_LOG(LogTemp, Error, TEXT("알 수 없는 무기 모드: %d"), CurrentWeaponMode);
				bIsTurretSpawnMode = false; // 모드 해제
				return;
		}
		
		// TurretBuilder에 포탑 클래스 설정 및 배치 모드 활성화
		if (TurretBuilder && SelectedTurretClass)
		{
			TurretBuilder->TurretClass = SelectedTurretClass;
			TurretBuilder->SetPlacing(true);
			UE_LOG(LogTemp, Warning, TEXT("TurretBuilder 설정 완료 및 배치 모드 활성화"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("TurretBuilder 또는 포탑 클래스가 없습니다 - TurretBuilder: %s, SelectedTurretClass: %s"), 
				TurretBuilder ? TEXT("존재") : TEXT("null"),
				SelectedTurretClass ? TEXT("존재") : TEXT("null"));
			bIsTurretSpawnMode = false; // 모드 해제
		}
	}
	else
	{
		// 포탑 스폰 모드 해제
		UE_LOG(LogTemp, Warning, TEXT("포탑 스폰 모드 비활성화"));
		
		if (TurretBuilder)
		{
			TurretBuilder->SetPlacing(false);
		}
	}
}

void AZombieSurvivalCharacter::OnTurretPlacementComplete()
{
	// 포탑 설치 완료 시 자동으로 포탑 스폰 모드 해제
	bIsTurretSpawnMode = false;
	UE_LOG(LogTemp, Warning, TEXT("포탑 설치 완료 - 포탑 스폰 모드 자동 해제"));
}

void AZombieSurvivalCharacter::PerformMeleeAttack()
{
	// 쿨다운 중이면 공격 불가
	if (!bCanMeleeAttack) return;
	
	bCanMeleeAttack = false;
	
	// 쿨다운 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(
		MeleeAttackCooldownTimer,
		[this]() { bCanMeleeAttack = true; },
		MeleeAttackCooldown,
		false
	);
	
	// 공격 사운드 재생 (있을 경우)
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}
	
	// 플레이어 위치와 방향 구하기
	FVector PlayerLocation = GetActorLocation();
	FVector ForwardVector = GetActorForwardVector();
	
	// 부채꼴 공격 범위 내의 모든 적 찾기
	TArray<AActor*> FoundActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	
	// 구체 범위 내의 모든 액터 검색
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		PlayerLocation,
		MeleeRange,
		ObjectTypes,
		nullptr,
		ActorsToIgnore,
		FoundActors
	);
	
	// 부채꼴 범위 내의 적들에게 데미지 적용
	for (AActor* Actor : FoundActors)
	{
		AEnemyBase* Enemy = Cast<AEnemyBase>(Actor);
		if (!Enemy) continue;
		
		// 적까지의 방향 벡터
		FVector ToEnemy = (Enemy->GetActorLocation() - PlayerLocation).GetSafeNormal();
		
		// 플레이어가 바라보는 방향과의 각도 계산
		float DotProduct = FVector::DotProduct(ForwardVector, ToEnemy);
		float AngleInRadians = FMath::Acos(DotProduct);
		float AngleInDegrees = FMath::RadiansToDegrees(AngleInRadians);
		
		// 부채꼴 각도 범위 내에 있는지 확인 (180도면 좌우 90도씩)
		if (AngleInDegrees <= MeleeAngle / 2.0f)
		{
			// 데미지 적용
			Enemy->TakeDamage(MeleeDamage);
			UE_LOG(LogTemp, Warning, TEXT("근접 공격으로 %s에게 %.1f 데미지"), *Enemy->GetName(), MeleeDamage);
		}
	}
	
	// 공격 범위 시각화 (디버그용)
	DrawMeleeAttackRange(PlayerLocation, ForwardVector);
}

void AZombieSurvivalCharacter::DrawMeleeAttackRange(const FVector& Center, const FVector& ForwardVector)
{
	// 부채꼴 시각화를 위한 여러 선분 그리기
	int32 NumLines = 20; // 부채꼴을 구성할 선분 개수
	float HalfAngle = FMath::DegreesToRadians(MeleeAngle / 2.0f);
	
	// 중앙 선 (플레이어가 바라보는 방향)
	FVector CenterEnd = Center + ForwardVector * MeleeRange;
	DrawDebugLine(GetWorld(), Center, CenterEnd, FColor::Red, false, 2.0f, 0, 3.0f);
	
	// 부채꼴 가장자리 선들
	for (int32 i = 0; i <= NumLines; ++i)
	{
		float CurrentAngle = -HalfAngle + (HalfAngle * 2.0f * i / NumLines);
		
		// ForwardVector를 CurrentAngle만큼 회전
		FVector RotatedVector = ForwardVector.RotateAngleAxis(FMath::RadiansToDegrees(CurrentAngle), FVector::UpVector);
		FVector LineEnd = Center + RotatedVector * MeleeRange;
		
		// 선 그리기
		FColor LineColor = (i == 0 || i == NumLines) ? FColor::Yellow : FColor::Orange;
		DrawDebugLine(GetWorld(), Center, LineEnd, LineColor, false, 2.0f, 0, 2.0f);
	}
	
	// 부채꼴 호 그리기 (선택사항)
	FVector RightVector = ForwardVector.RotateAngleAxis(MeleeAngle / 2.0f, FVector::UpVector);
	FVector LeftVector = ForwardVector.RotateAngleAxis(-MeleeAngle / 2.0f, FVector::UpVector);
	
	// 호를 작은 선분들로 근사
	int32 ArcSegments = 15;
	for (int32 i = 0; i < ArcSegments; ++i)
	{
		float StartAngle = -HalfAngle + (HalfAngle * 2.0f * i / ArcSegments);
		float EndAngle = -HalfAngle + (HalfAngle * 2.0f * (i + 1) / ArcSegments);
		
		FVector StartPoint = Center + ForwardVector.RotateAngleAxis(FMath::RadiansToDegrees(StartAngle), FVector::UpVector) * MeleeRange;
		FVector EndPoint = Center + ForwardVector.RotateAngleAxis(FMath::RadiansToDegrees(EndAngle), FVector::UpVector) * MeleeRange;
		
		DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Green, false, 2.0f, 0, 2.0f);
	}
}

// === 달리기 시스템 구현 ===

void AZombieSurvivalCharacter::StartSprinting()
{
	// 스태미나가 최소 요구량보다 적으면 달리기 불가
	if (CurrentStamina < MinStaminaToSprint)
	{
		return;
	}

	bIsSprinting = true;
	
	// 이동 속도 증가
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->MaxWalkSpeed = DefaultWalkSpeed * SprintSpeedMultiplier;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("달리기 시작 - 속도: %.1f"), DefaultWalkSpeed * SprintSpeedMultiplier);
}

void AZombieSurvivalCharacter::StopSprinting()
{
	bIsSprinting = false;
	
	// 이동 속도 원상복구 (조준 중이면 조준 속도, 아니면 기본 속도)
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->MaxWalkSpeed = bIsAiming ? AimingWalkSpeed : DefaultWalkSpeed;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("달리기 종료 - 속도: %.1f"), bIsAiming ? AimingWalkSpeed : DefaultWalkSpeed);
}

void AZombieSurvivalCharacter::UpdateStamina(float DeltaTime)
{
	if (bIsSprinting)
	{
		// 달리는 중일 때 스태미나 소모
		CurrentStamina -= StaminaDrainRate * DeltaTime;
		CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);
		
		// 스태미나가 0이 되면 강제로 달리기 중지
		if (CurrentStamina <= 0.0f)
		{
			StopSprinting();
			UE_LOG(LogTemp, Warning, TEXT("스태미나 고갈 - 달리기 자동 중지"));
		}
	}
	else
	{
		// 달리지 않을 때 스태미나 회복
		CurrentStamina += StaminaRegenRate * DeltaTime;
		CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);
	}
	
	// 스태미나 UI 업데이트용 로그 (5초마다)
	static float LastStaminaLogTime = 0.f;
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastStaminaLogTime > 5.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("스태미나: %.1f/%.1f (%.1f%%), 달리기: %s"), 
			CurrentStamina, MaxStamina, (CurrentStamina / MaxStamina) * 100.0f,
			bIsSprinting ? TEXT("ON") : TEXT("OFF"));
		LastStaminaLogTime = CurrentTime;
	}
}

void AZombieSurvivalCharacter::TogglePause()
{
	// ZombieSurvivalGameMode 가져오기
	AZombieSurvivalGameMode* ZombieGameMode = Cast<AZombieSurvivalGameMode>(GetWorld()->GetAuthGameMode());
	if (ZombieGameMode)
	{
		// 현재 정지 상태 확인 후 토글
		if (ZombieGameMode->IsGamePaused())
		{
			UE_LOG(LogTemp, Warning, TEXT("Player requested game resume"));
			ZombieGameMode->GameResume();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Player requested game pause"));
			ZombieGameMode->GameStop();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get ZombieSurvivalGameMode"));
	}
}

