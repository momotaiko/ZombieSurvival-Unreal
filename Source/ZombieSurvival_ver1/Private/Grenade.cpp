// Fill out your copyright notice in the Description page of Project Settings.


#include "Grenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "ZombieSurvivalCharacter.h"
#include "EnemyBase.h" // ���ϸ� AEnemyBase -> EnemyBase�� �ٲ� 
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// Sets default values
AGrenade::AGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// ��ź �޽� ������Ʈ ���� �� ��Ʈ ����
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

    // �⺻ ��ü �޽� ����
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        Mesh->SetStaticMesh(SphereMesh.Object);
        Mesh->SetWorldScale3D(FVector(0.2f)); // �۰� ���
    }

    // 충돌 설정: 물리 활성, Pawn과는 겹치기만 허용
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Mesh->SetCollisionObjectType(ECC_WorldDynamic);
    Mesh->SetCollisionResponseToAllChannels(ECR_Block);  // 기본적으로 모든 것과 충돌
    Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);      // 캐릭터와는 겹치기 (적 감지용)
    Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block); // 벽/바닥과는 충돌
    Mesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block); // 다른 오브젝트와 충돌

    Mesh->OnComponentHit.AddDynamic(this, &AGrenade::OnHit);
    Mesh->OnComponentBeginOverlap.AddDynamic(this, &AGrenade::OnOverlapBegin);

    // 이동 성분: 속도, 중력, 튕김 등
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = 1200.f;
    ProjectileMovement->MaxSpeed = 1200.f;
    ProjectileMovement->bShouldBounce = false;  // 튕기지 않도록 변경
    ProjectileMovement->Bounciness = 0.0f;      // 튕김 제거
    ProjectileMovement->Friction = 0.2f;
    ProjectileMovement->ProjectileGravityScale = 1.0f;
    
    // ProjectileMovementComponent에 충돌 컴포넌트 설정
    ProjectileMovement->SetUpdatedComponent(Mesh);
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bInitialVelocityInLocalSpace = false;
}

// Called when the game starts or when spawned
void AGrenade::BeginPlay()
{
	Super::BeginPlay();

    // 플레이어와의 물리적 충돌 무시 (발사 직후 충돌 방지)
    if (AActor* OwnerActor = GetOwner())
    {
        if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(GetRootComponent()))
        {
            // 플레이어 자체와의 충돌 무시
            RootPrimitive->IgnoreActorWhenMoving(OwnerActor, true);
            
            // 플레이어의 컴포넌트들과의 충돌도 무시
            if (AZombieSurvivalCharacter* Player = Cast<AZombieSurvivalCharacter>(OwnerActor))
            {
                if (Player->GetCapsuleComponent())
                    RootPrimitive->IgnoreComponentWhenMoving(Player->GetCapsuleComponent(), true);
                if (Player->GetMesh())
                    RootPrimitive->IgnoreComponentWhenMoving(Player->GetMesh(), true);
                    
                UE_LOG(LogTemp, Warning, TEXT("Grenade set to ignore player collisions"));
            }
        }
    }

    // ProjectileMovementComponent의 충돌 이벤트 바인딩
    if (ProjectileMovement)
    {
        ProjectileMovement->OnProjectileStop.AddDynamic(this, &AGrenade::OnProjectileStop);
        UE_LOG(LogTemp, Warning, TEXT("ProjectileMovement OnProjectileStop event bound"));
    }

    // 2초 후 폭발하도록 타이머 설정 (백업용)
    GetWorldTimerManager().SetTimer(ExplosionTimerHandle, this, &AGrenade::Explode, 2.0f, false);
    
    UE_LOG(LogTemp, Warning, TEXT("Grenade created and timer set for 2 seconds"));
}

// Called every frame
void AGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// 벽에 부딪힐 때 호출 (즉시 폭발하도록 수정)
void AGrenade::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    UE_LOG(LogTemp, Warning, TEXT("OnHit called! OtherActor: %s, Owner: %s"), 
        OtherActor ? *OtherActor->GetName() : TEXT("NULL"),
        GetOwner() ? *GetOwner()->GetName() : TEXT("NULL"));

    // 이미 폭발했다면 무시
    if (bHasExploded)
    {
        UE_LOG(LogTemp, Warning, TEXT("Already exploded, ignoring hit"));
        return;
    }

    // 자기 자신과의 충돌은 무시
    if (!OtherActor || OtherActor == this)
    {
        UE_LOG(LogTemp, Warning, TEXT("Hit with self, ignoring"));
        return;
    }

    // 플레이어(Owner)와의 충돌은 무시 (발사 직후 충돌 방지)
    if (OtherActor == GetOwner())
    {
        UE_LOG(LogTemp, Warning, TEXT("Hit with owner (player), ignoring"));
        return;
    }

    // 플레이어의 무기와의 충돌도 무시
    if (GetOwner() && OtherActor->GetOwner() == GetOwner())
    {
        UE_LOG(LogTemp, Warning, TEXT("Hit with player's weapon, ignoring"));
        return;
    }

    // 그 외 모든 것과 충돌하면 즉시 폭발 (벽, 바닥, 적 등)
    UE_LOG(LogTemp, Warning, TEXT("Grenade hit: %s - Exploding immediately!"), *OtherActor->GetName());
    Explode();
}

