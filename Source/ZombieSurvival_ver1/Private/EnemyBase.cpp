// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h" // ���ϸ� AEnemyBase.h -> EnemyBase.h�� �ٲ�
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "EnemyHealthBarWidget.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ZombieSurvivalCharacter.h"
#include "ZombieSurvivalGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"

// 적 클래스 생성자
AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 감지 컴포넌트 SphereComponent 생성
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(RootComponent);
	DetectionSphere->SetSphereRadius(DetectionRadius);
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DetectionSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Overlap);

	// 자동 AI 설정
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AAIController::StaticClass();

	// ü¹ٴ  ̻ (ּó)
	/*
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen); // 2D 
	HealthBarWidget->SetDrawSize(FVector2D(150.f, 20.f));
	HealthBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 120.f)); // Ӹ  ġ
	*/
}

// 게임 시작 시 호출
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("EnemyBase BeginPlay 시작"));

	CurrentHealth = MaxHealth;
	SpawnLocation = GetActorLocation(); // 스폰 위치 저장ㄴ
	
	UE_LOG(LogTemp, Warning, TEXT("스폰 위치: (%f, %f, %f)"), SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);

	// DetectionSphere 반지름을 블루프린트 설정값으로 업데이트
	if (DetectionSphere)
	{
		DetectionSphere->SetSphereRadius(DetectionRadius);
		UE_LOG(LogTemp, Warning, TEXT("Detection Radius 설정: %.1f"), DetectionRadius);
	}

	// 적들끼리 충돌하지 않도록 캡슐 컴포넌트 설정
	if (GetCapsuleComponent())
	{
		// 새로운 Enemy 충돌 프로파일 사용
		GetCapsuleComponent()->SetCollisionProfileName("Enemy");
		UE_LOG(LogTemp, Warning, TEXT("Enemy 충돌 프로파일 적용 완료"));
	}

	// 메쉬를 시야(ECC_Visibility) 채널 블로킹하게 설정
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	// 감지 이벤트 바인딩
	DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBase::OnPlayerDetected);
	UE_LOG(LogTemp, Warning, TEXT("감지 이벤트 바인딩 완료"));

	// 프로젝타일 충돌 감지 바인딩 (캡슐 컴포넌트 사용)
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBase::OnProjectileHit);
		UE_LOG(LogTemp, Warning, TEXT("프로젝타일 충돌 감지 바인딩 완료"));
	}

	// AI 컨트롤러 강제 생성
	if (!GetController())
	{
		UE_LOG(LogTemp, Warning, TEXT("AI Controller 생성 시도..."));
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		AAIController* NewController = GetWorld()->SpawnActor<AAIController>(AAIController::StaticClass(), SpawnParams);
		if (NewController)
		{
			NewController->Possess(this);
			UE_LOG(LogTemp, Warning, TEXT("AI Controller 생성 및 Possess 완료"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AI Controller 생성 실패"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AI Controller 이미 존재: %s"), *GetController()->GetName());
	}

	// 전역적으로 플레이어 찾기 (거리 상관없이)
	FindAndChasePlayer();
	
	UE_LOG(LogTemp, Warning, TEXT("EnemyBase BeginPlay 완료"));
}

