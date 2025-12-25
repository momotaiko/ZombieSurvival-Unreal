// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gun.generated.h"

UCLASS()
class ZOMBIESURVIVAL_VER1_API AGun : public AActor
{
	GENERATED_BODY()
	
public:	
	AGun();

	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	void Fire();

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* GunMesh;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* MuzzleLocation;

	UPROPERTY(VisibleDefaultsOnly)
	USceneComponent* Root;


	UPROPERTY(EditAnywhere)
	float Damage = 10.f;

	UPROPERTY(EditAnywhere)
	float MaxRange = 10000.f;

};
