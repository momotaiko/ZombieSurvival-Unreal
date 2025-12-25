// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyFSM2.generated.h"

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle,
	Chasing,
	Attacking,
	Dead
};

UCLASS()
class ZOMBIESURVIVAL_VER1_API AEnemyFSM2 : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyFSM2();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EEnemyState CurrentState = EEnemyState::Idle;

	// 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHealth;

	// 감지 범위
	UPROPERTY(VisibleAnywhere, Category = "AI")
	class USphereComponent* DetectionSphere;

	UPROPERTY(EditAnywhere, Category = "AI")
	float DetectionRadius = 800.f;

	// 타겟
	UPROPERTY()
	APawn* TargetPlayer = nullptr;

	// 공격 사거리
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRange = 200.f;

	// 상태별 동작
	void MoveToPlayer();
	void AttackPlayer();

public:	
	// 데미지
	UFUNCTION(BlueprintCallable)
	void TakeDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable)
	bool IsAlive() const { return CurrentHealth > 0.f; }

	// 블루프린트용 사망 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnDeath();

	// 감지 이벤트
	UFUNCTION()
	void OnPlayerDetected(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
};
