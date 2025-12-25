// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemNormal.h"

AItemNormal::AItemNormal()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AItemNormal::BeginPlay()
{
	Super::BeginPlay();

}

void AItemNormal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AItemNormal::OnPickedUp(AZombieSurvivalCharacter* Player)
{
	Super::OnPickedUp(Player);
}