// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UpgradeDataTypes.h"
#include "UIData.h"
#include "ZombieSurvivalGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIDataChanged, UUIData*, Data);

UCLASS()
class ZOMBIESURVIVAL_VER1_API AZombieSurvivalGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	AZombieSurvivalGameMode();

	UFUNCTION(BlueprintCallable)
	void AddKillCount();

	UFUNCTION(BlueprintCallable)
	void PauseGameForUpgrade();

	UFUNCTION(BlueprintCallable)
	TArray<FUpgradeData> GenerateUpgradeOptions();

	UFUNCTION(BlueprintCallable)
	void ApplyUpgrade(const FUpgradeData& SelectedUpgrade);

	// 업그레이드 UI 관련 함수들
	UFUNCTION(BlueprintCallable, Category = "Upgrade UI")
	void ShowUpgradeUI();

	UFUNCTION(BlueprintCallable, Category = "Upgrade UI")
	void HideUpgradeUI();

	UFUNCTION(BlueprintCallable, Category = "Upgrade UI")
	TArray<FUpgradeData> GetCurrentUpgradeOptions() const { return CurrentUpgradeOptions; }

	// UI에서 업그레이드 선택 시 호출
	UFUNCTION(BlueprintCallable, Category = "Upgrade UI")
	void SelectUpgrade(int32 OptionIndex);

	// 정지화면 관련 함수들
	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void GameStop();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void GameResume();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	bool IsGamePaused() const { return bIsGamePaused; }

	/* 1) DTO 인스턴스 포인터 */
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	UUIData* CurrentUIData = nullptr;

	/* 2) 블루프린트에서 바인딩할 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnUIDataChanged OnUIDataChanged;

	/* 3) 값 채우고 브로드캐스트 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void PushUIData();

protected:
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade System")
	class UDataTable* UpgradeDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
	int32 KillCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
	int32 KillsRequiredForUpgrade;

	// 플레이어가 선택한 업그레이드 기록
	UPROPERTY(BlueprintReadOnly, Category = "Upgrade System")
	TArray<FName> AcquiredUpgrades;

	// 업그레이드 UI 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade UI")
	TSubclassOf<class UUserWidget> UpgradeWidgetClass;

	// 현재 표시 중인 업그레이드 UI 위젯
	UPROPERTY(BlueprintReadOnly, Category = "Upgrade UI")
	class UUserWidget* CurrentUpgradeWidget;

	// 현재 생성된 업그레이드 옵션들
	UPROPERTY(BlueprintReadOnly, Category = "Upgrade UI")
	TArray<FUpgradeData> CurrentUpgradeOptions;

	// 정지화면 UI 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pause Menu")
	TSubclassOf<class UUserWidget> PauseMenuWidgetClass;

	// 현재 표시 중인 정지화면 UI 위젯
	UPROPERTY(BlueprintReadOnly, Category = "Pause Menu")
	class UUserWidget* CurrentPauseMenuWidget;

	// 게임 정지 상태
	UPROPERTY(BlueprintReadOnly, Category = "Pause Menu")
	bool bIsGamePaused;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class AZombieSurvivalCharacter* Player;
};
