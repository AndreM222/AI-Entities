// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionSetup.h"
#include "AIEntityCharacter.generated.h"

/**
 * 
 */
UCLASS()
class AIENTITY_API AAIEntityCharacter : public AActionSetup
{
	GENERATED_BODY()
protected:
	virtual void Tick(float DeltaTime) override;
	
};
