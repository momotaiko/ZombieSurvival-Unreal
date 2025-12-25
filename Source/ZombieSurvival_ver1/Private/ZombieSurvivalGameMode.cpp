// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieSurvivalGameMode.h"
#include "ZombieSurvivalCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "UpgradeDataTypes.h"
#include "UpgradeManager.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/HUD.h"  

AZombieSurvivalGameMode::AZombieSurvivalGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("Blueprint'/Game/Blueprints/Character/BP_ThirdPersonCharacter'"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerBP is missing"))
	}

	static ConstructorHelpers::FClassFinder<AHUD> HUDBPClass(TEXT("/Game/Blueprints/UI/NewHUD"));
	if (HUDBPClass.Succeeded())
	{
		HUDClass = HUDBPClass.Class;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HUD Blueprint class '/Game/Blueprints/UI/BP_MyGameHUD.BP_MyGameHUD_C' not found."));
	}

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// 기본값 설정
	KillsRequiredForUpgrade = 20; // 5킬마다 업그레이드
	CurrentUpgradeWidget = nullptr;
	
	// 정지화면 관련 초기화
	CurrentPauseMenuWidget = nullptr;
	bIsGamePaused = false;
}


void AZombieSurvivalGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!CurrentUIData)
	{
		CurrentUIData = NewObject<UUIData>(this);
	}

	Player = Cast<AZombieSurvivalCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
}

void AZombieSurvivalGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	PushUIData();
}

void AZombieSurvivalGameMode::AddKillCount()
{
	UE_LOG(LogTemp, Warning, TEXT("AddKillCount"), KillCount);
	KillCount++;

	// 20 100 200 400 킬 시 업그레이드 가능
	if (KillCount == 20 || KillCount == 100 || KillCount == 200 || KillCount == 300 || KillCount == 400)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reached for upgrade"), KillCount);
		PauseGameForUpgrade();
	}
}

void AZombieSurvivalGameMode::PauseGameForUpgrade()
{
	// 게임을 일시정지하고 업그레이드 UI를 표시
	UGameplayStatics::SetGamePaused(GetWorld(), true);
	
	// 업그레이드 옵션 생성 및 UI 표시
	ShowUpgradeUI();
}

