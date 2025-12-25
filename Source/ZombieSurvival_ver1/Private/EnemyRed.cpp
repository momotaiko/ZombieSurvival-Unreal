// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyRed.h"

AEnemyRed::AEnemyRed()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemyRed::BeginPlay()
{
	Super::BeginPlay();

}

void AEnemyRed::DropCrystals()
{
    UWorld* World = GetWorld();
    if (!World || !CrystalToDrop) return;

    int32 DropCount = FMath::RandRange(1, 3);
    for (int32 i = 0; i < DropCount; ++i)
    {
        FVector SpawnLoc = GetActorLocation() + FMath::VRand() * 100.f;
        FRotator SpawnRot = FRotator::ZeroRotator;

        World->SpawnActor<AActor>(CrystalToDrop, SpawnLoc, SpawnRot);
    }
}