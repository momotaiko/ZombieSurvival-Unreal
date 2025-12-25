// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.generated.h"

UCLASS()
class ZOMBIESURVIVAL_VER1_API AItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// 아이템 획득 처리 (플레이어가 먹었을 때)
	virtual void OnPickedUp(class AZombieSurvivalCharacter* Player);

protected:
	// 충돌 판정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	class USphereComponent* CollisionSphere;

	// 아이템 외형
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	class UStaticMeshComponent* Mesh;

	// 회전 속도 (초당 몇 도 회전할지)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Visual")
	float RotationSpeed = 60.f;

	// 아이템 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemName;

	// 아이템 설명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText ItemDescription;

	// 습득 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	class USoundBase* PickupSound;

	// 오버랩 이벤트 핸들러
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
};