TArray<FUpgradeData> AZombieSurvivalGameMode::GenerateUpgradeOptions()
{
	TArray<FUpgradeData> UpgradeOptions;
	if (!UpgradeDataTable)
	{
		return UpgradeOptions;
	}

	// 현재 사용 가능한 모든 업그레이드를 가져옴
	TArray<FUpgradeData*> AvailableUpgrades;
	UpgradeDataTable->GetAllRows<FUpgradeData>("", AvailableUpgrades);

	// UpgradeManager 참조 가져오기
	AZombieSurvivalCharacter* PlayerCharacter = Cast<AZombieSurvivalCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	UUpgradeManager* UpgradeManager = nullptr;
	if (PlayerCharacter)
	{
		UpgradeManager = PlayerCharacter->UpgradeManager;
	}

	// 현재 킬 카운트와 조건에 맞는 업그레이드만 필터링
	TArray<FUpgradeData*> FilteredUpgrades;
	TArray<FName> RowNames = UpgradeDataTable->GetRowNames();
	
	for (int32 i = 0; i < AvailableUpgrades.Num(); i++)
	{
		FUpgradeData* Upgrade = AvailableUpgrades[i];
		if (!Upgrade || i >= RowNames.Num()) continue;
		
		FName CurrentRowName = RowNames[i];
		
		// 기본 조건 체크 (킬 카운트)
		if (KillCount < Upgrade->MinKillCount || KillCount > Upgrade->MaxKillCount)
		{
			continue;
		}
		
		// 이미 획득한 특수 업그레이드 제외
		if (Upgrade->UpgradeType == EUpgradeType::SpecialUpgrade)
		{
			if (AcquiredUpgrades.Contains(CurrentRowName))
			{
				UE_LOG(LogTemp, Warning, TEXT("Special upgrade already acquired, skipping: %s"), *CurrentRowName.ToString());
				continue;
			}
		}
		else
		{
			// 일반 업그레이드: 최대 레벨 체크
			if (UpgradeManager)
			{
				int32 CurrentLevel = UpgradeManager->GetUpgradeLevel(Upgrade->UpgradeType);
				if (CurrentLevel >= Upgrade->MaxUpgradeLevel)
				{
					UE_LOG(LogTemp, Warning, TEXT("Upgrade at max level, skipping: %s (Level: %d/%d)"), 
						*CurrentRowName.ToString(), CurrentLevel, Upgrade->MaxUpgradeLevel);
					continue;
				}
			}
		}
		
		// 선행 조건 체크
		bool bMeetsRequirements = true;
		for (const FName& RequiredUpgrade : Upgrade->RequiredUpgrades)
		{
			if (!AcquiredUpgrades.Contains(RequiredUpgrade))
			{
				UE_LOG(LogTemp, Warning, TEXT("Required upgrade not met: %s needs %s"), 
					*CurrentRowName.ToString(), *RequiredUpgrade.ToString());
				bMeetsRequirements = false;
				break;
			}
		}

		if (bMeetsRequirements)
		{
			UE_LOG(LogTemp, Warning, TEXT("Upgrade added to pool: %s"), *CurrentRowName.ToString());
			FilteredUpgrades.Add(Upgrade);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Total filtered upgrades: %d"), FilteredUpgrades.Num());

	// 랜덤하게 3개 선택
	while (UpgradeOptions.Num() < 3 && FilteredUpgrades.Num() > 0)
	{
		// 희귀도를 기반으로 가중치 부여
		float TotalWeight = 0.0f;
		for (const FUpgradeData* Upgrade : FilteredUpgrades)
		{
			TotalWeight += (100.0f - Upgrade->Rarity); // 희귀도가 낮을수록 더 자주 등장
		}

		float RandomValue = FMath::RandRange(0.0f, TotalWeight);
		float CurrentWeight = 0.0f;
		
		for (int32 i = 0; i < FilteredUpgrades.Num(); i++)
		{
			CurrentWeight += (100.0f - FilteredUpgrades[i]->Rarity);
			if (RandomValue <= CurrentWeight)
			{
				UpgradeOptions.Add(*FilteredUpgrades[i]);
				FilteredUpgrades.RemoveAt(i);
				break;
			}
		}
	}

	return UpgradeOptions;
}

void AZombieSurvivalGameMode::ApplyUpgrade(const FUpgradeData& SelectedUpgrade)
{
	AZombieSurvivalCharacter* PlayerCharacter = Cast<AZombieSurvivalCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (PlayerCharacter && PlayerCharacter->UpgradeManager)
	{
		// 업그레이드 매니저를 통해 업그레이드 적용
		FUpgradeApplyResult Result = PlayerCharacter->UpgradeManager->ApplyUpgrade(SelectedUpgrade);
		
		if (Result.bSuccess)
		{
			// Row Name으로 획득한 업그레이드 기록 (Required Upgrades와 매칭하기 위해)
			FString RowNameString = TEXT("");
			
			// 데이터테이블에서 Row Name 찾기
			if (UpgradeDataTable)
			{
				TArray<FName> RowNames = UpgradeDataTable->GetRowNames();
				for (const FName& RowName : RowNames)
				{
					if (FUpgradeData* RowData = UpgradeDataTable->FindRow<FUpgradeData>(RowName, TEXT("")))
					{
						if (RowData->UpgradeName.ToString() == SelectedUpgrade.UpgradeName.ToString())
						{
							RowNameString = RowName.ToString();
							break;
						}
					}
				}
			}
			
			if (!RowNameString.IsEmpty())
			{
				FName RowNameAsName = FName(*RowNameString);
				AcquiredUpgrades.Add(RowNameAsName);
				UE_LOG(LogTemp, Warning, TEXT("Upgrade applied and recorded with Row Name: %s"), *RowNameString);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to find Row Name for upgrade: %s"), *SelectedUpgrade.UpgradeName.ToString());
			}
			
			UE_LOG(LogTemp, Warning, TEXT("Upgrade successfully applied: %s (Level: %d)"), 
				*SelectedUpgrade.UpgradeName.ToString(), Result.CurrentLevel);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to apply upgrade: %s. Error: %s"), 
				*SelectedUpgrade.UpgradeName.ToString(), *Result.ErrorMessage);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Player character or UpgradeManager not found"));
	}

	UGameplayStatics::SetGamePaused(GetWorld(), false);
}

void AZombieSurvivalGameMode::PushUIData()
{
	if (!Player)
	{
		Player = Cast<AZombieSurvivalCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
		if (!Player) return;
	}
	if (!CurrentUIData) return;

	/* 2) DTO 채우기 (플레이어가 가진 값 사용) */
	float MaxHealth = Player->MaxHealth;
	float CurrentHealth = Player->CurrentHealth;
	int32 RedCrystalCount = Player->RedCrystalCount;
	int32 GreenCrystalCount = Player->GreenCrystalCount;
	int32 BlueCrystalCount = Player->BlueCrystalCount;
	int32 Kills = Player->Kills;
	float Stamina = Player->GetCurrentStamina();
	float MaxStamina = Player->GetMaxStamina();

	CurrentUIData->MaxHealth = MaxHealth;
	CurrentUIData->CurrentHealth = CurrentHealth;
	CurrentUIData->Red = RedCrystalCount;
	CurrentUIData->Green = GreenCrystalCount;
	CurrentUIData->Blue = BlueCrystalCount;
	CurrentUIData->Kills = Kills;
	CurrentUIData->Stamina = Stamina;
	CurrentUIData->MaxStamina = MaxStamina;

	// 업그레이드 정보 추가
	if (Player->UpgradeManager)
	{
		UUpgradeManager* UpgradeManager = Player->UpgradeManager;
		
		// 일반 업그레이드 레벨 (0~5 또는 0~3)
		CurrentUIData->Gun = UpgradeManager->GetUpgradeLevel(EUpgradeType::Weapon1DamageIncrease);
		CurrentUIData->GrenadeLauncher = UpgradeManager->GetUpgradeLevel(EUpgradeType::Weapon2DamageIncrease);
		CurrentUIData->Sword = UpgradeManager->GetUpgradeLevel(EUpgradeType::Weapon3DamageIncrease);
		CurrentUIData->MoveSpeed = UpgradeManager->GetUpgradeLevel(EUpgradeType::MovementSpeedIncrease);
		CurrentUIData->AdditionalMaxHealth = UpgradeManager->GetUpgradeLevel(EUpgradeType::MaxHealthIncrease);
		
		// 특수 업그레이드 (Boolean)
		CurrentUIData->DoubleJump = UpgradeManager->HasSpecialUpgrade(FName("DoubleJump"));
		CurrentUIData->SwordRangeIncrease = UpgradeManager->HasSpecialUpgrade(FName("SwordRangeIncrease"));
		CurrentUIData->GunReloadSpeedIncrease = UpgradeManager->HasSpecialUpgrade(FName("GunReloadSpeedIncrease"));
		CurrentUIData->GunMaxAmmoIncrease = UpgradeManager->HasSpecialUpgrade(FName("GunMaxAmmoIncrease"));
		CurrentUIData->GrenadeExplosionRangeIncrease = UpgradeManager->HasSpecialUpgrade(FName("GrenadeExplosionRangeIncrease"));
		CurrentUIData->ZombieDropMoreCrystals = UpgradeManager->HasSpecialUpgrade(FName("ZombieDropMoreCrystals"));
		CurrentUIData->TurretRangeIncrease = UpgradeManager->HasSpecialUpgrade(FName("TurretRangeIncrease"));
		CurrentUIData->SelfHealingAbility = UpgradeManager->HasSpecialUpgrade(FName("SelfHealingAbility"));
		CurrentUIData->TurretCostReduction = UpgradeManager->HasSpecialUpgrade(FName("TurretCostReduction"));
		CurrentUIData->CrystalPickupRangeIncrease = UpgradeManager->HasSpecialUpgrade(FName("CrystalPickupRangeIncrease"));
	}
	else
	{
		// UpgradeManager가 없을 경우 기본값으로 설정
		CurrentUIData->Gun = 0;
		CurrentUIData->GrenadeLauncher = 0;
		CurrentUIData->Sword = 0;
		CurrentUIData->MoveSpeed = 0;
		CurrentUIData->AdditionalMaxHealth = 0;
		
		CurrentUIData->DoubleJump = false;
		CurrentUIData->SwordRangeIncrease = false;
		CurrentUIData->GunReloadSpeedIncrease = false;
		CurrentUIData->GunMaxAmmoIncrease = false;
		CurrentUIData->GrenadeExplosionRangeIncrease = false;
		CurrentUIData->ZombieDropMoreCrystals = false;
		CurrentUIData->TurretRangeIncrease = false;
		CurrentUIData->SelfHealingAbility = false;
		CurrentUIData->TurretCostReduction = false;
		CurrentUIData->CrystalPickupRangeIncrease = false;
	}

	/* 2) 모든 위젯에 전파 */
	OnUIDataChanged.Broadcast(CurrentUIData);
}

void AZombieSurvivalGameMode::ShowUpgradeUI()
{
	// 디버그: 현재 획득한 업그레이드 목록 출력
	UE_LOG(LogTemp, Warning, TEXT("=== Current Acquired Upgrades ==="));
	for (const FName& AcquiredUpgrade : AcquiredUpgrades)
	{
		UE_LOG(LogTemp, Warning, TEXT("Acquired: %s"), *AcquiredUpgrade.ToString());
	}
	UE_LOG(LogTemp, Warning, TEXT("Total acquired upgrades: %d"), AcquiredUpgrades.Num());
	UE_LOG(LogTemp, Warning, TEXT("Current kill count: %d"), KillCount);
	UE_LOG(LogTemp, Warning, TEXT("====================================="));

	// 업그레이드 옵션 생성
	CurrentUpgradeOptions = GenerateUpgradeOptions();
	
	if (CurrentUpgradeOptions.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No upgrade options available"));
		UGameplayStatics::SetGamePaused(GetWorld(), false);
		return;
	}

	// 위젯 클래스가 설정되어 있다면 UI 생성
	if (UpgradeWidgetClass)
	{
		if (!CurrentUpgradeWidget)
		{
			CurrentUpgradeWidget = CreateWidget<UUserWidget>(GetWorld(), UpgradeWidgetClass);
		}
		
		if (CurrentUpgradeWidget)
		{
			CurrentUpgradeWidget->AddToViewport();
			
			// 마우스 커서 표시
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC)
			{
				PC->SetShowMouseCursor(true);
				PC->SetInputMode(FInputModeUIOnly());
			}
			
			UE_LOG(LogTemp, Warning, TEXT("Upgrade UI shown with %d options"), CurrentUpgradeOptions.Num());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UpgradeWidgetClass not set in GameMode"));
	}
}

void AZombieSurvivalGameMode::HideUpgradeUI()
{
	if (CurrentUpgradeWidget)
	{
		CurrentUpgradeWidget->RemoveFromParent();
		
		// 마우스 커서 숨기기
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			PC->SetShowMouseCursor(false);
			PC->SetInputMode(FInputModeGameOnly());
		}
	}
	
	// 게임 재개
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	
	// 옵션 초기화
	CurrentUpgradeOptions.Empty();
	
	UE_LOG(LogTemp, Warning, TEXT("Upgrade UI hidden, game resumed"));
}

void AZombieSurvivalGameMode::SelectUpgrade(int32 OptionIndex)
{
	if (CurrentUpgradeOptions.IsValidIndex(OptionIndex))
	{
		const FUpgradeData& SelectedUpgrade = CurrentUpgradeOptions[OptionIndex];
		
		UE_LOG(LogTemp, Warning, TEXT("Player selected upgrade: %s"), *SelectedUpgrade.UpgradeName.ToString());
		
		// 업그레이드 적용
		ApplyUpgrade(SelectedUpgrade);
		
		// UI 숨기기
		HideUpgradeUI();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid upgrade option index: %d"), OptionIndex);
	}
}

void AZombieSurvivalGameMode::GameStop()
{
	// 이미 정지 상태이면 무시
	if (bIsGamePaused)
	{
		UE_LOG(LogTemp, Warning, TEXT("Game is already paused"));
		return;
	}

	// 게임 정지
	UGameplayStatics::SetGamePaused(GetWorld(), true);
	bIsGamePaused = true;

	UE_LOG(LogTemp, Warning, TEXT("Game paused"));

	// 정지화면 위젯 표시
	if (PauseMenuWidgetClass)
	{
		if (!CurrentPauseMenuWidget)
		{
			CurrentPauseMenuWidget = CreateWidget<UUserWidget>(GetWorld(), PauseMenuWidgetClass);
		}

		if (CurrentPauseMenuWidget)
		{
			CurrentPauseMenuWidget->AddToViewport();

			// 마우스 커서 표시 및 UI 모드 설정
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC)
			{
				PC->SetShowMouseCursor(true);
				PC->SetInputMode(FInputModeGameAndUI());
			}

			UE_LOG(LogTemp, Warning, TEXT("Pause menu widget displayed"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create pause menu widget"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PauseMenuWidgetClass not set in GameMode"));
	}
}

void AZombieSurvivalGameMode::GameResume()
{
	// 정지 상태가 아니면 무시
	if (!bIsGamePaused)
	{
		UE_LOG(LogTemp, Warning, TEXT("Game is not paused"));
		return;
	}

	// 정지화면 위젯 제거
	if (CurrentPauseMenuWidget)
	{
		CurrentPauseMenuWidget->RemoveFromParent();

		// 마우스 커서 숨기기 및 게임 모드 설정
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			PC->SetShowMouseCursor(false);
			PC->SetInputMode(FInputModeGameOnly());
		}

		UE_LOG(LogTemp, Warning, TEXT("Pause menu widget removed"));
	}

	// 게임 재개
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	bIsGamePaused = false;

	UE_LOG(LogTemp, Warning, TEXT("Game resumed"));
}