//    ȣ ( )
void AGrenade::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UE_LOG(LogTemp, Warning, TEXT("OnOverlapBegin called! OtherActor: %s, Owner: %s"), 
        OtherActor ? *OtherActor->GetName() : TEXT("NULL"),
        GetOwner() ? *GetOwner()->GetName() : TEXT("NULL"));

    // 이미 폭발했다면 무시
    if (bHasExploded)
    {
        UE_LOG(LogTemp, Warning, TEXT("Already exploded, ignoring overlap"));
        return;
    }

    // 자기 자신과의 겹침은 무시
    if (!OtherActor || OtherActor == this)
    {
        UE_LOG(LogTemp, Warning, TEXT("Overlap with self, ignoring"));
        return;
    }

    // 플레이어(Owner)와의 겹침은 무시
    if (OtherActor == GetOwner())
    {
        UE_LOG(LogTemp, Warning, TEXT("Overlap with owner (player), ignoring"));
        return;
    }

    // 플레이어의 무기와의 겹침도 무시
    if (GetOwner() && OtherActor->GetOwner() == GetOwner())
    {
        UE_LOG(LogTemp, Warning, TEXT("Overlap with player's weapon, ignoring"));
        return;
    }

    // 적과 겹치면 즉시 폭발
    if (AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("Grenade overlapped with enemy: %s - Exploding immediately!"), *Enemy->GetName());
        Explode();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Overlap with non-enemy: %s - Not exploding"), *OtherActor->GetName());
    }
}

// ProjectileMovementComponent가 충돌을 감지했을 때 호출
void AGrenade::OnProjectileStop(const FHitResult& ImpactResult)
{
    UE_LOG(LogTemp, Warning, TEXT("OnProjectileStop called! Hit Actor: %s"), 
        ImpactResult.GetActor() ? *ImpactResult.GetActor()->GetName() : TEXT("NULL"));

    // 이미 폭발했다면 무시
    if (bHasExploded)
    {
        UE_LOG(LogTemp, Warning, TEXT("Already exploded, ignoring projectile stop"));
        return;
    }

    AActor* HitActor = ImpactResult.GetActor();

    // 자기 자신과의 충돌은 무시
    if (!HitActor || HitActor == this)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProjectileStop with self, ignoring"));
        return;
    }

    // 플레이어(Owner)와의 충돌은 무시
    if (HitActor == GetOwner())
    {
        UE_LOG(LogTemp, Warning, TEXT("ProjectileStop with owner (player), ignoring"));
        return;
    }

    // 플레이어의 무기와의 충돌도 무시
    if (GetOwner() && HitActor->GetOwner() == GetOwner())
    {
        UE_LOG(LogTemp, Warning, TEXT("ProjectileStop with player's weapon, ignoring"));
        return;
    }

    // 그 외 모든 것과 충돌하면 즉시 폭발
    UE_LOG(LogTemp, Warning, TEXT("Projectile stopped by: %s - Exploding immediately!"), *HitActor->GetName());
    Explode();
}

// 유탄 폭발 처리 함수
void AGrenade::Explode()
{
    // 이미 폭발했다면 중복 실행 방지
    if (bHasExploded)
    {
        return;
    }
    bHasExploded = true;

    // 타이머 취소 (혹시 모를 중복 호출 방지)
    GetWorldTimerManager().ClearTimer(ExplosionTimerHandle);

    // 폭발 범위 내 캐릭터 검색
    TArray<FOverlapResult> Overlaps;
    FCollisionShape ExplosionShape = FCollisionShape::MakeSphere(ExplosionRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this); // 자기 자신은 제외

    FVector Center = GetActorLocation();

    // 폭발 반경 시각화 디버그용 구
    DrawDebugSphere(GetWorld(), Center, ExplosionRadius, 16, FColor::Red, false, 1.5f);

    // 폭발 범위 내 캐릭터들을 검색
    bool bHit = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        Center,
        FQuat::Identity,
        ECC_Pawn,
        ExplosionShape,
        Params
    );

    if (bHit)
    {
        for (auto& Hit : Overlaps)
        {
            AActor* HitActor = Hit.GetActor();
            if (!HitActor) continue;

            // 중심점으로부터 실제 거리 계산 (정확한 판정)
            float Distance = FVector::Dist(HitActor->GetActorLocation(), Center);
            if (Distance > ExplosionRadius)
                continue;

            // 플레이어 데미지 처리
            if (AZombieSurvivalCharacter* Player = Cast<AZombieSurvivalCharacter>(HitActor))
            {
                Player->TakeDamage(ExplosionDamage);
            }
            // 적 데미지 처리
            else if (AEnemyBase* Enemy = Cast<AEnemyBase>(HitActor))
            {
                Enemy->TakeDamage(ExplosionDamage);
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Grenade exploded with damage: %.1f"), ExplosionDamage);

    // 폭발 후 유탄 제거
    Destroy();
}


