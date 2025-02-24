// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionSetup.h"

#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveVector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AActionSetup::AActionSetup()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(35.f, 90.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 420.f;
	GetCharacterMovement()->AirControl = 0.15f;
	GetCharacterMovement()->MaxAcceleration = 1500.f;
	GetCharacterMovement()->bUseSeparateBrakingFriction = 0;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 300.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 25.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2048.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 0.0f;
	GetCharacterMovement()->SetCrouchedHalfHeight(60.0f);

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Setup Timeline Component
	MantleTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("MantleTimeline"));
	CurveFloat = LoadObject<UCurveFloat>(
		nullptr, TEXT("/Game/CharacterAnimSetup/Data/Curves/MantleCurves/MantleTimeline"));
	MantleProgress.BindUFunction(this, FName("MantleUpdate"));
	MantleFinish.BindUFunction(this, FName("MantleEnd"));

	// Set To Not Climbable
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);

	// Set Default Values
	CurrentMovementState = MovementState::Grounded;
	PrevMovementState = MovementState::Grounded;
	CurrentMovementAction = MovementAction::None;
	CurrentRotationMode = RotationMode::LookingDirection;
	CurrentActualGait = Gait::Running;
	CurrentStance = Stance::Standing;
	CurrentViewMode = ViewMode::ThirdPerson;
	CurrentOverlayState = OverlayState::Default;

	GroundedTraceSettings = FMantle_TraceSettings(
		250,
		50,
		75,
		30,
		30,
		false
	);
	FallingTraceSettings = FMantle_TraceSettings(
		150,
		50,
		70,
		30,
		30,
		false
	);

	// Movement Setting
	CurrentMovementSettings = FMovementSettings(
		165,
		350,
		600,
		LoadObject<UCurveVector>(
			nullptr, TEXT("/Game/CharacterAnimSetup/Data/Curves/CharacterMovementCurves/NormalMovement")),
		nullptr
	);

	// Setup default anims

	// Roll - Montage
	RollMontage.Add(
		OverlayState::Default,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_LandRoll_F_Montage_Default"))
	);

	RollMontage.Add(
		OverlayState::Masculine,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_LandRoll_F_Montage_Default"))
	);

	RollMontage.Add(
		OverlayState::Feminine,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_LandRoll_F_Montage_Default"))
	);

	RollMontage.Add(
		OverlayState::Injured,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_LandRoll_F_Montage_LH"))
	);

	RollMontage.Add(
		OverlayState::Bow,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_LandRoll_F_Montage_LH"))
	);

	RollMontage.Add(
		OverlayState::Torch,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_LandRoll_F_Montage_LH"))
	);

	RollMontage.Add(
		OverlayState::Barrel,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_LandRoll_F_Montage_LH"))
	);

	RollMontage.Add(
		OverlayState::Pistol_1H,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_LandRoll_F_Montage_RH"))
	);

	RollMontage.Add(
		OverlayState::Pistol_2H,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_LandRoll_F_Montage_RH"))
	);

	RollMontage.Add(
		OverlayState::HandsTied,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_LandRoll_F_Montage_2H"))
	);

	RollMontage.Add(
		OverlayState::Rifle,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_LandRoll_F_Montage_2H"))
	);

	RollMontage.Add(
		OverlayState::Binoculars,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_LandRoll_F_Montage_2H"))
	);

	RollMontage.Add(
		OverlayState::Box,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_LandRoll_F_Montage_2H"))
	);

	// Mantle - Montage

	MantleMontage.Add(
		OverlayState::Default,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_Mantle_1m_Montage_Default"))
	);

	MantleMontage.Add(
		OverlayState::Masculine,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_Mantle_1m_Montage_Default"))
	);

	MantleMontage.Add(
		OverlayState::Feminine,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_Mantle_1m_Montage_Default"))
	);

	MantleMontage.Add(
		OverlayState::Injured,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_Mantle_2m_Montage_Default"))
	);

	MantleMontage.Add(
		OverlayState::Bow,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_Mantle_2m_Montage_Default"))
	);

	MantleMontage.Add(
		OverlayState::Torch,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_Mantle_2m_Montage_Default"))
	);

	MantleMontage.Add(
		OverlayState::Barrel,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_Mantle_2m_Montage_Default"))
	);

	MantleMontage.Add(
		OverlayState::Pistol_1H,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_Mantle_1m_Montage_RH"))
	);

	MantleMontage.Add(
		OverlayState::Pistol_2H,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_Mantle_1m_Montage_RH"))
	);

	MantleMontage.Add(
		OverlayState::Rifle,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_Mantle_1m_Montage_RH"))
	);

	MantleMontage.Add(
		OverlayState::Binoculars,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_Mantle_1m_Montage_RH"))
	);

	MantleMontage.Add(
		OverlayState::HandsTied,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_Mantle_2m_Montage_Default"))
	);

	MantleMontage.Add(
		OverlayState::Box,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_Mantle_1m_Montage_Box"))
	);

	// Mantle - Montage - Back

	RagdollMontageBack.Add(
		OverlayState::Default,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Back_Montage_Default"))
	);

	RagdollMontageBack.Add(
		OverlayState::Masculine,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Back_Montage_Default"))
	);

	RagdollMontageBack.Add(
		OverlayState::Feminine,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Back_Montage_Default"))
	);

	RagdollMontageBack.Add(
		OverlayState::Injured,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Back_Montage_Default"))
	);

	RagdollMontageBack.Add(
		OverlayState::Bow,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Back_Montage_Default"))
	);

	RagdollMontageBack.Add(
		OverlayState::Torch,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Back_Montage_Default"))
	);

	RagdollMontageBack.Add(
		OverlayState::Barrel,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Back_Montage_Default"))
	);

	RagdollMontageBack.Add(
		OverlayState::Pistol_1H,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Back_Montage_RH"))
	);

	RagdollMontageBack.Add(
		OverlayState::Pistol_2H,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Back_Montage_RH"))
	);

	RagdollMontageBack.Add(
		OverlayState::Rifle,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Back_Montage_RH"))
	);

	RagdollMontageBack.Add(
		OverlayState::Binoculars,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Back_Montage_RH"))
	);

	RagdollMontageBack.Add(
		OverlayState::HandsTied,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Back_Montage_2H"))
	);

	RagdollMontageBack.Add(
		OverlayState::Box,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Back_Montage_2H"))
	);

	// Mantle - Montage - Front

	RagdollMontageFront.Add(
		OverlayState::Default,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Front_Montage_Default"))
	);

	RagdollMontageFront.Add(
		OverlayState::Masculine,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Front_Montage_Default"))
	);

	RagdollMontageFront.Add(
		OverlayState::Feminine,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Front_Montage_Default"))
	);

	RagdollMontageFront.Add(
		OverlayState::Injured,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Front_Montage_Default"))
	);

	RagdollMontageFront.Add(
		OverlayState::Bow,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Front_Montage_Default"))
	);

	RagdollMontageFront.Add(
		OverlayState::Torch,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Front_Montage_Default"))
	);

	RagdollMontageFront.Add(
		OverlayState::Barrel,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Front_Montage_Default"))
	);

	RagdollMontageFront.Add(
		OverlayState::Pistol_1H,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Front_Montage_RH"))
	);

	RagdollMontageFront.Add(
		OverlayState::Pistol_2H,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Front_Montage_RH"))
	);

	RagdollMontageFront.Add(
		OverlayState::Rifle,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Front_Montage_RH"))
	);

	RagdollMontageFront.Add(
		OverlayState::Binoculars,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Front_Montage_RH"))
	);

	RagdollMontageFront.Add(
		OverlayState::HandsTied,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Front_Montage_2H"))
	);

	RagdollMontageFront.Add(
		OverlayState::Box,
		LoadObject<UAnimMontage>(nullptr, TEXT(
			                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Front_Montage_2H"))
	);
}

////////////////////////////////////////////////////////////////////////////////////
// Actions

void AActionSetup::AdvanceMove(const FInputActionValue& Value)
{
	if (CurrentMovementState != MovementState::Grounded && CurrentMovementState != MovementState::In_Air) return;

	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AActionSetup::AdvanceLook(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AActionSetup::SetState(OverlayState stance)
{
	if (Controller != nullptr)
	{
		if (CurrentOverlayState != stance) CurrentOverlayState = stance;
	}
}

void AActionSetup::AdvanceJump(double ForwardDesiredLocation, double RightDesiredLocation)
{
	if (Controller == nullptr) return;

	if (CurrentMovementAction != MovementAction::None && (CurrentMovementState != MovementState::Grounded ||
		CurrentMovementState != MovementState::In_Air))
		return;

	if (CurrentMovementState == MovementState::In_Air)
	{
		MantleCheck(FallingTraceSettings, ForwardDesiredLocation, RightDesiredLocation);
		return;
	}

	if (HasMovementInput && MantleCheck(GroundedTraceSettings, ForwardDesiredLocation, RightDesiredLocation))
		return;

	if (CurrentStance == Stance::Crouching)
	{
		CurrentStance = Stance::Standing;
		UnCrouch();
		return;
	}

	Jump();
}

void AActionSetup::AdvanceRagdoll()
{
	if (CurrentMovementState == MovementState::Ragdoll) // Ragdoll End
	{
		AnimInstance->SavePoseSnapshot("RagdollPose");

		if (RagdollOnGround)
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Walking);

			if (RagdollFaceUp)
			{
				AnimInstance->Montage_Play(RagdollMontageBack[CurrentOverlayState]);
			}
			else
			{
				AnimInstance->Montage_Play(RagdollMontageFront[CurrentOverlayState]);
			}
		}
		else
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Falling);
			GetCharacterMovement()->Velocity = LastRagdollVelocity;
		}

		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetCollisionObjectType(ECC_Pawn);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		GetMesh()->SetAllBodiesSimulatePhysics(false);

		return;
	}

	// Ragdoll Start
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	CurrentMovementState = MovementState::Ragdoll;

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetAllBodiesBelowSimulatePhysics("pelvis", true, true);

	AnimInstance->Montage_Stop(0.2);
}


