// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CrystalBase.h"
#include "CrystalRed.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIESURVIVAL_VER1_API ACrystalRed : public ACrystalBase
{
	GENERATED_BODY()
	
public:
	virtual void OnPickedUp(class AZombieSurvivalCharacter* Player) override;
};
