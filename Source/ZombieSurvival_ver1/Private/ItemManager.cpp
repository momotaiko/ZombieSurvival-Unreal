// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemManager.h"
#include "CoreMinimal.h"
#include "ItemNormalManager.h"
#include "ItemAllManager.h"
#include "ItemRandomLocationManager.h"
#include <Kismet/GameplayStatics.h>
#include <Kismet/KismetMathLibrary.h>
#include <Engine/World.h>

class ItemNormalManager;
class ItemAllManager;
class ItemRandomLocationManager;

// Sets default values
AItemManager::AItemManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AItemManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void AItemManager::CreateItem()
{
	//----------Spawn Random Loation Item 라인트레이스 설정----------
	// 중심 위치와 랜덤 박스 범위 내 위치 설정
	FVector originLocation = GetActorLocation();
	FVector randomLocation = UKismetMathLibrary::RandomPointInBoundingBox(originLocation, SpawnRange);

	// 라인 트레이스를 위에서 아래로 쏨
	FVector traceStart = randomLocation + FVector(0.f, 0.f, 300.f);
	FVector traceEnd = randomLocation - FVector(0.f, 0.f, 300.f);

	FHitResult hitResult;
	FCollisionQueryParams traceParams;
	traceParams.bTraceComplex = true;
	traceParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		hitResult,
		traceStart,
		traceEnd,
		ECC_Visibility,
		traceParams
	);
	//--------------------------------------------------------

	//1. 사용 가능한 스폰포인트 찾기
	TArray<AActor*> unusedSpawnPoints;
	for (AActor* spwanpoint : spawnPoints)
		if (!activeItems_SpawnPoint.Contains(spwanpoint) || !IsValid(activeItems_SpawnPoint[spwanpoint]))
			unusedSpawnPoints.Add(spwanpoint);

	//2. 사용 가능한 스폰포인트가 없으면 종료
	if (unusedSpawnPoints.Num() == 0)
	{
		float createTime;
		if (this->IsA(AItemNormalManager::StaticClass()))
			createTime = FMath::RandRange(minTime_Normal, maxTime_Normal);
		else
			createTime = FMath::RandRange(minTime_Special, maxTime_Special);

		GetWorld()->GetTimerManager().SetTimer(spawnTimerHandle, this, &AItemManager::CreateItem, createTime);
	}

	//3. 몇개의 아이템을 생성할지 정하기
	int32 spawnCount = FMath::RandRange(1, unusedSpawnPoints.Num());

	//스폰포인트 배열을 랜덤하게 섞기
	ShuffleArray(unusedSpawnPoints);

	//4. 아이템 스폰
	for (int32 i = 0; i < spawnCount; ++i)
	{
		//남은 스폰 포인트가 없을 때
		if (unusedSpawnPoints.Num() == 0)
			return;

		//아이템 스폰할 위치 가져오기
		FVector location = unusedSpawnPoints[i]->GetActorLocation();
		//생성된 아이템을 저장할 위치
		AActor* spawnedItem = nullptr;

		//Normal 아이템의 경우
		if (this->IsA(AItemNormalManager::StaticClass()))
		{
			//일반 아이템 중에서 하나를 랜덤 선택하여 스폰
			int32 random = FMath::RandRange(0, itemNormalFactory.Num() - 1);
			spawnedItem = GetWorld()->SpawnActor<AItemNormal>(itemNormalFactory[random], location, FRotator(0));
		}
		//Random location 아이템의 경우
		else if (this->IsA(AItemRandomLocationManager::StaticClass()))
		{
			FVector spawnLocation = hitResult.ImpactPoint + FVector(500.0f, 500.0f, 100.0f);

			//일반 아이템 중에서 하나를 랜덤 선택하여 스폰
			int32 random = FMath::RandRange(0, itemNormalFactory.Num() - 1);
			spawnedItem = GetWorld()->SpawnActor<AItemNormal>(itemNormalFactory[random], spawnLocation, FRotator(0));
		}
		//Normal+Special 아이템의 경우
		else
		{
			int32 itemType = FMath::RandRange(0, 1);
			//일반 아이템
			if (itemType == 0 && itemNormalFactory.Num() > 0)
			{
				int32 random = FMath::RandRange(0, itemNormalFactory.Num() - 1);
				spawnedItem = GetWorld()->SpawnActor<AItemNormal>(itemNormalFactory[random], location, FRotator(0));
			}
			// 특수 아이템
			else if (itemType == 1 && itemSpecialFactory.Num() > 0)
			{
				int32 random = FMath::RandRange(0, itemSpecialFactory.Num() - 1);
				spawnedItem = GetWorld()->SpawnActor<AItemSpecial>(itemSpecialFactory[random], location, FRotator(0));
			}
		}
		if (IsValid(spawnedItem))
		{
			//아이템 추적 시작
			activeItems_SpawnPoint.Add(unusedSpawnPoints[i], spawnedItem);
			//아이템 추적 끝
			spawnedItem->OnDestroyed.AddDynamic(this, &AItemManager::OnItemDestroyed);
		}
	}

	//다시 랜덤 시간에 CreateItem 함수가 호출되도록 타이머 설정
	float createTime;
	if (this->IsA(AItemAllManager::StaticClass()))
		createTime = FMath::RandRange(minTime_Special, maxTime_Special);
	else
		createTime = FMath::RandRange(minTime_Normal, maxTime_Normal);

	GetWorld()->GetTimerManager().SetTimer(spawnTimerHandle, this, &AItemManager::CreateItem, createTime);
}
void AItemManager::ShuffleArray(TArray<AActor*>& Array)
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
void AItemManager::OnItemDestroyed(AActor* DestroyedActor)
{
	for (auto& pair : activeItems_SpawnPoint)
	{
		if (pair.Value == DestroyedActor)
		{
			activeItems_SpawnPoint.Remove(pair.Key);
			break;
		}
	}
}

void AItemManager::FindSpawnPoints(FString text)
{
	//검색으로 찾은 결과를 저장할 배열
	TArray<AActor*> allActors;
	//원하는 타입의 액터 모두 찾아오기
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), allActors);
	//찾은 결과가 있을 경우 반복적으로
	for (auto spawn : allActors)
	{
		//찾은 액터의 이름에 해당 문자열을 포함하고 있다면
		if (spawn->GetName().Contains(text))
		{
			//스폰 목록에 추가
			spawnPoints.Add(spawn);
		}
	}
}
