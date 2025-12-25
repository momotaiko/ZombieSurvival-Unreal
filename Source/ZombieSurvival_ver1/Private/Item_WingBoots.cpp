// Fill out your copyright notice in the Description page of Project Settings.


#include "Item_WingBoots.h"
#include "ZombieSurvivalCharacter.h"
#include "Kismet/GameplayStatics.h"

void AItem_WingBoots::OnPickedUp(AZombieSurvivalCharacter* Player)
{
	Super::OnPickedUp(Player);

	if (!Player) return;

	// 2단 점프 가능하게 설정
	Player->haveWing = true;

	// 아이템 제거
	Destroy();
}