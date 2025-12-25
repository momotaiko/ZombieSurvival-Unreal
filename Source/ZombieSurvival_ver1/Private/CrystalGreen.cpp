// Fill out your copyright notice in the Description page of Project Settings.


#include "CrystalGreen.h"
#include "ZombieSurvivalCharacter.h"
#include "Kismet/GameplayStatics.h"

void ACrystalGreen::OnPickedUp(AZombieSurvivalCharacter* Player)
{
    Super::OnPickedUp(Player);  // 부모의 OnPickedUp 호출

    if (!Player) return;

    Player->GreenCrystalCount += 1;
    this->Destroy();
}
