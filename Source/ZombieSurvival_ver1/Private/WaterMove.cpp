// Fill out your copyright notice in the Description page of Project Settings.


#include "WaterMove.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AWaterMove::AWaterMove()
{
	PrimaryActorTick.bCanEverTick = true;

	boxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Move Water Box Collider"));
	SetRootComponent(boxComp);
	boxComp->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Move Water Static mesh"));
	meshComp->SetupAttachment(boxComp);

	boxComp->SetCollisionProfileName("Water");
	UE_LOG(LogTemp, Log, TEXT("WaterMove Constructor"));
}

void AWaterMove::BeginPlay()
{
	Super::BeginPlay();
	boxComp->OnComponentBeginOverlap.AddDynamic(this, &AWaterMove::OnWaterOverlap);
	boxComp->OnComponentEndOverlap.AddDynamic(this, &AWaterMove::OutWaterOverlap);

}

void AWaterMove::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (onWater == true && character != nullptr)
	{
		FVector characterLocation = character->GetActorLocation();

		// FloorWater�� �ĸ����
		FVector forwardVector = -GetActorForwardVector();
		FVector NewLocation = characterLocation + forwardVector * moveDistance;

		character->SetActorLocation(NewLocation, true);
	}
}

//�� ���� ���� ��� ���������� ������
void AWaterMove::OnWaterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!onWater)
	{
		AZombieSurvivalCharacter* overlapCharacter = Cast<AZombieSurvivalCharacter>(OtherActor);
		if (overlapCharacter)
		{
			character = overlapCharacter;
			onWater = true;
			UE_LOG(LogTemp, Log, TEXT("OnWaterOverlap"));
		}
	}
}
//������ ����
void AWaterMove::OutWaterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == character)
	{
		onWater = false;
		character = nullptr;
		UE_LOG(LogTemp, Log, TEXT("OutWaterOverlap"));
	}
}

