// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHealthBarWidget.h"
#include "Components/ProgressBar.h"

void UEnemyHealthBarWidget::SetHealthPercent(float Percent)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(Percent); // 퍼센트 값 (0.0 ~ 1.0)
	}
}