void AActionSetup::AdvanceSprintStart()
{
	desiredGait = Gait::Sprinting;
}

void AActionSetup::AdvanceSprintEnd()
{
	desiredGait = Gait::Running;
}

void AActionSetup::AdvanceCrouch()
{
	if (CurrentMovementAction != MovementAction::None) return;

	if (Controller == nullptr) return;

	if (CurrentMovementState != MovementState::Grounded && CurrentMovementState != MovementState::In_Air) return;

	if (CurrentStance == Stance::Crouching)
	{
		CurrentStance = Stance::Standing;
		UnCrouch();
		return;
	}

	CurrentStance = Stance::Crouching;
	Crouch();
}

void AActionSetup::AdvanceRoll()
{
	if (CurrentMovementAction != MovementAction::None) return;

	if (Controller == nullptr) return;

	if (CurrentMovementState == MovementState::In_Air)
	{
		BreakFall = true;
		UKismetSystemLibrary::RetriggerableDelay(this, 0.4f, FLatentActionInfo());
		BreakFall = false;
	}

	if (CurrentMovementState != MovementState::Grounded) return;

	if (AnimInstance && RollMontage.Find(CurrentOverlayState) != nullptr)
	{
		AnimInstance->Montage_Play(
			RollMontage[CurrentOverlayState],
			1.15
		);
	}

	if (CurrentStance == Stance::Crouching)
	{
		CurrentStance = Stance::Crouching;
		return;
	}

	CurrentStance = Stance::Standing;
}

