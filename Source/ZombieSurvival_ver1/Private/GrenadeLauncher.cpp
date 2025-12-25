// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeLauncher.h"

// Sets default values
AGrenadeLauncher::AGrenadeLauncher()
{
	PrimaryActorTick.bCanEverTick = false;

	LauncherMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LauncherMesh"));
	SetRootComponent(LauncherMesh);

	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	MuzzleLocation->SetupAttachment(LauncherMesh);
	MuzzleLocation->SetRelativeLocation(FVector(0.f, 50.f, 10.f)); // 총구 위치 조정
}

// Called when the game starts or when spawned
void AGrenadeLauncher::BeginPlay()
{
	Super::BeginPlay();
	
}

