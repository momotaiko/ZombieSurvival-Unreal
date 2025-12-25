// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemManager.h"
#include "ItemRandomLocationManager.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIESURVIVAL_VER1_API AItemRandomLocationManager : public AItemManager
{
	GENERATED_BODY()

public:
	AItemRandomLocationManager();

protected:
	virtual void BeginPlay() override;

public:
	
};
