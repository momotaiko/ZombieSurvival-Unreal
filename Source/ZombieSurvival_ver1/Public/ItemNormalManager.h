// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemManager.h"
#include "ItemNormalManager.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIESURVIVAL_VER1_API AItemNormalManager : public AItemManager
{
	GENERATED_BODY()
public:
	AItemNormalManager();

protected:
	virtual void BeginPlay() override;

public:
	
};
