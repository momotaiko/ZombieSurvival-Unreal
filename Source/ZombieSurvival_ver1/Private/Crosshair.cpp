// Fill out your copyright notice in the Description page of Project Settings.


#include "Crosshair.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

void ACrosshair::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas) return;

	// 화면 중앙 계산
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
	const float Size = 6.0f;
	const float Thickness = 1.5f;

	// 붉은 십자선
	DrawLine(Center.X - Size, Center.Y, Center.X + Size, Center.Y, FLinearColor::Red, Thickness);
	DrawLine(Center.X, Center.Y - Size, Center.X, Center.Y + Size, FLinearColor::Red, Thickness);
}
