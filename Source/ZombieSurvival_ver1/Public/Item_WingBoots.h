// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemSpecial.h"
#include "Item_WingBoots.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIESURVIVAL_VER1_API AItem_WingBoots : public AItemSpecial
{
	GENERATED_BODY()
	
public:
	virtual void OnPickedUp(class AZombieSurvivalCharacter* Player) override;
};
