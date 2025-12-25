// Fill out your copyright notice in the Description page of Project Settings.


#include "TurretBuilderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Blueprint/UserWidget.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UTurretBuilderComponent::UTurretBuilderComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTurretBuilderComponent::BeginPlay()
{
	Super::BeginPlay();

	if (PreviewClass)
    {
        PreviewActor = GetWorld()->SpawnActor<AActor>(PreviewClass);
        if (PreviewActor)
        {
            PreviewActor->SetActorHiddenInGame(true);
            if (bDebug) UE_LOG(LogTemp, Warning, TEXT("PreviewActor SPAWN OK"));
        }
        else if (bDebug) UE_LOG(LogTemp, Error, TEXT("PreviewActor SPAWN FAILED"));
    }
}


// Called every frame
void UTurretBuilderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bPlacing) return;
    if (!PreviewActor) return;

    /* 1) 라인트레이스 – 화면 중앙(크로스헤어) 기준 */
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	APlayerController* PC = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
	if (!PC) return;

    // 화면 중앙에서 시작하는 라인트레이스
    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
    
    FVector TraceStart = CameraLocation;
    FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * 10000.f);

    FHitResult Hit;
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(OwnerPawn);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);

    if (!bHit)
    {
        PreviewActor->SetActorHiddenInGame(true);
        bCanPlace = false;
        return;
    }

	/* 2) 평지 판정 */
    const bool bFlat = MaxSlopeDot <= 0.0f ? true : FVector::DotProduct(Hit.ImpactNormal, FVector::UpVector) >= MaxSlopeDot;

    /* 3) Overlap 체크 */
    const FVector Loc = Hit.ImpactPoint;
    const float Radius = PreviewRadius;
    TArray<AActor*> Overlaps;
    
    // Pawn과 Vehicle 채널만 체크 (정적 메시는 무시)
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Vehicle));
    
    UKismetSystemLibrary::SphereOverlapActors(GetWorld(), Loc, Radius,
        ObjectTypes, nullptr, TArray<AActor*>{GetOwner()}, Overlaps);

    // 실제로 설치를 방해하는 액터들만 필터링
    int32 BlockingActorCount = 0;
    for (AActor* Actor : Overlaps)
    {
        if (!Actor) continue;
        
        // 포탑이나 다른 동적 오브젝트만 카운트
        if (Actor->GetName().Contains(TEXT("Turret")) || 
            Actor->GetName().Contains(TEXT("Building")) ||
            Actor->GetName().Contains(TEXT("Obstacle")) ||
            Actor->IsA<APawn>())
        {
            BlockingActorCount++;
            if (bDebug)
            {
                UE_LOG(LogTemp, Warning, TEXT("설치 방해 액터 감지: %s"), *Actor->GetName());
            }
        }
    }

    if (bDebug)
    {
        DrawDebugSphere(GetWorld(), Loc, Radius, 12,
            bFlat && BlockingActorCount == 0 ? FColor::Green : FColor::Red,
            false, 0.f, 0, 1.5f);
            
        // 설치 불가 원인 로그
        if (!bFlat)
        {
            UE_LOG(LogTemp, Warning, TEXT("설치 불가: 경사가 너무 가파름 (Dot: %.3f, 기준: %.3f)"), 
                FVector::DotProduct(Hit.ImpactNormal, FVector::UpVector), MaxSlopeDot);
        }
        if (BlockingActorCount > 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("설치 불가: 반지름 %.1f 내에 %d개 방해 액터 감지"), Radius, BlockingActorCount);
        }
    }

    bCanPlace = bFlat && BlockingActorCount == 0;

    /* 4) 프리뷰 갱신 */
	PreviewActor->SetActorHiddenInGame(false);
	PreviewActor->SetActorLocation(Loc);
	PreviewActor->SetActorRotation(FRotator(0.f, PC->GetControlRotation().Yaw, 0.f));

	FLinearColor Tint = bCanPlace ? FLinearColor::Green : FLinearColor::Red;
	UMeshComponent* MeshComp = PreviewActor->FindComponentByClass<UMeshComponent>();
	if (MeshComp)
	{
		MeshComp->SetVectorParameterValueOnMaterials("Tint", FVector(Tint)); // <- 타입 맞춤
	}
}

void UTurretBuilderComponent::ConfirmPlacement()
{
    if (!bCanPlace || !TurretClass) return;

    AActor* Turret = GetWorld()->SpawnActor<AActor>(
        TurretClass,
        PreviewActor->GetActorLocation(),
        PreviewActor->GetActorRotation());

    if (Turret)
    {
        UE_LOG(LogTemp, Warning, TEXT("포탑 설치 완료!"));
        
        // 포탑 설치 완료 이벤트 발생
        OnTurretPlaced.Broadcast();
        
        // 배치 모드 자동 해제
        SetPlacing(false);
    }

    // 성공적으로 설치됐음을 보여 주기 위해 프리뷰를 잠시 숨김
    PreviewActor->SetActorHiddenInGame(true);

    // 포탑 설치 후 로직 여기다가 추가
}

void UTurretBuilderComponent::SetPlacing(bool bEnable)
{
    bPlacing = bEnable;
    if (PreviewActor)
	{
		UE_LOG(LogTemp, Log, TEXT("PreviewActor EXIST."));
		PreviewActor->SetActorHiddenInGame(!bEnable);
	}

    if (bDebug) UE_LOG(LogTemp, Warning, TEXT("SetPlacing(%d)"), bEnable);
}
