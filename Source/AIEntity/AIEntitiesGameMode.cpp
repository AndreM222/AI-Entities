// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIEntitiesGameMode.h"
#include "ActionSetup.h"
#include "UObject/ConstructorHelpers.h"

AAIEntitiesGameMode::AAIEntitiesGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/AI-Entities/BP_UserCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
