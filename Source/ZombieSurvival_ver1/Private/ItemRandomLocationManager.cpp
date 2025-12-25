// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemRandomLocationManager.h"
#include "ItemManager.h"

AItemRandomLocationManager::AItemRandomLocationManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AItemRandomLocationManager::BeginPlay()
{
	Super::BeginPlay();

	//1. 랜덤 생성 시간 구하기
	float createTime = FMath::RandRange(minTime_Normal, maxTime_Normal);
	//2. Timer Manger에 알람 등록
	GetWorld()->GetTimerManager().SetTimer(spawnTimerHandle, this, &AItemRandomLocationManager::CreateItem, createTime);

	//스폰 위치 동적 할당
	FindSpawnPoints("BP_ItemRandomLocationSpawnPoint");
}
