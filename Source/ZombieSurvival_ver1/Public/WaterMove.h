// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZombieSurvivalCharacter.h"
#include "GameFramework/Actor.h"
#include "WaterMove.generated.h"

UCLASS()
class ZOMBIESURVIVAL_VER1_API AWaterMove : public AActor
{
	GENERATED_BODY()

public:
	AWaterMove();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	class AZombieSurvivalCharacter* character;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* boxComp;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* meshComp;

	bool onWater = false;

	// 이동 거리 조절
	UPROPERTY(EditAnywhere)
	float moveDistance = 3.0f;

	UFUNCTION()
	void OnWaterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OutWaterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
