// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemSpecial.h"

AItemSpecial::AItemSpecial()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AItemSpecial::BeginPlay()
{
	Super::BeginPlay();

}

void AItemSpecial::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AItemSpecial::OnPickedUp(AZombieSurvivalCharacter* Player)
{
	Super::OnPickedUp(Player);
}