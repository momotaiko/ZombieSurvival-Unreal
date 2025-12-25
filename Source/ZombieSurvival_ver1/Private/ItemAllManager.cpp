// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemAllManager.h"
#include "ItemManager.h"

AItemAllManager::AItemAllManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AItemAllManager::BeginPlay()
{
	Super::BeginPlay();

	//1. 랜덤 생성 시간 구하기
	float createTime = FMath::RandRange(minTime_Special, maxTime_Special);
	//2. Timer Manger에 알람 등록
	GetWorld()->GetTimerManager().SetTimer(spawnTimerHandle, this, &AItemManager::CreateItem, createTime);

	//스폰 위치 동적 할당
	FindSpawnPoints("BP_ItemAllSpawnPoin");
}