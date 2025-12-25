// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Crosshair.generated.h"

UCLASS()
class ZOMBIESURVIVAL_VER1_API ACrosshair : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void DrawHUD() override;
	
};
