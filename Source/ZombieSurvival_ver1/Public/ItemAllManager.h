// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemManager.h"
#include "ItemAllManager.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIESURVIVAL_VER1_API AItemAllManager : public AItemManager
{
	GENERATED_BODY()

public:
	AItemAllManager();

protected:
	virtual void BeginPlay() override;

public:
	
};
