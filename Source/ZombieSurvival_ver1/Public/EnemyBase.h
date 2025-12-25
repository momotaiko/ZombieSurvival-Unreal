// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

// 적 AI 상태 열거형
UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
	Idle,		// 대기
	Patrolling,	// 순찰
	Chasing,	// 추적
	Attacking,	// 공격
	Dead		// 사망
};

UCLASS()
class ZOMBIESURVIVAL_VER1_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyBase();

protected:
	virtual void BeginPlay() override;

	// === 체력 관련 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Health")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Health")
	float CurrentHealth;

	// === AI 상태 관련 ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
	EEnemyAIState CurrentAIState = EEnemyAIState::Patrolling;

	// === 감지 관련 ===
	UPROPERTY(VisibleAnywhere, Category = "Enemy|Detection")
	class USphereComponent* DetectionSphere;

	UPROPERTY(EditAnywhere, Category = "Enemy|Detection")
	float DetectionRadius = 800.f;

	// === 순찰 관련 ===
	UPROPERTY(EditAnywhere, Category = "Enemy|Patrol")
	float PatrolRadius = 500.f;  // 순찰 반경

	UPROPERTY(EditAnywhere, Category = "Enemy|Patrol")
	float PatrolWaitTime = 2.f;  // 순찰 지점에서 대기 시간

	UPROPERTY(EditAnywhere, Category = "Enemy|Patrol")
	float PatrolSpeed = 150.f;   // 순찰 속도

	UPROPERTY(EditAnywhere, Category = "Enemy|Patrol")
	float ChaseSpeed = 300.f;    // 추적 속도

	// === 내부 변수 ===
	FVector SpawnLocation;       // 스폰 위치 (순찰 중심점)
	FVector CurrentPatrolTarget; // 현재 순찰 목표 지점
	APawn* TargetPlayer = nullptr;

	// 타이머 핸들
	FTimerHandle PatrolTimerHandle;
	FTimerHandle StateTimerHandle;

	// === UI 관련 ===
	UPROPERTY(VisibleAnywhere, Category = "Enemy|UI")
	UWidgetComponent* HealthBarWidget;

	// === AI 컨트롤러 ===
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI")
	TSubclassOf<class AAIController> EnemyAIControllerClass;

	// === 사운드 ===
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Sound")
	USoundBase* HitSound;

	// === 애니메이션 관련 ===
	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Animation")
	float Speed = 0.f; // 현재 이동 속도

	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Animation")
	bool bIsDead = false; // 죽음 여부

	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Animation")
	bool bIsMoving = false; // 이동 중인지 여부

	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Animation")
	EEnemyAIState AnimationState = EEnemyAIState::Patrolling; // 애니메이션용 상태

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// === 체력 관련 함수 ===
	UFUNCTION(BlueprintCallable)
	void TakeDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable)
	bool IsAlive() const { return CurrentHealth > 0.f; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy")
	void OnDeath();

	// === AI 상태 관련 함수 ===
	UFUNCTION(BlueprintCallable)
	void SetAIState(EEnemyAIState NewState);

	UFUNCTION(BlueprintCallable)
	EEnemyAIState GetAIState() const { return CurrentAIState; }

	// === 이벤트 핸들러 ===
	UFUNCTION()
	void OnPlayerDetected(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	// === 프로젝타일 충돌 처리 ===
	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	// === AI 행동 함수 ===
	void StartPatrolling();
	void MoveToPlayer();
	void MoveToPatrolPoint();
	void GenerateNewPatrolPoint();
	void FindAndChasePlayer();

	// === 애니메이션 함수 ===
	UFUNCTION(BlueprintCallable, Category = "Enemy|Animation")
	void UpdateAnimationVariables();

	// === 드롭 시스템 ===
	virtual void DropCrystals();

protected:
	// === AI 상태 처리 함수 ===
	void HandleIdleState();
	void HandlePatrollingState();
	void HandleChasingState();
	void HandleAttackingState();

	// === 타이머 콜백 ===
	UFUNCTION()
	void OnPatrolPointReached();
};