void AActionSetup::AdvanceAimStart()
{
    CurrentRotationMode = RotationMode::Aiming;
}

void AActionSetup::AdvanceAimEnd()
{
    CurrentRotationMode = RotationMode::LookingDirection;
}

////////////////////////////////////////////////////////////////////////////////////
// Events

void AActionSetup::Jump()
{
	Super::Jump();

	InAirRotation = Speed > 100 ? LastVelocityRotation : GetActorRotation();
	if (AnimInstance && AnimInstance->Implements<UAnimation_INTF>())
	{
		IAnimation_INTF::Execute_INTF_Jumped(AnimInstance); // Communicate with BP
	}
}

void AActionSetup::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if (BreakFall)
	{
		if (AnimInstance && RollMontage.Find(CurrentOverlayState) != nullptr)
		{
			AnimInstance->Montage_Play(
				RollMontage[CurrentOverlayState],
				1.35
			);
		}

		return;
	}

	if (HasMovementInput) GetCharacterMovement()->BrakingFriction = 0.5;
	else GetCharacterMovement()->BrakingFriction = 3;

	DELAY(0.5);

	GetCharacterMovement()->BrakingFriction = 0;
}

void AActionSetup::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	switch (GetCharacterMovement()->MovementMode)
	{
	case MOVE_Falling:
		CurrentMovementState = MovementState::In_Air;
		break;
	case MOVE_Walking:
		CurrentMovementState = MovementState::Grounded;
		break;
	case MOVE_NavWalking:
		CurrentMovementState = MovementState::Grounded;
		break;
	}
}

void AActionSetup::BeginPlay()
{
	Super::BeginPlay();

	MovementModel.DataTable = LoadObject<UDataTable>(
		nullptr, TEXT("/Game/CharacterAnimSetup/Table/MovementModelTable"));

	if (MovementModel.DataTable)
	{
		MovementModel.RowName = FName("Normal");

		MovementData = MovementModel.GetRow<FMovementSettings_State>("");
	}

	if (CurveFloat)
	{
		MantleTimeline->AddInterpFloat(CurveFloat, MantleProgress);
		MantleTimeline->SetTimelineFinishedFunc(MantleFinish);
	}

	AnimInstance = GetMesh()->GetAnimInstance();

	TargetRotation = LastVelocityRotation = LastMovementInputRotation = GetActorRotation();
}

void AActionSetup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	EssentialValuesSetup();

	switch (CurrentMovementState)
	{
	case MovementState::Grounded:
		UpdateCharacterMovement();
		UpdateGroundRotation();
		break;

	case MovementState::In_Air:
		UpdateAirRotation();
		if (HasMovementInput) MantleCheck(FallingTraceSettings, 0, 0);
		break;

	case MovementState::Ragdoll:
		UpdateRagdoll();
		break;
	}

	// Cache values Setup
	PreviousVelocity = GetVelocity();
	PreviousAimYaw = GetControlRotation().Yaw;
}

////////////////////////////////////////////////////////////////////////////////////
// Interface

void AActionSetup::INTF_Set_MovementState_Implementation(MovementState NewMovementState)
{
	NewMovementState = CurrentMovementState;
}

