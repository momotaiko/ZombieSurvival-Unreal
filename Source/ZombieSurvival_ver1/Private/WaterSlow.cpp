#include "WaterSlow.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AWaterSlow::AWaterSlow()
{
	PrimaryActorTick.bCanEverTick = false;

	boxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Move Water Box Collider"));
	SetRootComponent(boxComp);
	boxComp->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Move Water Static mesh"));
	meshComp->SetupAttachment(boxComp);

	boxComp->SetCollisionProfileName("Water");
	UE_LOG(LogTemp, Log, TEXT("WaterSlow Constructor"));
}

void AWaterSlow::BeginPlay()
{
	Super::BeginPlay();

	boxComp->OnComponentBeginOverlap.AddDynamic(this, &AWaterSlow::OnWaterOverlap);
	boxComp->OnComponentEndOverlap.AddDynamic(this, &AWaterSlow::OutWaterOverlap);

}

//�� ���� ���� ��� �ӵ� ����
void AWaterSlow::OnWaterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Log, TEXT("OnWaterOverlap"));
	character = Cast<AZombieSurvivalCharacter>(OtherActor);
	if (character)
		character->GetCharacterMovement()->MaxWalkSpeed = slowSpeed;

}
//�ӵ� ���� ����
void AWaterSlow::OutWaterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Log, TEXT("OutWaterOverlap"));
	character = Cast<AZombieSurvivalCharacter>(OtherActor);
	if (character)
		character->GetCharacterMovement()->MaxWalkSpeed = defaultSpeed;

}
