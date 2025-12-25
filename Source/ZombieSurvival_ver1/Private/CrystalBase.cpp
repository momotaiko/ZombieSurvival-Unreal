// Fill out your copyright notice in the Description page of Project Settings.


#include "CrystalBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ZombieSurvivalCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

// Sets default values
ACrystalBase::ACrystalBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 시각적 메쉬를 루트 컴포넌트로 설정
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	
	// 기본 큐브 메쉬 로드
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
		Mesh->SetRelativeScale3D(FVector(0.3f));
	}
	
	// 충돌 컴포넌트 Sphere - 오버랩 전용
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(30.0f);
	CollisionSphere->SetupAttachment(RootComponent);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

// Called when the game starts or when spawned
void ACrystalBase::BeginPlay()
{
	Super::BeginPlay();

	// 런타임에 물리 설정을 강제로 적용 (블루프린트 설정 오버라이드)
	if (Mesh)
	{
		// 메쉬 물리 설정 - 바닥과 충돌하지만 다른 수정들과는 충돌하지 않음
		Mesh->SetSimulatePhysics(true);
		Mesh->SetEnableGravity(true);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
		Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block); // 바닥과 충돌
		Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore); // 플레이어와는 충돌하지 않음
		
		// 물리 머티리얼 설정 (바운스 줄이기)
		Mesh->SetLinearDamping(0.5f);
		Mesh->SetAngularDamping(0.5f);
	}

	// 오버랩 이벤트 바인딩
	if (CollisionSphere)
	{
		CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACrystalBase::OnBeginOverlap);
		
		// 이미 오버랩된 액터들 체크 (생성 시점에 플레이어가 이미 근처에 있는 경우)
		TArray<AActor*> OverlappingActors;
		CollisionSphere->GetOverlappingActors(OverlappingActors);
		
		for (AActor* Actor : OverlappingActors)
		{
			AZombieSurvivalCharacter* Player = Cast<AZombieSurvivalCharacter>(Actor);
			if (Player)
			{
				// 바로 획득 처리
				OnPickedUp(Player);
				return; // 이미 획득되었으므로 타이머 설정할 필요 없음
			}
		}
	}

	// 60초 후 자동 삭제
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &ACrystalBase::AutoDestroy, 60.0f, false);
}

// Called every frame
void ACrystalBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 회전 코드 제거 - 가만히 있도록 함
}

void ACrystalBase::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	AZombieSurvivalCharacter* Player = Cast<AZombieSurvivalCharacter>(OtherActor);
	if (Player)
	{
		OnPickedUp(Player);
	}
}

void ACrystalBase::OnPickedUp(AZombieSurvivalCharacter* Player)
{
	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	}

	// 오브젝트 삭제 처리 - 이 부분은 실제 게임에서는 필요 없을 수도 있습니다.
}

void ACrystalBase::AutoDestroy()
{
	// 60초 후 자동으로 삭제
	Destroy();
}