// Fill out your copyright notice in the Description page of Project Settings.


#include "CrystalBlue.h"
#include "ZombieSurvivalCharacter.h"
#include "Kismet/GameplayStatics.h"

void ACrystalBlue::OnPickedUp(AZombieSurvivalCharacter* Player)
{
    Super::OnPickedUp(Player);  // 부모의 OnPickedUp 호출

    if (!Player) return;

    Player->BlueCrystalCount += 1;
    this->Destroy();
}