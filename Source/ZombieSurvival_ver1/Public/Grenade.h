// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grenade.generated.h"

UCLASS()
class ZOMBIESURVIVAL_VER1_API AGrenade : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrenade();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 폭발 처리
	void Explode();

	// 충돌 처리
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// 적과 겹칠 때 폭발 처리
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// ProjectileMovementComponent 충돌 처리
	UFUNCTION()
	void OnProjectileStop(const FHitResult& ImpactResult);

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category = "Grenade")
	float ExplosionRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Grenade")
	float ExplosionDamage = 50.f;

private:
	FTimerHandle ExplosionTimerHandle;

	// 중복 폭발 방지 플래그
	bool bHasExploded = false;

};