// 매 프레임마다 호출
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsAlive()) return;

	// 애니메이션 변수 업데이트
	UpdateAnimationVariables();

	// 10초마다 현재 상태 로그
	static float LastStateLogTime = 0.f;
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastStateLogTime > 10.f)
	{
		const FString StateNames[] = { TEXT("Idle"), TEXT("Patrolling"), TEXT("Chasing"), TEXT("Attacking"), TEXT("Dead") };
		UE_LOG(LogTemp, Warning, TEXT("[%s] 현재 상태: %s, 타겟: %s"), 
			*GetName(), 
			*StateNames[(int32)CurrentAIState],
			TargetPlayer ? *TargetPlayer->GetName() : TEXT("없음"));
		LastStateLogTime = CurrentTime;
	}

	// 플레이어 타겟이 없으면 다시 찾기
	if (!TargetPlayer)
	{
		static float LastSearchTime = 0.f;
		if (CurrentTime - LastSearchTime > 5.f) // 5초마다 플레이어 재검색
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] 플레이어를 잃음 - 재검색 중..."), *GetName());
			FindAndChasePlayer();
			LastSearchTime = CurrentTime;
		}
		return;
	}

	// 상태별 처리 (순찰 제거, 추적만 처리)
	switch (CurrentAIState)
	{
		case EEnemyAIState::Idle:
			HandleIdleState();
			break;
		case EEnemyAIState::Patrolling:
			// 순찰 비활성화 - 바로 추적으로 전환
			SetAIState(EEnemyAIState::Chasing);
			break;
		case EEnemyAIState::Chasing:
			HandleChasingState();
			break;
		case EEnemyAIState::Attacking:
			HandleAttackingState();
			break;
		case EEnemyAIState::Dead:
			break;
	}
}

// 상태 설정 함수
void AEnemyBase::SetAIState(EEnemyAIState NewState)
{
	if (CurrentAIState == NewState) return;

	// 상태 변경 로그
	const FString StateNames[] = { TEXT("Idle"), TEXT("Patrolling"), TEXT("Chasing"), TEXT("Attacking"), TEXT("Dead") };
	UE_LOG(LogTemp, Warning, TEXT("AI 상태 변경: %s -> %s"), 
		*StateNames[(int32)CurrentAIState], *StateNames[(int32)NewState]);

	CurrentAIState = NewState;

	// 애니메이션 상태도 업데이트
	AnimationState = NewState;

	// 상태 변경 시 이동 속도 조정
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		switch (NewState)
		{
			case EEnemyAIState::Patrolling:
				MovementComp->MaxWalkSpeed = PatrolSpeed;
				UE_LOG(LogTemp, Warning, TEXT("이동 속도를 순찰 속도로 변경: %.1f"), PatrolSpeed);
				break;
			case EEnemyAIState::Chasing:
				MovementComp->MaxWalkSpeed = ChaseSpeed;
				UE_LOG(LogTemp, Warning, TEXT("이동 속도를 추적 속도로 변경: %.1f"), ChaseSpeed);
				break;
			default:
				break;
		}
	}
}

// 순찰 시작 (현재는 플레이어 찾기로 변경)
void AEnemyBase::StartPatrolling()
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] StartPatrolling 호출됨 - 플레이어 찾기로 변경"), *GetName());
	
	// 순찰 대신 플레이어 찾기
	FindAndChasePlayer();
}

// 새로운 순찰 지점 생성
void AEnemyBase::GenerateNewPatrolPoint()
{
	UE_LOG(LogTemp, Warning, TEXT("새로운 순찰 지점 생성 중..."));
	
	if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
	{
		FNavLocation RandomPoint;
		
		// 스폰 위치 주변에서 랜덤 포인트 찾기
		if (NavSys->GetRandomPointInNavigableRadius(SpawnLocation, PatrolRadius, RandomPoint))
		{
			CurrentPatrolTarget = RandomPoint.Location;
			UE_LOG(LogTemp, Warning, TEXT("NavMesh 기반 순찰 지점 생성 성공: (%f, %f, %f)"), 
				CurrentPatrolTarget.X, CurrentPatrolTarget.Y, CurrentPatrolTarget.Z);
		}
		else
		{
			// 네비게이션 실패 시 스폰 위치 주변 랜덤 위치
			FVector RandomDirection = FMath::VRand();
			RandomDirection.Z = 0; // 수평 이동만
			RandomDirection.Normalize();
			
			CurrentPatrolTarget = SpawnLocation + (RandomDirection * FMath::RandRange(100.f, PatrolRadius));
			UE_LOG(LogTemp, Warning, TEXT("NavMesh 실패 - 랜덤 순찰 지점 생성: (%f, %f, %f)"), 
				CurrentPatrolTarget.X, CurrentPatrolTarget.Y, CurrentPatrolTarget.Z);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("NavigationSystem을 찾을 수 없음!"));
		
		// NavMesh가 없을 때 대체 방법
		FVector RandomDirection = FMath::VRand();
		RandomDirection.Z = 0;
		RandomDirection.Normalize();
		
		CurrentPatrolTarget = SpawnLocation + (RandomDirection * FMath::RandRange(100.f, PatrolRadius));
		UE_LOG(LogTemp, Warning, TEXT("NavMesh 없음 - 대체 순찰 지점 생성: (%f, %f, %f)"), 
			CurrentPatrolTarget.X, CurrentPatrolTarget.Y, CurrentPatrolTarget.Z);
	}
}

