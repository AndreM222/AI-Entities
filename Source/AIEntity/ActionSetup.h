/** Copyright Epic Games, Inc. All Rights Reserved. */

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
	/** Initial setup of character */
	AActionSetup();

protected:
	/*********************************************************************************************
	* Action Functions
	********************************************************************************************* */

	/** On game begin event */
	virtual void BeginPlay() override;

	/**
	 * Event every second game runs
	 * 
	 * @param DeltaTime In game time
	 */
	virtual void Tick(float DeltaTime) override;

	/**
	 * On movement mode change event
	 * 
	 * @param PrevMovementMode Previous movement mode
	 * @param PreviousCustomMode Previous custom movement mode
	 */
	void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	/** On jump Event */
	void Jump() override;

	/** On Crouch Event */
	void Crouch(bool bClientSimulation = false) override;

	/** On UnCrouch Event */
	void UnCrouch(bool bClientSimulation = false) override;

	/**
	 * On land event
	 * 
	 * @param Hit Platform landed information
	 */
	void Landed(const FHitResult& Hit) override;

	/**
	 * Movement input
	 *
	 * @param Value Direction to move to
	 */
	void AdvanceMove(const FInputActionValue& Value);

	/**
	 * Looking input
	 *
	 * @param Value Direction to look to
	 */
	void AdvanceLook(const FInputActionValue& Value);

	/** Jumping Action Start */
	void AdvanceJump(double ForwardDesiredLocation = 0, double RightDesiredLocation = 0);

	/** Crouch Action */
	void AdvanceCrouch();

	/** Roll Action */
	void AdvanceRoll();

	/** Sprint Action Start */
	void AdvanceSprintStart();
	
	/** Sprint Action End */
	void AdvanceSprintEnd();

	/** Ragdoll Action */
	void AdvanceRagdoll();

	/** Aim Action Start */
	void AdvanceAimStart();
	
	/** Aim Action End */
	void AdvanceAimEnd();

	/**
	 * Set list of roll montage
	 * 
	 * @param PathFolder Path to folder of animations
	 * @param States Set montage based on state
	 */
	void SetRollMontage(FString PathFolder, TMap<FString, TArray<OverlayState>> States);

	/**
	 * Set list of mantle/climb montage
	 * 
	 * @param PathFolder Path to folder of animations
	 * @param States Set montage based on state
	 */
	void SetMantleMontage(FString PathFolder, TMap<FString, TArray<OverlayState>> States);

	/**
	 * Set list of ragdoll montage
	 *
	 * @param PathFolder Path to folder of animations
	 * @param States Set montage based on state
	 * @param IsFront Is the character facing up in ragdoll
	 */
	void SetRagdollMontage(FString PathFolder, TMap<FString, TArray<OverlayState>> States, bool IsFront = true);

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }


	/*********************************************************************************************
	* Interface
	********************************************************************************************* */

	/**
	 * Get character states
	 *
     * @param OutPawnMovementMode Pawn movement mode
     * @param OutMovementState Current state of movement
     * @param OutPrevMovementState Previous state of movement
     * @param OutMovementAction Current type of movement action
     * @param OutRotationMode Current rotation mode
     * @param OutActualGait Current type of movement
     * @param OutActualStance Current stance of character
     * @param OutViewMode Type of camera view mode
     * @param OutOverlayState Type of state
	 */
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

	/**
	 * Setup new movement state
	 * 
	 * @param NewMovementState Setup new movement state
	 */
	virtual void INTF_Set_MovementState_Implementation(MovementState NewMovementState) override;

	/**
	 * Setup new action state
	 * 
	 * @param NewMovementAction Setup new movement action
	 */
	virtual void INTF_Set_MovementAction_Implementation(MovementAction NewMovementAction) override;

	/**
	 * Set new character state
	 * 
	 * @param NewOverlayState Setup new state
	 */
	virtual void INTF_Set_OverlayState_Implementation(OverlayState NewOverlayState) override;

	/**
	 * Set up essential values for animation
	 *
     * @param OutVelocity Character velocity
     * @param OutAcceleration Character acceleration
     * @param OutMovementInput Character movement
     * @param OutIsMoving Is character moving
     * @param OutHasMovementInput Can character move
     * @param OutSpeed Current speed
     * @param OutMovementInputAmount Current character movement amount
     * @param OutAimingRotation Aim rotation
     * @param OutAimYawRate Aim rate of rotation
	 */
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
	/*********************************************************************************************
	* Movement Values
	********************************************************************************************* */

	/** Character rotation in air */
	FRotator InAirRotation;

	/** Character current speed */
	float Speed;

	/** Amount of movement input received */
	float MovementInputAmount;

	/** Returns if character is moving */
	bool IsMoving;

	/** Current aim yaw rate for rotation */
	float AimYawRate;

	/** Previous aim yaw from rotation */
	float PreviousAimYaw;

	/** Curve asset for movement transitions */
	UCurveFloat* CurveFloat;

	/** Animation instance reference */
	UAnimInstance* AnimInstance;

	/** Last ragdoll velocity */
	FVector LastRagdollVelocity;

	/** Last ragdoll velocity */
	FVector TargetRagdollLocation;

	/** Returns if character is facing up on ragdoll */
	bool RagdollFaceUp;

	/** Returns if character has hit ground on ragdoll */
	bool RagdollOnGround;

	/** Current movement settings */
	FMovementSettings CurrentMovementSettings;

	/** Get data table with movement settings */
	FDataTableRowHandle MovementModel;

	/** Obtained Movement data from data table */
	FMovementSettings_State* MovementData;

	/** Current character acceleration */
	FVector Acceleration;

	/** Previous character velocity */
	FVector PreviousVelocity;

	/** Last input for rotation */
	FRotator LastMovementInputRotation;

	/** Last rotations velocity */
	FRotator LastVelocityRotation;

	/** Desired rotation */
	FRotator TargetRotation;

	/** Current state of movement */
	MovementState CurrentMovementState;

	/** Previous state of movement */
	MovementState PrevMovementState;

	/** Current type of movement action */
	MovementAction CurrentMovementAction;
	
	/** Previous type of movement action */
	MovementAction PrevMovementAction;

	/** Current rotation mode */
	RotationMode CurrentRotationMode;

	/** Current type of movement */
	Gait CurrentActualGait;

	/** Current stance of character */
	Stance CurrentStance;
	
	/** Desired stance of character */
	Stance DesiredStance;

	/** Type of camera view mode */
	ViewMode CurrentViewMode;

	/** Type of state */
	OverlayState CurrentOverlayState;

	/** Type of mantle/climbing */
	MantleType CurrentMantleType;

	/** Type of movement to transition to */
	Gait DesiredGait;

	/** Data for mantle/climbing when on ground */
	FMantle_TraceSettings GroundedTraceSettings;

	/** Data for mantle/climbing when falling */
	FMantle_TraceSettings FallingTraceSettings;

	/** Get mantle ledge data from world for moving object */
	FComponentAndTransform MantleLedgeLs;

	/** Get mantle/climbing data */
	FMantleParams MantleParams;

	/** Get current start offset */
	FTransform ActualStartOffset;

	/** Get animation start data */
	FTransform AnimatedStartOffset;

	/** Timeline for smooth transition on mantle/climbing */
	UTimelineComponent* MantleTimeline;

	/** Desired climbing/mantle data */
	FTransform MantleTarget;

	/** Reference to climbing/mantle finish function */
	FOnTimelineFloat MantleProgress;

	/** Reference to climbing/mantle finish function */
	FOnTimelineEvent MantleFinish;

	/** Returns if character can move */
	bool HasMovementInput;

	/** Returns if braking the fall with a roll */
	bool BreakFall;

	/*********************************************************************************************
	* Montage Setup
	********************************************************************************************* */

	/** Is a list for the roll animations per character state*/
	TMap<OverlayState, UAnimMontage*> RollMontage;

	/** Is a list for the Mantle/climbing animations per character state*/
	TMap<OverlayState, UAnimMontage*> MantleMontage;

	/** Is a list for the standing up from ragdoll looking face down animations per character state*/
	TMap<OverlayState, UAnimMontage*> RagdollMontageFront;

	/** Is a list for the standing up from ragdoll looking face up animations per character state*/
	TMap<OverlayState, UAnimMontage*> RagdollMontageBack;

	/** Mantle data used for climbing*/
	FMantleAsset MantleAsset;

	/*********************************************************************************************
	* Climbing/Mantle Setup
	********************************************************************************************* */

	/**
	 * Check if mantle/climbing is possible
	 *
	 * @param TraceSettings Settings for mantle/climbing
	 * @param ForwardDesiredLocation Detect forward/backward axis for climbing
	 * @param RightDesiredLocation Detect right/left axis for climbing
	 * @param Debug Trace draw for debugging
	 */
	bool MantleCheck(
		FMantle_TraceSettings TraceSettings,
		double ForwardDesiredLocation,
		double RightDesiredLocation,
		EDrawDebugTrace::Type Debug = EDrawDebugTrace::None
	);

	/**
	 * Starts the climbing/mantle action
	 * 
	 * @param MantleHeight Set height at which ledge is located
	 * @param MantleLedgeWS Obtain data from ledge transformation
	 * @param mantleType Obtain type of mantle/climbing for animation
	 */
	void MantleStart(float MantleHeight, FComponentAndTransform MantleLedgeWS, MantleType mantleType);

	/*********************************************************************************************
	* Values Setup
	********************************************************************************************* */

	/** Update all the values needed for character movement */
	void UpdateCharacterMovement();

	/** Setup data for movement */
	void EssentialValuesSetup();

	/** Start updating rotation on the ground with smooth animations */
	void UpdateGroundRotation();

	/** Start updating rotation on the air with smooth animations */
	void UpdateAirRotation();

	/**
	 * Obtain position, rotation, and other values when on ragdoll
	 *
	 * @param Debug Trace draw for debugging
	 */
	void UpdateRagdoll(EDrawDebugTrace::Type Debug = EDrawDebugTrace::None);

	/**
	 * Rotate the character with smooth transitions
	 *
	 * @param Target Desired final rotation
	 * @param TargetInterpSpeed Desired speed for rotation transition
	 * @param ActorInterpSpeed Speed at which is currently being transitioned
	 */
	void SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed);

	/** Maps speed to an appropriate animation curve */
	float MappedSpeed();

	/**
	 * Retrieves the roll animation for the given state
	 *
	 * @param State State which animation is desired
	 */
	UAnimMontage* GetRollMontage(OverlayState State);

	/**
	 * Retrieves the mantle animation for the given state
	 *
	 * @param State State which animation is desired
	 */
	UAnimMontage* GetMantleMontage(OverlayState State);

	/**
	 * Retrieves the ragdoll animation for the given state
	 *
	 * @param State State which animation is desired
	 * @param FaceUp Is character looking up
	 */
	UAnimMontage* GetRagdollMontage(OverlayState State, bool FaceUp = true);

	/*********************************************************************************************
	* Timeline Functions
	********************************************************************************************* */

	/**
	 * Update the mantling/climbing timeline for smooth transition
	 *
	 * @param BlendIn Time from timeline
	 */
	UFUNCTION()
	void MantleUpdate(float BlendIn);

	/** When climbing/mantling ends, give mobility back to character */
	UFUNCTION()
	void MantleEnd();
};
