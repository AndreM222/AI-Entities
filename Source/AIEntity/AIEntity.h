// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveVector.h"
#include "AIEntity.generated.h" // Uclass, ustruct, uproperty, ufunction (Macro)

////////////// Enums //////////////
// -- (Data/Enum)

UENUM(BlueprintType)
enum class MovementState : uint8
{
	None UMETA(DisplayName = "None"),
	Grounded UMETA(DisplayName = "Grounded"),
	In_Air UMETA(DisplayName = "In Air"),
	Mantling UMETA(DisplayName = "Mantling"),
	Ragdoll UMETA(DisplayName = "Ragdoll")
};

UENUM(BlueprintType)
enum class Gait : uint8
{
	Walking UMETA(DisplayName = "Walking"),
	Running UMETA(DisplayName = "Running"),
	Sprinting UMETA(DisplayName = "Sprinting")
};

UENUM(BlueprintType)
enum class MovementAction : uint8
{
	None UMETA(DisplayName = "None"),
	LowMantle UMETA(DisplayName = "Low Mantle"),
	HighMantle UMETA(DisplayName = "High Mantle"),
	Rolling UMETA(DisplayName = "Rolling"),
	GettingUp UMETA(DisplayName = "Getting Up")
};

UENUM(BlueprintType)
enum class OverlayState : uint8
{
	Default UMETA(DisplayName = "Default"),
	Masculine UMETA(DisplayName = "Masculine"),
	Feminine UMETA(DisplayName = "Feminine"),
	Injured UMETA(DisplayName = "Injured"),
	HandsTied UMETA(DisplayName = "HandsTied"),
	Rifle UMETA(DisplayName = "Rifle"),
	Pistol_1H UMETA(DisplayName = "Pistol 1H"),
	Pistol_2H UMETA(DisplayName = "Pistol 2H"),
	Bow UMETA(DisplayName = "Bow"),
	Torch UMETA(DisplayName = "Torch"),
	Binoculars UMETA(DisplayName = "Binoculars"),
	Box UMETA(DisplayName = "Box"),
	Barrel UMETA(DisplayName = "Barrel")
};

UENUM(BlueprintType)
enum class RotationMode : uint8
{
	VelocityDirection UMETA(DisplayName = "Velocity Direction"),
	LookingDirection UMETA(DisplayName = "Looking Direction"),
	Aiming UMETA(DisplayName = "Aiming")
};

UENUM(BlueprintType)
enum class Stance : uint8
{
	Standing UMETA(DisplayName = "Standing"),
	Crouching UMETA(DisplayName = "Crouching")
};

UENUM(BlueprintType)
enum class ViewMode : uint8
{
	ThirdPerson UMETA(DisplayName = "ThirdPerson"),
	FirstPerson UMETA(DisplayName = "FirstPerson")
};

UENUM(BlueprintType)
enum class FootstepType : uint8
{
	Step UMETA(DisplayName = "Step"),
	Walk_Run UMETA(DisplayName = "Walk/Run"),
	Jump UMETA(DisplayName = "Jump/Jump"),
	Land UMETA(DisplayName = "Land")
};

UENUM(BlueprintType)
enum class GroundedEntryState : uint8
{
	None UMETA(DisplayName = "None"),
	Roll UMETA(DisplayName = "Roll")
};

UENUM(BlueprintType)
enum class HipsDirection : uint8
{
	F UMETA(DisplayName = "F"),
	B UMETA(DisplayName = "B"),
	RF UMETA(DisplayName = "RF"),
	RB UMETA(DisplayName = "RB"),
	LF UMETA(DisplayName = "LF"),
	LB UMETA(DisplayName = "LB")
};

UENUM(BlueprintType)
enum class MovementDirection : uint8
{
	Forward UMETA(DisplayName = "Forward"),
	Right UMETA(DisplayName = "Right"),
	Left UMETA(DisplayName = "Left"),
	Backward UMETA(DisplayName = "Backward")
};

UENUM(BlueprintType)
enum class MantleType : uint8
{
    HighMantle UMETA(DisplayName = "High Mantle"),
	LowMantle UMETA(DisplayName = "Low Mantle"),
	FallingCatch UMETA(DisplayName = "Falling Catch")
};

///////////// Structs /////////////
// -- (Data/Structs)

USTRUCT(BlueprintType)
struct FComponentAndTransform
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FTransform Transform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPrimitiveComponent* Component;
};

USTRUCT(BlueprintType)
struct FDynamicMontageParams
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimSequenceBase* Animation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlendInTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlendOutTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartTime;
};

USTRUCT(BlueprintType)
struct FLeanAmount
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FB;
};