// 순찰 지점으로 이동
void AEnemyBase::MoveToPatrolPoint()
{
	UE_LOG(LogTemp, Warning, TEXT("순찰 지점으로 이동 시작"));
	
	AAIController* AI = Cast<AAIController>(GetController());
	if (AI)
	{
		auto Result = AI->MoveToLocation(CurrentPatrolTarget);
		UE_LOG(LogTemp, Warning, TEXT("MoveToLocation 결과: %d"), (int32)Result);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AI Controller가 없음!"));
	}
}

// 플레이어 추적
void AEnemyBase::MoveToPlayer()
{
	AAIController* AI = Cast<AAIController>(GetController());
	if (AI && TargetPlayer)
	{
		auto Result = AI->MoveToActor(TargetPlayer);
		// 로그 제거 - 너무 자주 발생
	}
	else
	{
		if (!AI)
			UE_LOG(LogTemp, Error, TEXT("AI Controller가 없어 플레이어 추적 불가"));
		if (!TargetPlayer)
			UE_LOG(LogTemp, Error, TEXT("타겟 플레이어가 없어 추적 불가"));
	}
}

// 상태별 처리 함수들
void AEnemyBase::HandleIdleState()
{
	// 대기 상태에서도 플레이어를 찾음
	UE_LOG(LogTemp, Warning, TEXT("[%s] Idle 상태 - 플레이어 찾기"), *GetName());
	FindAndChasePlayer();
}

void AEnemyBase::HandlePatrollingState()
{
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		// 목표 지점에 도달했는지 체크
		float DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentPatrolTarget);
		
		// 디버깅: 5초마다 로그 (이전에는 2초)
		static float LastLogTime = 0.f;
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - LastLogTime > 5.f)
		{
			UE_LOG(LogTemp, Warning, TEXT("순찰 중 - 목표까지 거리: %.1f"), DistanceToTarget);
			LastLogTime = CurrentTime;
		}
		
		if (DistanceToTarget < 100.f) // 100 유닛 이내면 도달한 것으로 간주
		{
			UE_LOG(LogTemp, Warning, TEXT("순찰 지점 도달! 다음 지점으로 이동 준비"));
			OnPatrolPointReached();
		}
	}
}

void AEnemyBase::HandleChasingState()
{
	if (TargetPlayer)
	{
		// 플레이어가 존재하는지 확인 (파괴되었거나 null이 된 경우)
		if (!IsValid(TargetPlayer))
		{
			UE_LOG(LogTemp, Warning, TEXT("타겟 플레이어가 유효하지 않음 - 재검색"));
			TargetPlayer = nullptr;
			return;
		}

		// 거리 상관없이 계속 추적 (DetectionRadius 제한 제거)
		MoveToPlayer();
	}
	else
	{
		// 타겟이 없으면 재검색 (순찰하지 않음)
		UE_LOG(LogTemp, Warning, TEXT("[%s] 타겟 없음 - 플레이어 재검색 예약"), *GetName());
		// Tick에서 자동으로 재검색됨
	}
}

void AEnemyBase::HandleAttackingState()
{
	// 공격 상태 처리 - 나중에 구현
}

