// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemNormalManager.h"
#include "ItemManager.h"

AItemNormalManager::AItemNormalManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AItemNormalManager::BeginPlay()
{
	Super::BeginPlay();

	//1. 랜덤 생성 시간 구하기
	float createTime = FMath::RandRange(minTime_Normal, maxTime_Normal);
	//2. Timer Manger에 알람 등록
	GetWorld()->GetTimerManager().SetTimer(spawnTimerHandle, this, &AItemManager::CreateItem, createTime);

	//스폰 위치 동적 할당
	FindSpawnPoints("BP_ItemNormalSpawnPoint");
}