USTRUCT(BlueprintType)
struct FVelocityBlend
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float B;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float L;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float R;
};

USTRUCT(BlueprintType)
struct FTurnInPlace_Asset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimSequenceBase* Animation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AnimatedAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool ScaleTurnAngle;
};

USTRUCT(BlueprintType)
struct FRotateInPlace_Asset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimSequenceBase* Animation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlowTurnRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FastTurnRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlowPlayRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FastPlayRate;
};

USTRUCT(BlueprintType)
struct FCameraSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetArmLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SocketOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LagSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotationLagSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool DoCollisionTest;
};

USTRUCT(BlueprintType)
struct FMovementSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RunSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SprintSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveVector* MovementCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* RotationRateCurve;
};

USTRUCT(BlueprintType)
struct FMovementSettings_Stance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMovementSettings Standing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMovementSettings Crouching;
};

USTRUCT(BlueprintType)
struct FMovementSettings_State: public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMovementSettings_Stance VelocityDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMovementSettings_Stance LookingDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMovementSettings_Stance Aiming;
};

USTRUCT(BlueprintType)
struct FCameraSettings_Gait
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCameraSettings Walking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCameraSettings Running;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCameraSettings Sprinting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCameraSettings Crouching;
};

USTRUCT(BlueprintType)
struct FMantle_TraceSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxLedgeHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinLedgeHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReachDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ForwardTraceRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownwardTraceRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool HasDesiredLocation;
};

USTRUCT(BlueprintType)
struct FMantleAsset
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* AnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveVector* PositionAndCorrectiveCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector StartingOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LowHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LowPlayRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LowStartPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighPlayRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighStartPosition;
};

USTRUCT(BLueprintType)
struct FMantleParams
{
	GENERATED_BODY()
	
    UPROPERTY(editAnywhere, BlueprintReadWrite)
	UAnimMontage* AnimMontage;

	UPROPERTY(editAnywhere, BlueprintReadWrite)
	UCurveVector* PositionAndCorrectiveCurve;

	UPROPERTY(editAnywhere, BlueprintReadWrite)
	float StartingPosition;

	UPROPERTY(editAnywhere, BlueprintReadWrite)
	float PlayRate;

	UPROPERTY(editAnywhere, BlueprintReadWrite)
	FVector StartingOffset;
};

//////////// Interface ////////////
// -- (../Blueprint/Interfaces)

UINTERFACE(Blueprintable, BlueprintType)
class UController_INTF : public UInterface
{
	GENERATED_BODY()
};

class IController_INTF
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void INTF_Get_DebugInfo(
		ACharacter*& OutDebugFocusCharacter,
		bool& OutDebugView,
		bool& OutShowHUD,
		bool& OutShowTraces,
		bool& OutShowDebugShapes,
		bool& OutShowLayerColors,
		bool& OutSlomo,
		bool& OutShowCharacterInfo
	);

};

UINTERFACE(Blueprintable, BlueprintType)
class UCharacter_INTF : public UInterface
{
	GENERATED_BODY()
};

class ICharacter_INTF
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void INTF_Get_CurrentStates(
		TEnumAsByte<EMovementMode>& OutPawnMovementMode,
		MovementState& OutMovementState,
		MovementState& OutPrevMovementState,
		MovementAction& OutMovementAction,
		RotationMode& OutRotationMode,
		Gait& OutActualGait,
		Stance& OutActualStance,
		ViewMode& OutViewMode,
		OverlayState& OutOverlayState
	);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void INTF_Get_EssentialValues(
		FVector& OutVelocity,
		FVector& OutAcceleration,
		FVector& OutMovementInput,
		bool& OutIsMoving,
		bool& OutHasMovementInput,
		double& OutSpeed,
		double& OutMovementInputAmount,
		FRotator& OutAimingRotation,
		double& OutAimYawRate
	);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void INTF_Set_MovementState(MovementState NewMovementState);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void INTF_Set_MovementAction(MovementAction NewMovementAction);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void INTF_Set_RotationMode(RotationMode NewRotationMode);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void INTF_Set_Gait(Gait NewGait);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void INTF_Set_ViewMode(ViewMode NewViewMode);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void INTF_Set_OverlayState(OverlayState NewOverlayState);
};


UINTERFACE(Blueprintable, BlueprintType)
class UAnimation_INTF : public UInterface
{
	GENERATED_BODY()
};

class IAnimation_INTF : public UInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	 void INTF_Jumped();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	 void INTF_SetGroundedEntryState(GroundedEntryState GroundedEntryState);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	 void INTF_SetOverlayOverrideState(int32 OverlayOverrideState);
};