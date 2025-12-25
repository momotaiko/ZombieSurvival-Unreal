// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "ItemNormal.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIESURVIVAL_VER1_API AItemNormal : public AItemBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AItemNormal();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OnPickedUp(AZombieSurvivalCharacter* Player) override;
};