void AActionSetup::INTF_Get_EssentialValues_Implementation(
	FVector& OutVelocity, FVector& OutAcceleration,
	FVector& OutMovementInput, bool& OutIsMoving,
	bool& OutHasMovementInput, double& OutSpeed,
	double& OutMovementInputAmount, FRotator& OutAimingRotation,
	double& OutAimYawRate
)
{
	OutVelocity = GetVelocity();
	OutAcceleration = Acceleration;
	OutMovementInput = GetCharacterMovement()->GetCurrentAcceleration();
	OutIsMoving = IsMoving;
	OutHasMovementInput = HasMovementInput;
	OutSpeed = Speed;
	OutMovementInputAmount = MovementInputAmount;
	OutAimingRotation = GetControlRotation();
	OutAimYawRate = AimYawRate;
}

void AActionSetup::INTF_Get_CurrentStates_Implementation(
	TEnumAsByte<EMovementMode>& OutPawnMovementMode,
	MovementState& OutMovementState, MovementState& OutPrevMovementState,
	MovementAction& OutMovementAction, RotationMode& OutRotationMode,
	Gait& OutActualGait, Stance& OutActualStance, ViewMode& OutViewMode,
	OverlayState& OutOverlayState
)
{
	OutPawnMovementMode = GetCharacterMovement()->MovementMode;
	OutMovementState = CurrentMovementState;
	OutPrevMovementState = PrevMovementState;
	OutMovementAction = CurrentMovementAction;
	OutRotationMode = CurrentRotationMode;
	OutActualGait = CurrentActualGait;
	OutActualStance = CurrentStance;
	OutViewMode = CurrentViewMode;
	OutOverlayState = CurrentOverlayState;
}

////////////////////////////////////////////////////////////////////////////////////
// Components

bool AActionSetup::MantleCheck(FMantle_TraceSettings traceSettings, double ForwardDesiredLocation,
                               double RightDesiredLocation, EDrawDebugTrace::Type debug)
{
	///////////////////////////////////////////////////////////////////////
	/// Find non-walkable object

	FVector capsuleBaseLocation = GetCapsuleComponent()->GetComponentLocation() - (GetCapsuleComponent()->GetUpVector()
		* (
			GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2));

	// find out which way is forward
	const FRotator rotation = Controller->GetControlRotation();
	const FRotator yawRotation(0, rotation.Yaw, 0);

	FHitResult Hit; // Current hit object from trace
	ECollisionChannel collissionChannel = ECC_GameTraceChannel1; // Custom channel of collission

	// get forward vector
	FVector forwardDirection = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
	// get right vector 
	FVector rightDirection = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);

	if (traceSettings.HasDesiredLocation)
	{
		forwardDirection *= ForwardDesiredLocation;

		rightDirection *= RightDesiredLocation;
	}

	FVector movementInput = forwardDirection + rightDirection;
	movementInput.Normalize();
	// FVector movementInput = VectorNormalize(GetActorForwardVector()); // -> If (forward + right) doesnt work

	FVector startTrace = capsuleBaseLocation + (movementInput * -30);
	startTrace.Z += (traceSettings.MaxLedgeHeight + traceSettings.MinLedgeHeight) / 2;

	FVector endTrace = startTrace + (movementInput * traceSettings.ReachDistance);

	float halfHeight = traceSettings.MaxLedgeHeight - traceSettings.MinLedgeHeight;

	halfHeight = (halfHeight / 2) + 1;

	UKismetSystemLibrary::CapsuleTraceSingle(
		this,
		startTrace,
		endTrace,
		traceSettings.ForwardTraceRadius,
		halfHeight,
		UEngineTypes::ConvertToTraceType(collissionChannel),
		false,
		TArray<AActor*>(),
		debug,
		Hit,
		true
	);

	if (GetCharacterMovement()->IsWalkable(Hit) || Hit.bStartPenetrating || !Hit.bBlockingHit) return false;

	FVector initialTraceImpactPoint = Hit.ImpactPoint;
	FVector initialTraceImpactNormal = Hit.ImpactNormal;

	///////////////////////////////////////////////////////////////////////
	/// Find walkable surface and impact point

	endTrace = FVector(initialTraceImpactPoint.X, initialTraceImpactPoint.Y, capsuleBaseLocation.Z) +
		initialTraceImpactNormal * -15;
	startTrace = endTrace + FVector(0, 0, traceSettings.MaxLedgeHeight + traceSettings.DownwardTraceRadius + 1);

	UKismetSystemLibrary::SphereTraceSingle(
		this,
		startTrace,
		endTrace,
		traceSettings.DownwardTraceRadius,
		UEngineTypes::ConvertToTraceType(collissionChannel),
		false,
		TArray<AActor*>(),
		debug,
		Hit,
		true,
		FLinearColor::FromSRGBColor(FColor::Purple),
		FLinearColor::FromSRGBColor(FColor::Cyan)
	);

	if (!GetCharacterMovement()->IsWalkable(Hit) || !Hit.bBlockingHit) return false;

	FVector DownTraceLocation = FVector(Hit.Location.X, Hit.Location.Y, Hit.ImpactPoint.Z);

	UPrimitiveComponent* hitComponent = Hit.GetComponent();

	///////////////////////////////////////////////////////////////////////
	/// Find if capsule has room and calculate mantle height

	FVector capsuleLocationFromBase = FVector(
		DownTraceLocation.X,
		DownTraceLocation.Y,
		GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2 + DownTraceLocation.Z
	);

	startTrace = FVector(
		capsuleLocationFromBase.X,
		capsuleLocationFromBase.Y,
		GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere() + capsuleLocationFromBase.Z
	);
	endTrace = FVector(
		capsuleLocationFromBase.X,
		capsuleLocationFromBase.Y,
		capsuleLocationFromBase.Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere()
	);

	UKismetSystemLibrary::SphereTraceSingle(
		this,
		startTrace,
		endTrace,
		GetCapsuleComponent()->GetScaledCapsuleRadius(),
		UEngineTypes::ConvertToTraceType(collissionChannel),
		false,
		TArray<AActor*>(),
		debug,
		Hit,
		true,
		FLinearColor::Yellow,
		FLinearColor::Blue
	);

	if (Hit.bBlockingHit || Hit.bStartPenetrating) return false; // No room

	FTransform targetTransform(
		FVector(initialTraceImpactNormal * FVector(-1, -1, 0)).Rotation(),
		capsuleLocationFromBase,
		FVector(1, 1, 1)
	);

	float mantleHeight = targetTransform.GetLocation().Z - GetActorLocation().Z;

	if (CurrentMovementState == MovementState::In_Air) CurrentMantleType = MantleType::FallingCatch;
	else if (mantleHeight > 120) CurrentMantleType = MantleType::HighMantle;
	else CurrentMantleType = MantleType::LowMantle;

	MantleStart(mantleHeight, FComponentAndTransform(targetTransform, hitComponent), CurrentMantleType);

	return true;
}