// 순찰 지점 도달 시 호출
void AEnemyBase::OnPatrolPointReached()
{
	UE_LOG(LogTemp, Warning, TEXT("OnPatrolPointReached 호출됨 - %.1f초 후 새 지점으로 이동"), PatrolWaitTime);
	
	// 기존 타이머가 있다면 클리어
	GetWorldTimerManager().ClearTimer(PatrolTimerHandle);
	
	// 잠시 대기 후 새로운 순찰 지점으로 이동
	GetWorldTimerManager().SetTimer(PatrolTimerHandle, [this]()
	{
		if (CurrentAIState == EEnemyAIState::Patrolling)
		{
			UE_LOG(LogTemp, Warning, TEXT("새로운 순찰 지점 생성 및 이동"));
			GenerateNewPatrolPoint();
			MoveToPatrolPoint();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("순찰 상태가 아니므로 타이머 취소"));
		}
	}, PatrolWaitTime, false);
}

// 플레이어 감지 시 호출되는 함수
void AEnemyBase::OnPlayerDetected(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	// 로그 빈도 제한 (5초마다만 로그)
	static float LastDetectionLogTime = 0.f;
	float CurrentTime = GetWorld()->GetTimeSeconds();
	bool bShouldLog = (CurrentTime - LastDetectionLogTime > 5.f);
	
	if (bShouldLog)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPlayerDetected 호출됨 - Actor: %s"), *OtherActor->GetName());
		LastDetectionLogTime = CurrentTime;
	}
	
	// 이미 추적 중인 플레이어가 있으면 무시
	if (TargetPlayer != nullptr) 
	{
		return; // 로그 제거
	}

	// 적이면 무시 (적들끼리 감지하지 않음)
	if (OtherActor->IsA<AEnemyBase>())
	{
		return; // 로그 제거 - 너무 자주 발생
	}

	// 플레이어 Pawn인지 확인
	APawn* PlayerPawn = Cast<APawn>(OtherActor);
	if (PlayerPawn)
	{
		// PlayerController가 있는지 확인 (실제 플레이어)
		APlayerController* PC = Cast<APlayerController>(PlayerPawn->GetController());
		bool bHasPlayerTag = PlayerPawn->ActorHasTag("Player");
		
		if (PC || bHasPlayerTag)
		{
			UE_LOG(LogTemp, Warning, TEXT("플레이어 감지 성공! (PlayerController: %s, Player태그: %s)"), 
				PC ? TEXT("있음") : TEXT("없음"), 
				bHasPlayerTag ? TEXT("있음") : TEXT("없음"));
			TargetPlayer = PlayerPawn;
			SetAIState(EEnemyAIState::Chasing);
			MoveToPlayer();
		}
		// else 부분의 로그 제거 - 너무 자주 발생
	}
	// else 부분의 로그 제거 - 불필요
}

// 데미지 받는 함수
void AEnemyBase::TakeDamage(float DamageAmount)
{
	if (!IsAlive()) return;

	// 체력 감소 및 클램프
	CurrentHealth -= DamageAmount;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.f, MaxHealth);

	// 피격 사운드 재생
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}

	// 체력 UI 업데이트 (활성화되어 있다면)
	if (HealthBarWidget)
	{
		if (UEnemyHealthBarWidget* HealthBar = Cast<UEnemyHealthBarWidget>(HealthBarWidget->GetUserWidgetObject()))
		{
			HealthBar->SetHealthPercent(CurrentHealth / MaxHealth);
		}
	}

	// 죽음 시 처리
	if (!IsAlive())
	{
		SetAIState(EEnemyAIState::Dead);
		
		// 애니메이션 변수 즉시 업데이트
		bIsDead = true;
		bIsMoving = false;
		Speed = 0.f;
		
		DropCrystals();
		
		AZombieSurvivalCharacter* PlayerChar = Cast<AZombieSurvivalCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
		AZombieSurvivalGameMode* GameMode = Cast<AZombieSurvivalGameMode>(UGameplayStatics::GetGameMode(this));
		
		if (PlayerChar)
		{
			PlayerChar->Kills += 1;
		}
		
		if (GameMode)
		{
			GameMode->AddKillCount();
		}

		Destroy();
	}
}

