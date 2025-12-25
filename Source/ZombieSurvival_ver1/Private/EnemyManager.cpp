// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyManager.h"
#include "EnemyBase.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
AEnemyManager::AEnemyManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemyManager::BeginPlay()
{
	Super::BeginPlay();

	//1. 랜덤 생성 시간 구하기
	float createTime = FMath::RandRange(minTime, maxTime);
	//2. Timer Manger에 알람 등록
	GetWorld()->GetTimerManager().SetTimer(spawnTimerHandle, this, &AEnemyManager::CreateEnemy, createTime);

	//스폰 위치 동적 할당
	FindSpawnPoints();

	//시간에 따른 EnemyManager 증가
	spawnCount = 1;
	maxSpawnCount = spawnPoints.Num();
	GetWorld()->GetTimerManager().SetTimer(spawnerTimerHandle, this, &AEnemyManager::AddSpawnerCount, addSpawnerTime, true);
	
}

void AEnemyManager::CreateEnemy()
{
	//스폰포인트 배열을 랜덤하게 섞기
	ShuffleArray(spawnPoints);

	//적 스폰
	for (int32 i = 0; i < spawnCount; ++i)
	{
		//아이템 스폰할 위치 가져오기
		FVector location = spawnPoints[i]->GetActorLocation();
		//모든 적 1명씩 스폰
		for (const auto& enemys : enemyFactory)
			GetWorld()->SpawnActor<AEnemyBase>(enemys, location, FRotator(0));
	}
	//다시 랜덤 시간에 CreateEnemy 함수가 호출되도록 타이머 설정
	float createTime = FMath::RandRange(minTime, maxTime);
	GetWorld()->GetTimerManager().SetTimer(spawnTimerHandle, this, &AEnemyManager::CreateEnemy, createTime);
}

void AEnemyManager::ShuffleArray(TArray<AActor*>& Array)
{
	const int32 LastIndex = Array.Num() - 1;
	for (int32 i = 0; i <= LastIndex; ++i)
	{
		int32 Index = FMath::RandRange(i, LastIndex);
		if (i != Index)
		{
			Array.Swap(i, Index);
		}
	}
}

void AEnemyManager::AddSpawnerCount()
{
	if (spawnCount < maxSpawnCount)
		++spawnCount;

}

void AEnemyManager::FindSpawnPoints()
{
	//검색으로 찾은 결과를 저장할 배열
	TArray<AActor*> allActors;
	//원하는 타입의 액터 모두 찾아오기
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), allActors);
	//찾은 결과가 있을 경우 반복적으로
	for (auto spawn : allActors)
	{
		//찾은 액터의 이름에 해당 문자열을 포함하고 있다면
		if (spawn->GetName().Contains(TEXT("BP_EnemySpawnPoint")))
		{
			//스폰 목록에 추가
			spawnPoints.Add(spawn);
		}
	}
}