void AActionSetup::MantleStart(float mantleHeight, FComponentAndTransform mantleLedgeWS, MantleType mantleType)
{
	FMantleAsset mantleAsset;

	switch (mantleType)
	{
	case MantleType::HighMantle:
		mantleAsset = FMantleAsset(
			LoadObject<UAnimMontage>(nullptr, TEXT(
				                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_Mantle_2m_Montage_Default")),
			LoadObject<UCurveVector>(nullptr, TEXT("/Game/CharacterAnimSetup/Data/Curves/MantleCurves/Mantle_2m")),
			FVector(0, 65, 200),
			125,
			1.2,
			0.6,
			200,
			1.2,
			0
		);
		break;

	case MantleType::LowMantle:
		mantleAsset = FMantleAsset(
			MantleMontage[CurrentOverlayState],
			LoadObject<UCurveVector>(nullptr, TEXT("/Game/CharacterAnimSetup/Data/Curves/MantleCurves/Mantle_1m")),
			FVector(0, 65, 100),
			50,
			1.0,
			0.5,
			100,
			1.0,
			0
		);
		break;

	case MantleType::FallingCatch:
		mantleAsset = FMantleAsset(
			LoadObject<UAnimMontage>(nullptr, TEXT(
				                         "/Game/CharacterAnimSetup/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_N_Mantle_2m_Montage_Default")),
			LoadObject<UCurveVector>(nullptr, TEXT("/Game/CharacterAnimSetup/Data/Curves/MantleCurves/Mantle_2m")),
			FVector(0, 65, 200),
			125,
			1.2,
			0.6,
			200,
			1.2,
			0
		);
		break;
	}

	MantleParams = FMantleParams(
		mantleAsset.AnimMontage,
		mantleAsset.PositionAndCorrectiveCurve,
		FMath::GetMappedRangeValueClamped(
			FVector2d(mantleAsset.LowHeight, mantleAsset.HighHeight),
			FVector2d(mantleAsset.LowStartPosition, mantleAsset.HighStartPosition),
			mantleHeight
		),
		FMath::GetMappedRangeValueClamped(
			FVector2d(mantleAsset.LowHeight, mantleAsset.HighHeight),
			FVector2d(mantleAsset.LowPlayRate, mantleAsset.HighPlayRate),
			mantleHeight
		),
		mantleAsset.StartingOffset
	);

	MantleLedgeLs = FComponentAndTransform(
		mantleLedgeWS.Transform * mantleLedgeWS.Component->GetComponentTransform().Inverse(),
		mantleLedgeWS.Component
	);

	MantleTarget = mantleLedgeWS.Transform;

	ActualStartOffset = FTransform(
		GetActorTransform().GetRotation() - MantleTarget.GetRotation(),
		GetActorTransform().GetTranslation() - MantleTarget.GetTranslation(),
		GetActorTransform().GetScale3D() - MantleTarget.GetScale3D()
	);

	FVector currOffset = MantleTarget.GetRotation().GetForwardVector() * MantleParams.StartingOffset.Y;

	AnimatedStartOffset = FTransform(
		FRotator(0, 0, 0),
		-FVector(currOffset.X, currOffset.Y, MantleParams.StartingOffset.Z),
		FVector(1, 1, 1) - MantleTarget.GetScale3D()
	);

	GetCharacterMovement()->SetMovementMode(MOVE_None);

	CurrentMovementState = MovementState::Mantling;

	float maxTime, minTime;
	MantleParams.PositionAndCorrectiveCurve->GetTimeRange(minTime, maxTime);

	MantleTimeline->SetTimelineLength(maxTime - MantleParams.StartingPosition);

	MantleTimeline->SetPlayRate(MantleParams.PlayRate);

	MantleTimeline->PlayFromStart();

	if (MantleParams.AnimMontage)
	{
		AnimInstance->Montage_Play(
			MantleParams.AnimMontage,
			MantleParams.PlayRate,
			EMontagePlayReturnType::MontageLength,
			MantleParams.StartingPosition,
			false
		);
	}
}

void AActionSetup::UpdateCharacterMovement()
{
	Gait AllowedGait = Gait::Running;

	///////////////////////////////////////////////////////////////////////
	/// Check if gait can be used

	if (CurrentStance == Stance::Crouching || CurrentRotationMode == RotationMode::Aiming)
	{
		if (desiredGait == Gait::Walking) AllowedGait = Gait::Walking;
		else AllowedGait = Gait::Running;
	}
	else
	{
		switch (desiredGait)
		{
		case Gait::Running:
			AllowedGait = Gait::Running;
			break;
		case Gait::Walking:
			AllowedGait = Gait::Walking;
			break;
		case Gait::Sprinting:
			if (HasMovementInput || CurrentRotationMode != RotationMode::Aiming) // Sprint Check
			{
				if (CurrentRotationMode == RotationMode::VelocityDirection && MovementInputAmount > 0.9)
					AllowedGait = Gait::Sprinting;
				else if (CurrentRotationMode == RotationMode::LookingDirection)
				{
					int CalcLookRotation = abs(UKismetMathLibrary::NormalizedDeltaRotator(
							UKismetMathLibrary::MakeRotFromX(GetCharacterMovement()->GetCurrentAcceleration()),
							GetControlRotation()).Yaw
					);

					if (MovementInputAmount > 0.9 && CalcLookRotation < 50)
						AllowedGait = Gait::Sprinting;
					else AllowedGait = Gait::Running;
				}
				else AllowedGait = Gait::Running;
			}
			else AllowedGait = Gait::Running;
			break;
		}
	}

	if (Speed >= CurrentMovementSettings.RunSpeed + 10)
		if (AllowedGait == Gait::Sprinting) CurrentActualGait = Gait::Sprinting;
		else CurrentActualGait = Gait::Running;
	else if (Speed >= CurrentMovementSettings.WalkSpeed + 10)
		CurrentActualGait = Gait::Running;
	else CurrentActualGait = Gait::Walking;

	///////////////////////////////////////////////////////////////////////
	/// Setup Movement Settings

	if (MovementModel.IsNull())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Empty Movement Table");
		return;
	}

	if (!MovementData)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Empty Movement Data");
		return;
	}

	switch (CurrentRotationMode)
	{
	case RotationMode::VelocityDirection:
		if (CurrentStance == Stance::Standing)
		{
			CurrentMovementSettings = MovementData->VelocityDirection.Standing;
			break;
		}
		CurrentMovementSettings = MovementData->VelocityDirection.Crouching;
		break;

	case RotationMode::LookingDirection:
		if (CurrentStance == Stance::Standing)
		{
			CurrentMovementSettings = MovementData->LookingDirection.Standing;
			break;
		}
		CurrentMovementSettings = MovementData->LookingDirection.Crouching;
		break;

	case RotationMode::Aiming:
		if (CurrentStance == Stance::Standing)
		{
			CurrentMovementSettings = MovementData->Aiming.Standing;
			break;
		}
		CurrentMovementSettings = MovementData->Aiming.Crouching;
		break;
	}

	switch (AllowedGait)
	{
	case Gait::Walking:
		GetCharacterMovement()->MaxWalkSpeed = CurrentMovementSettings.WalkSpeed;
		GetCharacterMovement()->MaxWalkSpeedCrouched = CurrentMovementSettings.WalkSpeed;
		break;

	case Gait::Running:
		GetCharacterMovement()->MaxWalkSpeed = CurrentMovementSettings.RunSpeed;
		GetCharacterMovement()->MaxWalkSpeedCrouched = CurrentMovementSettings.RunSpeed;
		break;

	case Gait::Sprinting:
		GetCharacterMovement()->MaxWalkSpeed = CurrentMovementSettings.SprintSpeed;
		GetCharacterMovement()->MaxWalkSpeedCrouched = CurrentMovementSettings.SprintSpeed;
		break;
	}

	if (!CurrentMovementSettings.MovementCurve)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Empty Movement Curve");
		return;
	}

	FVector MovementVal = CurrentMovementSettings.MovementCurve->GetVectorValue(MappedSeed());
	GetCharacterMovement()->MaxAcceleration = MovementVal.X;
	GetCharacterMovement()->BrakingDecelerationWalking = MovementVal.Y;
	GetCharacterMovement()->GroundFriction = MovementVal.Z;
}

