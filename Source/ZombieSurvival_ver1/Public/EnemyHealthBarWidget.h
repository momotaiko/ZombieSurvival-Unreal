// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHealthBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIESURVIVAL_VER1_API UEnemyHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 여기서 HealthBar 라는 이름은 위젯 안의 ProgressBar 이름과 같아야 함
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	// 체력 비율을 설정하는 함수
	void SetHealthPercent(float Percent);
	
};
