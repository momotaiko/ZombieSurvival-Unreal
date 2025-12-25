// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyFSM2.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "TimerManager.h"

// Sets default values
AEnemyFSM2::AEnemyFSM2()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(RootComponent);
	DetectionSphere->SetSphereRadius(DetectionRadius);
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AAIController::StaticClass();

}

// Called when the game starts or when spawned
void AEnemyFSM2::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemyFSM2::OnPlayerDetected);
}

void AEnemyFSM2::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsAlive()) return;

	switch (CurrentState)
	{
		case EEnemyState::Idle:
			break;

		case EEnemyState::Chasing:
			if (TargetPlayer)
			{
				float Distance = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());
				if (Distance <= AttackRange)
				{
					CurrentState = EEnemyState::Attacking;
					AttackPlayer();
				}
				else
				{
					MoveToPlayer();
				}
			}
			break;

		case EEnemyState::Attacking:
			// 공격 후 다음 상태 전환은 AttackPlayer 내부 처리
			break;

		case EEnemyState::Dead:
			break;
	}
}
void AEnemyFSM2::TakeDamage(float DamageAmount)
{
	if (!IsAlive()) return;

	CurrentHealth -= DamageAmount;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.f, MaxHealth);

	if (!IsAlive())
	{
		CurrentState = EEnemyState::Dead;
		OnDeath();
	}
}

void AEnemyFSM2::OnPlayerDetected(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (TargetPlayer != nullptr || !IsAlive()) return;

	if (APawn* PlayerPawn = Cast<APawn>(OtherActor))
	{
		TargetPlayer = PlayerPawn;
		CurrentState = EEnemyState::Chasing;
		MoveToPlayer();
	}
}

void AEnemyFSM2::MoveToPlayer()
{
	AAIController* AI = Cast<AAIController>(GetController());
	if (AI && TargetPlayer)
	{
		auto Result = AI->MoveToActor(TargetPlayer);
		UE_LOG(LogTemp, Warning, TEXT(" MoveToActor: %d"), (int32)Result);
	}
}

void AEnemyFSM2::AttackPlayer()
{
	if (!TargetPlayer || !IsAlive()) return;

	UE_LOG(LogTemp, Warning, TEXT("공격 시작")); // 이모지 제거(인코딩 오류 방지)

	// 예: 일정 시간 후 다시 추적
	GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			if (IsAlive())
				CurrentState = EEnemyState::Chasing;
		});
}
