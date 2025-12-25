// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TurretBuilderComponent.generated.h"

// 포탑 설치 완료 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTurretPlaced);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZOMBIESURVIVAL_VER1_API UTurretBuilderComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTurretBuilderComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/* -------- 설정 -------- */
    UPROPERTY(EditDefaultsOnly) TSubclassOf<AActor> PreviewClass;         // BP_TurretPreview
    UPROPERTY(EditAnywhere) float MaxSlopeDot = 0.85f;                // 약 32도 이하 (블루프린트에서 조정 가능)
	UPROPERTY(EditAnywhere) float PreviewRadius = 30.f;               // 겹침 검사 반지름 (작을수록 관대)

    /* -------- 런타임 -------- */
    UPROPERTY() AActor* PreviewActor = nullptr;
    bool bCanPlace = false;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/* 입력에서 호출 : 실제 설치 */
    UFUNCTION(BlueprintCallable)
    void ConfirmPlacement();

	bool bPlacing = false; // 지금 배치 모드인지?

	UFUNCTION(BlueprintCallable)
	void SetPlacing(bool bEnable);

	// TurretBuilderComponent.h ─────────
	UPROPERTY(EditAnywhere) bool bDebug = true;

	// 포탑 클래스 (외부에서 설정 가능)
	UPROPERTY(EditDefaultsOnly) TSubclassOf<AActor> TurretClass;          // BP_Turret

	// 포탑 설치 완료 이벤트
	UPROPERTY(BlueprintAssignable)
	FOnTurretPlaced OnTurretPlaced;
};
