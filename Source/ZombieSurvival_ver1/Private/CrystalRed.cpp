// Fill out your copyright notice in the Description page of Project Settings.


#include "CrystalRed.h"
#include "ZombieSurvivalCharacter.h"
#include "Kismet/GameplayStatics.h"

void ACrystalRed::OnPickedUp(AZombieSurvivalCharacter* Player)
{
    Super::OnPickedUp(Player);  // 부모의 OnPickedUp 호출

    if (!Player) return;

    Player->RedCrystalCount += 1;
    this->Destroy();
}
