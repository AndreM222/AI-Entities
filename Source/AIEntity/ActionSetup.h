// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIEntity.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "InputActionValue.h"
#include "Components/TimelineComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ActionSetup.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AActionSetup : public ACharacter, public ICharacter_INTF
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

public:
	AActionSetup();

protected:
	////////////////////////////////////////////////////////////////////////////////////
	// Action Functions

	// movement input
	void AdvanceMove(const FInputActionValue& Value);

	// looking input
	void AdvanceLook(const FInputActionValue& Value);

	// stance input
	void SetState(OverlayState stance);

	// Jump Action
	void AdvanceJump(double ForwardDesiredLocation = 0, double RightDesiredLocation = 0);

	// Crouch Action
	void AdvanceCrouch();

	// Roll Action
	void AdvanceRoll();

	// Sprint Action
	void AdvanceSprintStart();
	void AdvanceSprintEnd();

	// Ragdoll Action
	void AdvanceRagdoll();

	// Aim Action
	void AdvanceAimStart();
	void AdvanceAimEnd();

	////////////////////////////////////////////////////////////////////////////////////
	// Event Functions

	void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	void Jump() override;

	void Landed(const FHitResult& Hit) override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	UCurveFloat* CurveFloat;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	////////////////////////////////////////////////////////////////////////////////////
	// Interface

	virtual void INTF_Get_CurrentStates_Implementation(
		TEnumAsByte<EMovementMode>& OutPawnMovementMode,
		MovementState& OutMovementState,
		MovementState& OutPrevMovementState,
		MovementAction& OutMovementAction,
		RotationMode& OutRotationMode,
		Gait& OutActualGait,
		Stance& OutActualStance,
		ViewMode& OutViewMode,
		OverlayState& OutOverlayState
	) override;

	virtual void INTF_Set_MovementState_Implementation(MovementState NewMovementState) override;

	virtual void INTF_Get_EssentialValues_Implementation(
		FVector& OutVelocity,
		FVector& OutAcceleration,
		FVector& OutMovementInput,
		bool& OutIsMoving,
		bool& OutHasMovementInput,
		double& OutSpeed,
		double& OutMovementInputAmount,
		FRotator& OutAimingRotation,
		double& OutAimYawRate
	) override;

private:
	////////////////////////////////////////////////////////////////////////////////////
	// Variables
	FRotator InAirRotation;
	float Speed;
	UAnimInstance* AnimInstance;
	float MovementInputAmount;
	bool IsMoving;

	// Rotation
	float AimYawRate;
	float PreviousAimYaw;

	// Ragdoll
	FVector LastRagdollVelocity;
	FVector TargetRagdollLocation;
	bool RagdollFaceUp;
	bool RagdollOnGround;

	// Movement
	FMovementSettings CurrentMovementSettings;
	FDataTableRowHandle MovementModel;
	FMovementSettings_State* MovementData;
	FVector Acceleration;
	FVector PreviousVelocity;

	FRotator LastMovementInputRotation;
	FRotator LastVelocityRotation;
	FRotator TargetRotation;

	// States
	MovementState CurrentMovementState;
	MovementState PrevMovementState;
	MovementAction CurrentMovementAction;
	RotationMode CurrentRotationMode;
	Gait CurrentActualGait;
	Stance CurrentStance;
	ViewMode CurrentViewMode;
	OverlayState CurrentOverlayState;

	MantleType CurrentMantleType;
	Gait desiredGait;

	// Mantle
	FMantle_TraceSettings GroundedTraceSettings = FMantle_TraceSettings(250, 50, 75, 30, 30, false);
	FMantle_TraceSettings FallingTraceSettings = FMantle_TraceSettings(150, 50, 70, 30, 30, false);
	FComponentAndTransform MantleLedgeLs;
	FMantleParams MantleParams;
	FTransform ActualStartOffset;
	FTransform AnimatedStartOffset;
	UTimelineComponent* MantleTimeline;
	FTransform MantleTarget;
	FOnTimelineFloat MantleProgress;
	FOnTimelineEvent MantleFinish;

	// Movement
	bool HasMovementInput;
	bool BreakFall;

	// Montage Setup
	TMap<OverlayState, UAnimMontage*> RollMontage;
	TMap<OverlayState, UAnimMontage*> MantleMontage;
	TMap<OverlayState, UAnimMontage*> RagdollMontageFront;
	TMap<OverlayState, UAnimMontage*> RagdollMontageBack;

	////////////////////////////////////////////////////////////////////////////////////
	// Functions

	bool MantleCheck(
		FMantle_TraceSettings traceSettings,
		double ForwardDesiredLocation,
		double RightDesiredLocation,
		EDrawDebugTrace::Type debug = EDrawDebugTrace::None
	);

	void MantleStart(float mantleHeight, FComponentAndTransform mantleLedgeWS, MantleType mantleType);

	void UpdateCharacterMovement();

	void EssentialValuesSetup();

	void UpdateGroundRotation();

	void UpdateAirRotation();

	void UpdateRagdoll(EDrawDebugTrace::Type debug = EDrawDebugTrace::None);

	void SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed);

	float MappedSeed();

	////////////////////////////////////////////////////////////////////////////////////
	// Timeline Functions

	UFUNCTION()
	void MantleUpdate(float BlendIn);

	UFUNCTION()
	void MantleEnd();
};
