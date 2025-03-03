// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIEntity.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, AIEntity, "AIEntity" );

void IController_INTF::INTF_Get_DebugInfo(ACharacter*& OutDebugFocusCharacter, bool& OutDebugView, bool& OutShowHUD,
	bool& OutShowTraces, bool& OutShowDebugShapes, bool& OutShowLayerColors, bool& OutSlomo, bool& OutShowCharacterInfo)
{
}

void ICharacter_INTF::INTF_Get_CurrentStates(
	TEnumAsByte<EMovementMode>& OutPawnMovementMode,
	MovementState& OutMovementState, MovementState& OutPrevMovementState, MovementAction& OutMovementAction,
	RotationMode& OutRotationMode, Gait& OutActualGait, Stance& OutActualStance, ViewMode& OutViewMode,
	OverlayState& OutOverlayState)
{
}

void ICharacter_INTF::INTF_Get_EssentialValues(FVector& OutVelocity, FVector& OutAcceleration, FVector& OutMovementInput,
	bool& OutIsMoving, bool& OutHasMovementInput, double& OutSpeed, double& OutMovementInputAmount,
	FRotator& OutAimingRotation, double& OutAimYawRate)
{
}

void ICharacter_INTF::INTF_Set_MovementState(MovementState NewMovementState)
{
}

void ICharacter_INTF::INTF_Set_MovementAction(MovementAction NewMovementAction)
{
}

void ICharacter_INTF::INTF_Set_OverlayState(OverlayState NewOverlayState)
{
}

void IAnimation_INTF::INTF_Jumped()
{
}

void IAnimation_INTF::INTF_SetGroundedEntryState(GroundedEntryState GroundedEntryState)
{
}

void IAnimation_INTF::INTF_SetOverlayOverrideState(int32 OverlayOverrideState)
{
}
