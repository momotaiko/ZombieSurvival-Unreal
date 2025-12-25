// Fill out your copyright notice in the Description page of Project Settings.


#include "Item_HealKit.h"
#include "ZombieSurvivalCharacter.h"
#include "Kismet/GameplayStatics.h"


void AItem_HealKit::OnPickedUp(AZombieSurvivalCharacter* Player)
{
	Super::OnPickedUp(Player);

	if (!Player) return;

	if (Player->CurrentHealth <= 75)
		Player->CurrentHealth += 25;
	else
		Player->CurrentHealth = 100;
	
	// 아이템 제거
	Destroy();
}