void AEnemyBase::DropCrystals()
{
	// 기본 구현은 비어있음. 자식 클래스에서 오버라이드해서 구현.
}

void AEnemyBase::FindAndChasePlayer()
{
	UE_LOG(LogTemp, Warning, TEXT("전역적으로 플레이어 검색 중..."));
	
	// 월드 전체에서 플레이어 찾기
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("World가 null입니다"));
		return;
	}

	// 방법 1: GetPlayerCharacter 사용 (가장 간단)
	APawn* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);
	if (PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetPlayerCharacter로 플레이어 찾음: %s"), *PlayerCharacter->GetName());
		TargetPlayer = PlayerCharacter;
		SetAIState(EEnemyAIState::Chasing);
		MoveToPlayer();
		return;
	}

	// 방법 2: 모든 Pawn 검색 (백업)
	UE_LOG(LogTemp, Warning, TEXT("GetPlayerCharacter 실패, 모든 Pawn 검색..."));
	
	for (TActorIterator<APawn> ActorItr(World); ActorItr; ++ActorItr)
	{
		APawn* Pawn = *ActorItr;
		if (!Pawn || Pawn->IsA<AEnemyBase>())
			continue;

		// PlayerController가 있는지 확인
		APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
		bool bHasPlayerTag = Pawn->ActorHasTag("Player");
		
		if (PC || bHasPlayerTag)
		{
			UE_LOG(LogTemp, Warning, TEXT("전역 검색으로 플레이어 찾음: %s (거리: %.1f)"), 
				*Pawn->GetName(), 
				FVector::Dist(GetActorLocation(), Pawn->GetActorLocation()));
			
			TargetPlayer = Pawn;
			SetAIState(EEnemyAIState::Chasing);
			MoveToPlayer();
			return;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("플레이어를 찾을 수 없습니다!"));
}

// 애니메이션 변수 업데이트
void AEnemyBase::UpdateAnimationVariables()
{
	if (!IsAlive())
	{
		bIsDead = true;
		bIsMoving = false;
		Speed = 0.f;
		AnimationState = EEnemyAIState::Dead;
		return;
	}

	// 현재 속도 계산
	FVector Velocity = GetVelocity();
	Speed = Velocity.Size();
	
	// 이동 중인지 판단 (속도가 10 이상이면 이동 중)
	bIsMoving = Speed > 10.f;
	
	// 애니메이션 상태 업데이트
	AnimationState = CurrentAIState;
	
	// 죽음 상태 업데이트
	bIsDead = !IsAlive();
	
	// 디버깅용 로그 (15초마다)
	static float LastAnimLogTime = 0.f;
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAnimLogTime > 15.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 애니메이션 - Speed: %.1f, bIsMoving: %s"), 
			*GetName(), Speed, bIsMoving ? TEXT("True") : TEXT("False"));
		LastAnimLogTime = CurrentTime;
	}
}

