// Fill out your copyright notice in the Description page of Project Settings.


#include "UserCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void AUserCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////
// Input

void AUserCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AUserCharacter::AdvanceJump, 0.0, 0.0);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUserCharacter::AdvanceMove);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUserCharacter::AdvanceLook);

		// Crouch
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AUserCharacter::AdvanceCrouch);

		// Roll
		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Started, this, &AUserCharacter::AdvanceRoll);

		// Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AUserCharacter::AdvanceSprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AUserCharacter::AdvanceSprintEnd);
		
		// Ragdoll
		EnhancedInputComponent->BindAction(RagdollAction, ETriggerEvent::Started, this, &AUserCharacter::AdvanceRagdoll);

		///////////////////////////////////////////////////
		/// Stance Setup
        
		// Default Stance
		EnhancedInputComponent->BindAction(DefaultAction, ETriggerEvent::Triggered, this, &AUserCharacter::SetState,
		                                   OverlayState::Default);

		// Masculine Stance
		EnhancedInputComponent->BindAction(MasculineAction, ETriggerEvent::Triggered, this, &AUserCharacter::SetState,
		                                   OverlayState::Masculine);

		// Feminine Stance
		EnhancedInputComponent->BindAction(FeminineAction, ETriggerEvent::Triggered, this, &AUserCharacter::SetState,
		                                   OverlayState::Feminine);

		// Injured Stance
		EnhancedInputComponent->BindAction(InjuredAction, ETriggerEvent::Triggered, this, &AUserCharacter::SetState,
		                                   OverlayState::Injured);

		// HandsTied Stance
		EnhancedInputComponent->BindAction(HandsTiedAction, ETriggerEvent::Triggered, this, &AUserCharacter::SetState,
		                                   OverlayState::HandsTied);

		// Rifle Stance
		EnhancedInputComponent->BindAction(RifleAction, ETriggerEvent::Triggered, this, &AUserCharacter::SetState,
		                                   OverlayState::Rifle);

		// Pistol_1H Stance
		EnhancedInputComponent->BindAction(Pistol_1HAction, ETriggerEvent::Triggered, this, &AUserCharacter::SetState,
		                                   OverlayState::Pistol_1H);

		// Pistol_2H Stance
		EnhancedInputComponent->BindAction(Pistol_2HAction, ETriggerEvent::Triggered, this, &AUserCharacter::SetState,
		                                   OverlayState::Pistol_2H);

		// Bow Stance
		EnhancedInputComponent->BindAction(BowAction, ETriggerEvent::Triggered, this, &AUserCharacter::SetState,
		                                   OverlayState::Bow);

		// Torch Stance
		EnhancedInputComponent->BindAction(TorchAction, ETriggerEvent::Triggered, this, &AUserCharacter::SetState,
		                                   OverlayState::Torch);

		// Binoculars Stance
		EnhancedInputComponent->BindAction(BinocularsAction, ETriggerEvent::Triggered, this, &AUserCharacter::SetState,
		                                   OverlayState::Binoculars);

		// Box Stance
		EnhancedInputComponent->BindAction(BoxAction, ETriggerEvent::Triggered, this, &AUserCharacter::SetState,
		                                   OverlayState::Box);

		// Barrel Stance
		EnhancedInputComponent->BindAction(BarrelAction, ETriggerEvent::Triggered, this, &AUserCharacter::SetState,
		                                   OverlayState::Barrel);
		
		// Aim
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AUserCharacter::AdvanceAimStart);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AUserCharacter::AdvanceAimEnd);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