void AActionSetup::EssentialValuesSetup()
{
	Acceleration = (GetVelocity() - PreviousVelocity) / GetWorld()->GetDeltaSeconds();

	Speed = FVector(GetVelocity().X, GetVelocity().Y, 0).Size();

	IsMoving = Speed > 1;

	if (IsMoving)
	{
		LastVelocityRotation = UKismetMathLibrary::MakeRotFromX(GetVelocity());
	}

	MovementInputAmount = GetCharacterMovement()->GetCurrentAcceleration().Length() / GetCharacterMovement()->
		GetMaxAcceleration();

	HasMovementInput = MovementInputAmount > 0.f;

	if (HasMovementInput)
	{
		LastMovementInputRotation = UKismetMathLibrary::MakeRotFromX(GetCharacterMovement()->GetCurrentAcceleration());
	}

	AimYawRate = UKismetMathLibrary::Abs((GetControlRotation().Yaw - PreviousAimYaw) / GetWorld()->GetDeltaSeconds());
}

void AActionSetup::UpdateGroundRotation()
{
	if (CurrentMovementAction != MovementAction::None && CurrentMovementAction != MovementAction::Rolling) return;

	if (CurrentMovementAction == MovementAction::Rolling)
	{
		if (HasMovementInput)
		{
			SmoothCharacterRotation(FRotator(0, 0, LastMovementInputRotation.Yaw), 0, 2);
		}
		return;
	}

	if (((IsMoving && HasMovementInput) || Speed > 150) && !HasAnyRootMotion())
	{
		float GroundedRotationRate = CurrentMovementSettings.RotationRateCurve->GetFloatValue(MappedSeed());
		GroundedRotationRate *= UKismetMathLibrary::MapRangeClamped(AimYawRate, 0, 300, 1, 3);

		switch (CurrentRotationMode)
		{
		case RotationMode::VelocityDirection:
			SmoothCharacterRotation(FRotator(0, 0, LastVelocityRotation.Yaw), 500, GroundedRotationRate);
			break;

		case RotationMode::LookingDirection:
			if (CurrentActualGait == Gait::Sprinting)
			{
				SmoothCharacterRotation(FRotator(0, 0, LastVelocityRotation.Yaw), 500, GroundedRotationRate);
				break;
			}

			SmoothCharacterRotation(
				FRotator(0, 0, GetControlRotation().Yaw + (AnimInstance
					                                           ? AnimInstance->GetCurveValue("YawOffset")
					                                           : 0)),
				500,
				GroundedRotationRate
			);
			break;

		case RotationMode::Aiming:
			SmoothCharacterRotation(FRotator(0, 0, GetControlRotation().Yaw), 1000, 20);
			break;
		}

		return;
	}

	if (CurrentViewMode == ViewMode::FirstPerson || CurrentRotationMode == RotationMode::Aiming)
	{
		float DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(GetControlRotation(), GetActorRotation()).Yaw;
		if (!UKismetMathLibrary::InRange_FloatFloat(DeltaRotator, -100.f, 100.f, true, true))
		{
			SmoothCharacterRotation(
				FRotator(0, 0, DeltaRotator > 0 ? GetControlRotation().Yaw - 100 : GetControlRotation().Yaw + 100),
				0,
				20
			);
		}
	}

	const float AnimValue = AnimInstance ? AnimInstance->GetCurveValue("YawOffset") : 0;
	if (abs(AnimValue) <= 0.001) return;

	AddActorWorldRotation(FRotator(0, 0, AnimValue * (GetWorld()->GetDeltaSeconds() / (1 / 30))));

	TargetRotation = GetActorRotation();
}

