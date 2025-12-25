// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemNormal.h"
#include "ItemSpecial.h"
#include <Map>
#include "ItemManager.generated.h"

UCLASS()
class ZOMBIESURVIVAL_VER1_API AItemManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	TMap<FString, int32> itemNormalName = {
		{TEXT("HealKit"), 0},
	};
	TMap<FString, int32> itemSpecialName = {
		{TEXT("WingBoots"), 0},
	};

	//Normal 아이템 랜덤 최소 스폰시간
	UPROPERTY(EditAnywhere, Category = SpawnSettings)
	float minTime_Normal = 1;
	//Normal 아이템 랜덤 최대 스폰시간
	UPROPERTY(EditAnywhere, Category = SpawnSettings)
	float maxTime_Normal = 3;
	//Special 아이템 랜덤 최소 스폰시간
	UPROPERTY(EditAnywhere, Category = SpawnSettings)
	float minTime_Special = 3;
	//Special 아이템 랜덤 최대 스폰시간
	UPROPERTY(EditAnywhere, Category = SpawnSettings)
	float maxTime_Special = 5;
	//아이템 스폰 위치
	UPROPERTY(EditAnywhere, Category = SpawnSettings)
	TArray<class AActor*> spawnPoints;
	//AEnemy 타입의 블루프린트 할당받을 변수
	UPROPERTY(EditAnywhere, Category = SpawnSettings)
	TArray<TSubclassOf<AItemNormal>> itemNormalFactory;
	UPROPERTY(EditAnywhere, Category = SpawnSettings)
	TArray<TSubclassOf<AItemSpecial>> itemSpecialFactory;

	//아이템 랜덤 생성 관련
	UFUNCTION()
	void OnItemDestroyed(AActor* DestroyedActor);
	UPROPERTY()
	TMap<AActor*, AActor*> activeItems_SpawnPoint;
	void ShuffleArray(TArray<AActor*>& Array);

	//랜덤 위치 스폰 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FVector SpawnRange;

	//스폰 타이머
	FTimerHandle spawnTimerHandle;
	//아이템 생성 함수
	void CreateItem();

	//스폰할 위치 동적 찾아 할당하기
	void FindSpawnPoints(FString text);
};
