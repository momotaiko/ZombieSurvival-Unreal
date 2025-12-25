// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "ItemGetUpgrade.generated.h"

/**
 * 업그레이드 창을 열어주는 아이템
 */
UCLASS()
class ZOMBIESURVIVAL_VER1_API AItemGetUpgrade : public AItemBase
{
	GENERATED_BODY()
	
public:
	// 플레이어가 아이템을 획득했을 때 호출되는 함수
	virtual void OnPickedUp(class AZombieSurvivalCharacter* Player) override;
};