void AActionSetup::UpdateAirRotation()
{
	if (CurrentRotationMode == RotationMode::Aiming)
	{
		SmoothCharacterRotation(FRotator(0, 0, GetControlRotation().Yaw), 0, 15);
		InAirRotation = GetActorRotation();
		return;
	}

	SmoothCharacterRotation(FRotator(0, 0, InAirRotation.Yaw), 0, 5);
}

void AActionSetup::UpdateRagdoll(EDrawDebugTrace::Type debug)
{
	LastRagdollVelocity = GetMesh()->GetPhysicsLinearVelocity("root");

	GetMesh()->SetAllMotorsAngularDriveParams(
		UKismetMathLibrary::MapRangeClamped(
			LastRagdollVelocity.Length(),
			0,
			1000,
			0,
			25000
		),
		0,
		0,
		false
	);

	GetMesh()->SetEnableGravity(LastRagdollVelocity.Z > -4000);

	TargetRagdollLocation = GetMesh()->GetSocketLocation("pelvis");

	float PelvisRotation = GetMesh()->GetSocketRotation("pelvis").Yaw;
	
	RagdollFaceUp = GetMesh()->GetSocketRotation("pelvis").Roll < 0;

	FRotator TargetRagdollRotation = FRotator(
		0,
		0,
		RagdollFaceUp ? PelvisRotation - 180 : PelvisRotation
	);
	
	FHitResult Hit;

	UKismetSystemLibrary::LineTraceSingle(
		this,
		TargetRagdollLocation,
		FVector(TargetRagdollLocation.X, GetActorLocation().X,
		        TargetRagdollLocation.Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()),
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		TArray<AActor*>(),
		debug,
		Hit,
		true
	);

	RagdollOnGround = Hit.bBlockingHit;

	if (!RagdollOnGround)
	{
		SetActorLocationAndRotation(TargetRagdollLocation, TargetRagdollRotation);
		return;
	}

	SetActorLocationAndRotation(
		FVector(
			TargetRagdollLocation.X,
			TargetRagdollLocation.Y,
			TargetRagdollLocation.Z + 2 + (
				GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - abs(Hit.ImpactPoint.Z - Hit.TraceStart.Z)
			)
		),
		TargetRagdollRotation
	);
}

