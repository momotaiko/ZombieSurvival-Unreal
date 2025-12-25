// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemGetUpgrade.h"
#include "ZombieSurvivalCharacter.h"
#include "ZombieSurvivalGameMode.h"
#include "Kismet/GameplayStatics.h"

void AItemGetUpgrade::OnPickedUp(AZombieSurvivalCharacter* Player)
{
	Super::OnPickedUp(Player);

	if (!Player) return;

	// 게임모드 가져오기
	AZombieSurvivalGameMode* GameMode = Cast<AZombieSurvivalGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("Upgrade item picked up! Opening upgrade menu..."));
		
		// 업그레이드 창 열기 (게임 정지 및 업그레이드 UI 표시)
		GameMode->PauseGameForUpgrade();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get ZombieSurvivalGameMode for upgrade item"));
	}

	// 아이템 제거
	Destroy();
}

