// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyManager.generated.h"

UCLASS()
class ZOMBIESURVIVAL_VER1_API AEnemyManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemyManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	//적 랜덤 최소 스폰시간
	UPROPERTY(EditAnywhere, Category = SpawnSettings)
	float minTime = 1;
	//적 랜덤 최대 스폰시간
	UPROPERTY(EditAnywhere, Category = SpawnSettings)
	float maxTime = 5;
	//적 스폰 위치
	UPROPERTY(EditAnywhere, Category = SpawnSettings)
	TArray<class AActor*> spawnPoints;
	// AEnemyBase 타입의 블루프린트를 할당받을 변수
	UPROPERTY(EditAnywhere, Category = SpawnSettings)
	TArray<TSubclassOf<class AEnemyBase>> enemyFactory;

	//스폰 타이머
	FTimerHandle spawnTimerHandle;
	FTimerHandle spawnerTimerHandle;

	//적 생성 함수
	void CreateEnemy();
	void ShuffleArray(TArray<AActor*>& Array);

	//스폰할 위치를 동적으로 찾기
	void FindSpawnPoints();

	// 시간에 따라 생성되는 EnemyManager 증가
	int32 spawnCount;
	int32 maxSpawnCount;
	// 몇 초마다 증가할지
	float addSpawnerTime = 5.0f;
	void AddSpawnerCount();

};
