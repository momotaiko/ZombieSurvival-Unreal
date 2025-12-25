// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemNormal.h"
#include "Item_HealKit.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIESURVIVAL_VER1_API AItem_HealKit : public AItemNormal
{
	GENERATED_BODY()

public:
	virtual void OnPickedUp(class AZombieSurvivalCharacter* Player) override;

};