void AActionSetup::SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed)
{
	TargetRotation = UKismetMathLibrary::RInterpTo_Constant(
		TargetRotation,
		Target,
		GetWorld()->GetDeltaSeconds(),
		TargetInterpSpeed
	);

	SetActorRotation(UKismetMathLibrary::RInterpTo(
		GetActorRotation(),
		UKismetMathLibrary::RInterpTo_Constant(
			GetActorRotation(),
			TargetRotation,
			GetWorld()->GetDeltaSeconds(),
			ActorInterpSpeed
		),
		GetWorld()->GetDeltaSeconds(),
		ActorInterpSpeed
	));
}

float AActionSetup::MappedSeed()
{
	if (Speed > CurrentMovementSettings.RunSpeed)
		return UKismetMathLibrary::MapRangeClamped(
			Speed,
			CurrentMovementSettings.RunSpeed,
			CurrentMovementSettings.SprintSpeed,
			2,
			3
		);

	if (Speed > CurrentMovementSettings.WalkSpeed)
		return UKismetMathLibrary::MapRangeClamped(
			Speed,
			0,
			CurrentMovementSettings.WalkSpeed,
			0,
			1
		);

	return UKismetMathLibrary::MapRangeClamped(
		Speed,
		CurrentMovementSettings.WalkSpeed,
		CurrentMovementSettings.RunSpeed,
		1,
		2
	);
}


////////////////////////////////////////////////////////////////////////////////////
// Timelines

void AActionSetup::MantleUpdate(float BlendIn)
{
	MantleTarget = MantleLedgeLs.Transform * MantleLedgeLs.Component->GetComponentTransform();

	float PositionAlpha = MantleParams.PositionAndCorrectiveCurve->GetVectorValue(
		MantleTimeline->GetPlaybackPosition() + MantleParams.StartingPosition
	).X;

	float XYCorrectionAlpha = MantleParams.PositionAndCorrectiveCurve->GetVectorValue(
		MantleTimeline->GetPlaybackPosition() + MantleParams.StartingPosition
	).Y;

	float ZCorrectionAlpha = MantleParams.PositionAndCorrectiveCurve->GetVectorValue(
		MantleTimeline->GetPlaybackPosition() + MantleParams.StartingPosition
	).Y;

	FTransform hBlend = UKismetMathLibrary::TLerp(
		ActualStartOffset,
		FTransform(
			AnimatedStartOffset.GetRotation(),
			FVector(
				AnimatedStartOffset.GetLocation().X,
				AnimatedStartOffset.GetLocation().Y,
				ActualStartOffset.GetLocation().Z
			),
			FVector(1, 1, 1)
		),
		XYCorrectionAlpha
	);

	FTransform vBlend = UKismetMathLibrary::TLerp(
		ActualStartOffset,
		FTransform(
			AnimatedStartOffset.GetRotation(),
			FVector(
				AnimatedStartOffset.GetLocation().X,
				AnimatedStartOffset.GetLocation().Y,
				ActualStartOffset.GetLocation().Z
			),
			FVector(1, 1, 1)
		),
		ZCorrectionAlpha
	);

	FTransform LerpedTarget = FTransform(
		hBlend.GetRotation(),
		FVector(
			hBlend.GetLocation().X,
			hBlend.GetLocation().Y,
			vBlend.GetLocation().Z
		),
		FVector(1, 1, 1)
	);

	LerpedTarget += MantleTarget;

	LerpedTarget = UKismetMathLibrary::TLerp(
		LerpedTarget,
		MantleTarget,
		PositionAlpha
	);

	LerpedTarget = UKismetMathLibrary::TLerp(
		MantleTarget + ActualStartOffset,
		LerpedTarget,
		BlendIn
	);

	SetActorLocationAndRotation(LerpedTarget.GetLocation(), LerpedTarget.GetRotation());
}

void AActionSetup::MantleEnd()
{
	if (MantleTimeline->GetPlaybackPosition())
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}
