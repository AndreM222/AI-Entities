// Fill out your copyright notice in the Description page of Project Settings.


#include "AIEntityCharacter.h"

void AAIEntityCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	AdvanceMove(FVector2d(0,1));
}