// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "EnemyBlue.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIESURVIVAL_VER1_API AEnemyBlue : public AEnemyBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEnemyBlue();

	virtual void DropCrystals() override;

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	TSubclassOf<AActor> CrystalToDrop;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
