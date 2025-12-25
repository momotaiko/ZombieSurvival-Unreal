// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ZombieSurvivalCharacter.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AItemBase::AItemBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 충돌 감지용 Sphere
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(50.f);
	RootComponent = CollisionSphere;
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	// 시각적 메시
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 기본 큐브 메시 로드
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
		Mesh->SetRelativeScale3D(FVector(0.3f));
	}

}

// Called when the game starts or when spawned
void AItemBase::BeginPlay()
{
	Super::BeginPlay();

	// 공중에 살짝 띄우기
	FVector Location = GetActorLocation();
	Location.Z += 20.f;
	SetActorLocation(Location);

	// 오버랩 이벤트 등록
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AItemBase::OnBeginOverlap);
}

// Called every frame
void AItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 제자리 회전
	FRotator CurrentRotation = GetActorRotation();
	CurrentRotation.Yaw += RotationSpeed * DeltaTime;
	SetActorRotation(CurrentRotation);
}

void AItemBase::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	AZombieSurvivalCharacter* Player = Cast<AZombieSurvivalCharacter>(OtherActor);
	if (Player)
	{
		OnPickedUp(Player);
	}
}

void AItemBase::OnPickedUp(AZombieSurvivalCharacter* Player)
{
	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	}

	// 나머지 실제 효과 적용 및 파괴는(Destroy()) 파생된 아이템 클래스에서 개별 처리
}