// 프로젝타일 충돌 처리 함수
void AEnemyBase::OnProjectileHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("!!!OnProjectileHit 호출됨 - OtherActor: %s!!!"), *OtherActor->GetName());

	// 이미 죽었으면 무시
	if (!IsAlive())
	{
		return;
	}

	// 액터가 유효한지 확인
	if (!OtherActor || !IsValid(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("OtherActor가 유효하지 않음"));
		return;
	}

	// 적끼리 충돌한 경우 무시 (가장 먼저 체크)
	if (OtherActor->IsA<AEnemyBase>())
	{
		UE_LOG(LogTemp, Warning, TEXT("다른 적과 충돌 - 무시: %s"), *OtherActor->GetName());
		return;
	}

	// 디버깅: 클래스 이름과 태그 정보 출력
	FString ClassName = OtherActor->GetClass()->GetName();
	UE_LOG(LogTemp, Warning, TEXT("=== 프로젝타일 검사 시작 ==="));
	UE_LOG(LogTemp, Warning, TEXT("OtherActor 이름: %s"), *OtherActor->GetName());
	UE_LOG(LogTemp, Warning, TEXT("OtherActor 클래스: %s"), *ClassName);
	
	// 모든 태그 출력
	TArray<FName> ActorTags = OtherActor->Tags;
	if (ActorTags.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("OtherActor 태그 개수: %d"), ActorTags.Num());
		for (int32 i = 0; i < ActorTags.Num(); i++)
		{
			UE_LOG(LogTemp, Warning, TEXT("태그 [%d]: %s"), i, *ActorTags[i].ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OtherActor에 태그가 없음"));
	}

	// 프로젝타일인지 확인 (여러 방법으로 체크)
	bool bIsProjectile = false;
	float ProjectileDamage = 10.0f; // 기본 데미지

	// 방법 1: TurretProjectile 태그로 확인
	bool bHasTurretProjectileTag = OtherActor->ActorHasTag("TurretProjectile");
	UE_LOG(LogTemp, Warning, TEXT("TurretProjectile 태그 검사 결과: %s"), bHasTurretProjectileTag ? TEXT("TRUE") : TEXT("FALSE"));
	
	if (bHasTurretProjectileTag)
	{
		bIsProjectile = true;
		UE_LOG(LogTemp, Warning, TEXT("TurretProjectile 태그로 프로젝타일 감지: %s"), *OtherActor->GetName());
	}
	
	// 방법 2: 클래스 이름으로 확인 (백업용 - BP_BasicProjectile)
	bool bClassNameCheck1 = ClassName.Contains("BasicProjectile");
	bool bClassNameCheck2 = ClassName.Contains("Projectile");
	UE_LOG(LogTemp, Warning, TEXT("클래스명 검사 - BasicProjectile 포함: %s"), bClassNameCheck1 ? TEXT("TRUE") : TEXT("FALSE"));
	UE_LOG(LogTemp, Warning, TEXT("클래스명 검사 - Projectile 포함: %s"), bClassNameCheck2 ? TEXT("TRUE") : TEXT("FALSE"));
	
	if (!bIsProjectile && (bClassNameCheck1 || bClassNameCheck2))
	{
		bIsProjectile = true;
		UE_LOG(LogTemp, Warning, TEXT("클래스명으로 프로젝타일 감지: %s"), *ClassName);
	}

	UE_LOG(LogTemp, Warning, TEXT("최종 프로젝타일 판정: %s"), bIsProjectile ? TEXT("TRUE") : TEXT("FALSE"));
	UE_LOG(LogTemp, Warning, TEXT("=== 프로젝타일 검사 끝 ==="));

	// 프로젝타일이 아니면 무시
	if (!bIsProjectile)
	{
		UE_LOG(LogTemp, Warning, TEXT("프로젝타일이 아니므로 무시"));
		return;
	}

	// 프로젝타일에서 ProjectileDamage 값 가져오기 시도
	if (UObject* ProjectileObject = Cast<UObject>(OtherActor))
	{
		// "ProjectileDamage" 속성 찾기
		if (UFloatProperty* ProjDamageProperty = FindFProperty<UFloatProperty>(ProjectileObject->GetClass(), "ProjectileDamage"))
		{
			ProjectileDamage = ProjDamageProperty->GetPropertyValue_InContainer(ProjectileObject);
			UE_LOG(LogTemp, Warning, TEXT("프로젝타일에서 ProjectileDamage 값 발견: %.1f"), ProjectileDamage);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("프로젝타일에서 ProjectileDamage 속성을 찾지 못함, 기본값 사용: %.1f"), ProjectileDamage);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[%s] 프로젝타일 충돌! 데미지: %.1f"), *GetName(), ProjectileDamage);

	// 데미지 적용
	TakeDamage(ProjectileDamage);

	// 프로젝타일 안전하게 제거 (다음 프레임에)
	if (IsValid(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("프로젝타일 제거 예약: %s"), *OtherActor->GetName());
		// 즉시 삭제 대신 다음 프레임에 삭제
		OtherActor->SetLifeSpan(0.01f); // 0.01초 후 삭제
	}
}