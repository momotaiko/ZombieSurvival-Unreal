// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZombieSurvivalCharacter.h"
#include "GameFramework/Actor.h"
#include "WaterSlow.generated.h"

UCLASS()
class ZOMBIESURVIVAL_VER1_API AWaterSlow : public AActor
{
	GENERATED_BODY()

public:
	AWaterSlow();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere)
	class AZombieSurvivalCharacter* character;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* boxComp;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* meshComp;

	//speed °¨¼Ò
	UPROPERTY(EditAnywhere)
	float slowSpeed = 400.0f;
	UPROPERTY(EditAnywhere)
	float defaultSpeed = 600.0f;
	UFUNCTION()
	void OnWaterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OutWaterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
