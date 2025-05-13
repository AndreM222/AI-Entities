// Fill out your copyright notice in the Description page of Project Settings.


#include "AIEntityCharacter.h"

AAIEntityCharacter::AAIEntityCharacter()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AAIEntityCharacter::BeginPlay()
{
	Super::BeginPlay();

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAIEntityCharacter::StaticClass(), PopulationRef);

	GenomeInitialLengthMin = 24;
	GenomeInitialLengthMax = 24;
	GenomeMaxLength = 300;
	StepsPerGeneration = 300;
	MaxNumberNeurons = 40;
	PointMutationRate = 0.001;
	GeneInsertionDeletionRate = 0.0;
	DeletionRatio = 0.5;
	Responsiveness = 0.5;

	DisabledGenes.DisabledActions.Add(EAIActions::KILL_FORWARD);
	DisabledGenes.DisabledActions.Add(EAIActions::TOUCH_FORWARD);

	CharacterStats.Alive = true;
	CharacterStats.location = GetActorLocation();
	CharacterStats.Age = 0;
	CharacterStats.Genome = RandomGenomeGenerator();
	CharacterStats.Responsiveness = 0.5;
	CharacterStats.OscillationPeriod = 34;
	CharacterStats.LongProbesDistance = 16;
	CharacterStats.LastMovementDirection = FAIDIrection(
		FRotator(GetActorRotation()),
		FVector(GetActorLocation())
	);
	CharacterStats.SuccessRate = (unsigned)false;
	CharacterStats.KnownSpaceMin = FVector(100.0, 100.0, 0);
	CharacterStats.KnownSpaceMax = FVector(2980.0, 3400.0, 0);

	WireGenomes();
}

void AAIEntityCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// AdvanceMove(FVector2d(0, 1));
	for (unsigned CurrStep = 0; CurrStep < StepsPerGeneration; ++CurrStep)
	{
		// multithreaded loop: index 0 is reserved, start at 1
		if (CharacterStats.Alive)
		{
			UpdateEntity(CurrStep);
		}
	}
}

bool AAIEntityCharacter::ActionEnabled(EAIActions Action)
{
	return !DisabledGenes.DisabledActions.Contains(Action);
}

void AAIEntityCharacter::ExecuteAction(TMap<EAIActions, float>& ActionLevels)
{
	float Level, ResponsivenessAdjusted = 0;

	/*********************************************************************************************
	* Setup Type
	********************************************************************************************* */

	// Amount it takes to act
	if (ActionEnabled(EAIActions::SET_RESPONSIVENESS))
	{
		Level = ActionLevels[EAIActions::SET_RESPONSIVENESS];
		Level = (tanh(Level) + 1.0f) / 2.0f; // Converts to 0 -> 1 value range

		CharacterStats.Responsiveness = Level;
		ResponsivenessAdjusted = pow(CharacterStats.Responsiveness - 2.0f, -4) - pow(2, -4) * -2;
	}

	// Time to finish period
	if (ActionEnabled(EAIActions::SET_OSCILLATOR_PERIOD))
	{
		Level = ActionLevels[EAIActions::SET_OSCILLATOR_PERIOD];
		Level = (tanh(Level) + 1.0f) / 2.0f;
		Level = 1.5f + exp(7 * Level) + 1;

		CharacterStats.OscillationPeriod = Level;
	}

	// Sight distance to detect stuff
	if (ActionEnabled(EAIActions::SET_SIGHT_DIST))
	{
		float maxDistance = 32;
		Level = ActionLevels[EAIActions::SET_SIGHT_DIST];
		Level = (tanh(Level) + 1.0f) / 2.0f;
		Level = Level * maxDistance + 1;
		CharacterStats.LongProbesDistance = Level;
	}

	// Emit pheromone for procreation
	if (ActionEnabled(EAIActions::EMIT_PHEROMONE))
	{
		float threshold = 0.5;
		Level = ActionLevels[EAIActions::EMIT_PHEROMONE];
		Level = (tanh(Level) + 1.0f) / 2.0f;
		Level *= ResponsivenessAdjusted;

		if (Level > threshold)
		{
			// Increase pheromones on current location
		}
	}

	// Touch stuff in front
	if (ActionEnabled(EAIActions::TOUCH_FORWARD))
	{
		float threshold = 0.5;
		Level = ActionLevels[EAIActions::TOUCH_FORWARD];
		Level = (tanh(Level) + 1.0f) / 2.0f;
		Level *= ResponsivenessAdjusted;

		if (Level > threshold)
		{
			// Action of touch
		}
	}

	// Kill stuff in front
	if (ActionEnabled(EAIActions::KILL_FORWARD))
	{
		float threshold = 0.5;
		Level = ActionLevels[EAIActions::KILL_FORWARD];
		Level = (tanh(Level) + 1.0f) / 2.0f;
		Level *= ResponsivenessAdjusted;

		if (Level > threshold)
		{
			// Action of kill
		}
	}
	
	/*********************************************************************************************
	* Movement Type
	********************************************************************************************* */

	FVector Offset;
	FVector LastMoveOffset = CharacterStats.LastMovementDirection.Location;

	// Urges to move in certain axis
	float MoveX = ActionEnabled(EAIActions::MOVE_X) ? ActionLevels[EAIActions::MOVE_X] : 0;
	float MoveY = ActionEnabled(EAIActions::MOVE_Y) ? ActionLevels[EAIActions::MOVE_Y] : 0;

	if (ActionEnabled(EAIActions::MOVE_EAST)) MoveX += ActionLevels[EAIActions::MOVE_EAST];
	if (ActionEnabled(EAIActions::MOVE_WEST)) MoveX -= ActionLevels[EAIActions::MOVE_WEST];
	if (ActionEnabled(EAIActions::MOVE_NORTH)) MoveY += ActionLevels[EAIActions::MOVE_NORTH];
	if (ActionEnabled(EAIActions::MOVE_SOUTH)) MoveY -= ActionLevels[EAIActions::MOVE_SOUTH];

	if (ActionEnabled(EAIActions::JUMP))
	{
		float threshold = 0.5;
		Level = ActionLevels[EAIActions::JUMP];
		Level = (tanh(Level) + 1.0f) / 2.0f;
		Level *= ResponsivenessAdjusted;

		if (Level > threshold)
		{
            AdvanceJump();
		}
	}

	if (ActionEnabled(EAIActions::MOVE_FORWARD))
	{
		Level = ActionLevels[EAIActions::MOVE_FORWARD];
		MoveX += LastMoveOffset.X * Level;
		MoveY += LastMoveOffset.Y * Level;
	}

	if (ActionEnabled(EAIActions::MOVE_BACKWARD))
	{
		Level = ActionLevels[EAIActions::MOVE_BACKWARD];
		MoveX -= LastMoveOffset.X * Level;
		MoveY -= LastMoveOffset.Y * Level;
	}

	if (ActionEnabled(EAIActions::MOVE_LEFT))
	{
		Level = ActionLevels[EAIActions::MOVE_LEFT];
		Offset = CharacterStats.LastMovementDirection.Rotation.Vector();
		MoveX += Offset.X * Level;
		MoveY += Offset.Y * Level;
	}

	if (ActionEnabled(EAIActions::MOVE_RIGHT))
	{
		Level = ActionLevels[EAIActions::MOVE_RIGHT];
		Offset = CharacterStats.LastMovementDirection.Rotation.Vector();
		MoveX += Offset.X * Level;
		MoveY += Offset.Y * Level;
	}

	if (ActionEnabled(EAIActions::MOVE_RL))
	{
		Level = ActionLevels[EAIActions::MOVE_RL];
		Offset = CharacterStats.LastMovementDirection.Rotation.Vector();
		MoveX += Offset.X * Level;
		MoveY += Offset.Y * Level;
	}

	if (ActionEnabled(EAIActions::MOVE_RANDOM))
	{
		Level = ActionLevels[EAIActions::MOVE_RANDOM];

		FRotator RandomRotation = FRotator(
			FMath::RandRange(-180.0f, 180.0f),
			FMath::RandRange(-180.0f, 180.0f),
			FMath::RandRange(-180.0f, 180.0f)
		);
		Offset = RandomRotation.Vector();

		MoveX += Offset.X * Level;
		MoveY += Offset.Y * Level;
	}

	// Set between -1 to 1 to decide movement
	MoveX = tanh(MoveX);
	MoveY = tanh(MoveY);
	MoveX += ResponsivenessAdjusted;
	MoveY += ResponsivenessAdjusted;

	MoveX = MoveX > 0 ? 1 : MoveX < 0 ? -1 : 0;
	MoveY = MoveY > 0 ? 1 : MoveY < 0 ? -1 : 0;

	AdvanceMove(FVector2D(MoveX, MoveY));
}

TMap<EAIActions, float> AAIEntityCharacter::SensorToAction(unsigned CurrStep)
{
	TMap<EAIActions, float> ActionLevels;
	TArray<float> NeuralAccumulators;

	// Initialize values
	for (EAIActions action : TEnumRange<EAIActions>()) ActionLevels.Add(action, 0.0f);

	for (unsigned i = 0; i < CharacterStats.NeuralNet.Neurons.Num(); i++) NeuralAccumulators.Add(0);

	bool NeuronOutputsComputed = false;

	for (FAIGene Connection : CharacterStats.NeuralNet.Connections)
	{
		if (Connection.SinkType == ACTION && !NeuronOutputsComputed)
		{
			for (unsigned i = 0; i < NeuralAccumulators.Num(); i++)
			{
				if (CharacterStats.NeuralNet.Neurons[i].Driven)
					CharacterStats.NeuralNet.Neurons[i].Output = FMath::Tanh(NeuralAccumulators[i]);
			}

			NeuronOutputsComputed = true;
		}

		float InputValue;

		if (Connection.SourceType == SENSOR) InputValue = GetSensor((EAISensory)Connection.SourceNum, CurrStep, EDrawDebugTrace::ForOneFrame);
		else InputValue = CharacterStats.NeuralNet.Neurons[Connection.SourceNum].Output;

		if (Connection.SinkType == ACTION)
			ActionLevels[(EAIActions)Connection.SinkNum] += InputValue * (float)
				Connection.Weight / 8192.0;
		else NeuralAccumulators[Connection.SinkNum] += InputValue * (float)(Connection.Weight / 8192.0);
	}

	return ActionLevels;
}

FHitResult AAIEntityCharacter::DistanceObjectHit(EAIDirections Direction, ECollisionChannel Channel,
                                                 EDrawDebugTrace::Type Debug, FColor TraceColor, FColor TraceHitColor)
{
	FVector StartLocation = GetActorLocation();

	switch (Direction)
	{
	case EAIDirections::NORTH:
		{
			// Measures the distance to nearest boundary in the south-north axis,
			// max distance is half the grid height; scaled to sensor range 0.0..1.0.

			FHitResult HitNorth;

			FVector EndLocationNorth = StartLocation + FVector(0, MaxSensorRange, 0); // North (Y+)

			UKismetSystemLibrary::LineTraceSingle(
				this,
				StartLocation,
				EndLocationNorth,
				UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
				false,
				TArray<AActor*>(),
				Debug,
				HitNorth,
				true,
				FLinearColor::FromSRGBColor(TraceColor),
				FLinearColor::FromSRGBColor(TraceHitColor)
			);

			return HitNorth;
		}

	case EAIDirections::SOUTH:
		{
			// Measures the distance to nearest boundary in the south-north axis,
			// max distance is half the grid height; scaled to sensor range 0.0..1.0.

			FHitResult HitSouth;

			FVector EndLocationSouth = StartLocation + FVector(0, -MaxSensorRange, 0); // South (Y-)

			UKismetSystemLibrary::LineTraceSingle(
				this,
				StartLocation,
				EndLocationSouth,
				UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
				false,
				TArray<AActor*>(),
				Debug,
				HitSouth,
				true,
				FLinearColor::FromSRGBColor(TraceColor),
				FLinearColor::FromSRGBColor(TraceHitColor)
			);

			return HitSouth;
		}

	case EAIDirections::EAST:
		{
			// Measures the distance to nearest boundary in the east-west axis,
			// max distance is half the grid width; scaled to sensor range 0.0..1.0.

			FHitResult HitEast;

			FVector EndLocationEast = GetActorLocation() + FVector(MaxSensorRange, 0, 0); // East (X+)

			UKismetSystemLibrary::LineTraceSingle(
				this,
				StartLocation,
				EndLocationEast,
				UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
				false,
				TArray<AActor*>(),
				Debug,
				HitEast,
				true,
				FLinearColor::FromSRGBColor(TraceColor),
				FLinearColor::FromSRGBColor(TraceHitColor)
			);

			return HitEast;
		}

	case EAIDirections::WEST:
		{
			// Measures the distance to nearest boundary in the east-west axis,
			// max distance is half the grid width; scaled to sensor range 0.0..1.0.

			FHitResult HitWest;

			FVector EndLocationWest = GetActorLocation() + FVector(-MaxSensorRange, 0, 0); // West (X-)

			UKismetSystemLibrary::LineTraceSingle(
				this,
				StartLocation,
				EndLocationWest,
				UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
				false,
				TArray<AActor*>(),
				Debug,
				HitWest,
				true,
				FLinearColor::FromSRGBColor(TraceColor),
				FLinearColor::FromSRGBColor(TraceHitColor)
			);

			return HitWest;
		}

	default:
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Invalid Direction");
		break;
	}
}

float AAIEntityCharacter::GetSensor(EAISensory Sensor, unsigned CurrStep, EDrawDebugTrace::Type Debug)
{
	float SensorValue = 0.0f;

	switch (Sensor)
	{
	case EAISensory::AGE:
		{
			SensorValue = (float)CharacterStats.Age / StepsPerGeneration;
			break;
		}

	case EAISensory::BOUNDARY_DIST:
		{
			// Finds closest boundary, compares that to the max possible dist
			// to a boundary from the center, and converts that linearly to the
			// sensor range 0.0..1.0
			FHitResult HitEast, HitWest, HitNorth, HitSouth;

			HitEast = DistanceObjectHit(EAIDirections::EAST, ECC_WorldStatic, Debug);
			HitWest = DistanceObjectHit(EAIDirections::WEST, ECC_WorldStatic, Debug);

			// Take the closest boundary distance
			float ClosestDistanceX = FMath::Min(
				HitEast.bBlockingHit ? HitEast.Distance : MaxSensorRange,
				HitWest.bBlockingHit ? HitWest.Distance : MaxSensorRange
			);

			HitNorth = DistanceObjectHit(EAIDirections::NORTH, ECC_WorldStatic, Debug);
			HitSouth = DistanceObjectHit(EAIDirections::SOUTH, ECC_WorldStatic, Debug);

			// Take the closest boundary distance
			float ClosestDistanceY = FMath::Min(
				HitNorth.bBlockingHit ? HitNorth.Distance : MaxSensorRange,
				HitSouth.bBlockingHit ? HitSouth.Distance : MaxSensorRange
			);

			// Normalize to 0.0 - 1.0
			SensorValue = FMath::Min(ClosestDistanceY, ClosestDistanceX) / MaxSensorRange;
			break;
		}
	case EAISensory::BOUNDARY_DIST_X:
		{
			// Measures the distance to nearest boundary in the east-west axis,
			// max distance is half the grid width; scaled to sensor range 0.0..1.0.

			FHitResult HitEast, HitWest;

			HitEast = DistanceObjectHit(EAIDirections::EAST, ECC_WorldStatic, Debug);
			HitWest = DistanceObjectHit(EAIDirections::WEST, ECC_WorldStatic, Debug);

			// Take the closest boundary distance
			float ClosestDistanceX = FMath::Min(
				HitEast.bBlockingHit ? HitEast.Distance : MaxSensorRange,
				HitWest.bBlockingHit ? HitWest.Distance : MaxSensorRange
			);

			// Normalize to 0.0 - 1.0
			SensorValue = ClosestDistanceX / MaxSensorRange;
			break;
		}
	case EAISensory::BOUNDARY_DIST_Y:
		{
			// Measures the distance to nearest boundary in the south-north axis,
			// max distance is half the grid height; scaled to sensor range 0.0..1.0.

			FHitResult HitNorth, HitSouth;

			HitNorth = DistanceObjectHit(EAIDirections::NORTH, ECC_WorldStatic, Debug);
			HitSouth = DistanceObjectHit(EAIDirections::SOUTH, ECC_WorldStatic, Debug);

			// Take the closest boundary distance
			float ClosestDistanceY = FMath::Min(
				HitNorth.bBlockingHit ? HitNorth.Distance : MaxSensorRange,
				HitSouth.bBlockingHit ? HitSouth.Distance : MaxSensorRange
			);

			// Normalize to 0.0 - 1.0
			SensorValue = ClosestDistanceY / MaxSensorRange;
			break;
		}
	case EAISensory::LAST_MOVE_DIR_X:
		{
			// X component -1,0,1 maps to sensor values 0.0, 0.5, 1.0

			auto lastX = CharacterStats.LastMovementDirection.Location.X;
			SensorValue = lastX == 0 ? 0.5 : (lastX == -1 ? 0.0 : 1.0);
			break;
		}
	case EAISensory::LAST_MOVE_DIR_Y:
		{
			// Y component -1,0,1 maps to sensor values 0.0, 0.5, 1.0

			auto lastY = CharacterStats.LastMovementDirection.Location.Y;
			SensorValue = lastY == 0 ? 0.5 : (lastY == -1 ? 0.0 : 1.0);
			break;
		}
	case EAISensory::LOC_X:
		{
			// Maps current X location 0..p.sizeX-1 to sensor range 0.0..1.0
			SensorValue = (GetActorLocation().X - CharacterStats.KnownSpaceMin.X) /
				(CharacterStats.KnownSpaceMax.X - CharacterStats.KnownSpaceMin.X);
			break;
		}
	case EAISensory::LOC_Y:
		{
			// Maps current Y location 0..p.sizeY-1 to sensor range 0.0..1.0
			SensorValue = (GetActorLocation().Y - CharacterStats.KnownSpaceMin.Y) /
				(CharacterStats.KnownSpaceMax.Y - CharacterStats.KnownSpaceMin.Y);
			break;
		}
	case EAISensory::OSC1:
		{
			// Maps the oscillator sine wave to sensor range 0.0..1.0;
			// cycles starts at CurrStep 0 for everbody.
			float phase = (CurrStep % CharacterStats.OscillationPeriod) / (float)CharacterStats.OscillationPeriod;
			// 0.0..1.0
			float factor = -FMath::Cos(phase * 2.0f * 3.1415927f);
			factor += 1.0f; // convert to 0.0..2.0
			factor /= 2.0; // convert to 0.0..1.0
			SensorValue = factor;
			// Clip any round-off error
			SensorValue = FMath::Min(1.0f, FMath::Max(0.0f, SensorValue));
			break;
		}
	case EAISensory::LONGPROBE_POP_FWD:
		{
			// Measures the distance to the nearest other individual in the
			// forward direction. If non found, returns the maximum sensor value.
			// Maps the result to the sensor range 0.0..1.0.
			FHitResult Hit;

			UKismetSystemLibrary::LineTraceSingle(
				this,
				GetActorLocation(),
				GetActorLocation() + (GetActorRotation().Vector() * MaxSensorRange),
				UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
				false,
				TArray<AActor*>({this}),
				Debug,
				Hit,
				true
			);

			float distancePopulation;
			if (!Hit.GetActor() || Hit.GetActor()->IsA(AAIEntityCharacter::StaticClass()))
				distancePopulation = MaxSensorRange;
			else distancePopulation = Hit.Distance;

			// Normalize to 0.0 - 1.0
			SensorValue = distancePopulation / MaxSensorRange;
			break;
		}
	case EAISensory::LONGPROBE_BAR_FWD:
		{
			// Measures the distance to the nearest barrier in the forward
			// direction. If non found, returns the maximum sensor value.
			// Maps the result to the sensor range 0.0..1.0.
			FHitResult Hit;

			UKismetSystemLibrary::LineTraceSingle(
				this,
				GetActorLocation(),
				GetActorLocation() + (GetActorRotation().Vector() * MaxSensorRange),
				UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_WorldStatic),
				false,
				TArray<AActor*>({this}),
				Debug,
				Hit,
				true
			);

			float DistanceBarrier = Hit.bBlockingHit ? Hit.Distance : MaxSensorRange;

			// Normalize to 0.0 - 1.0
			SensorValue = DistanceBarrier / MaxSensorRange;
			break;
		}
	case EAISensory::POPULATION_IP:
		{
			// Returns population density in neighborhood converted linearly from
			// 0..100% to sensor range
			unsigned CountPopulation = 0;

			TArray<FHitResult> Hit;

			UKismetSystemLibrary::SphereTraceMulti(
				this,
				GetActorLocation(),
				GetActorLocation(),
				500,
				UEngineTypes::ConvertToTraceType(ECC_Pawn),
				false,
				TArray<AActor*>({this}),
				Debug,
				Hit,
				true
			);

			for (FHitResult HitResult : Hit)
			{
				if (Cast<AAIEntityCharacter>(HitResult.GetActor())) // Change this to your AI class
				{
					CountPopulation++;
				}
			}
			SensorValue = CountPopulation / PopulationRef.Num();
			break;
		}
	case EAISensory::POPULATION_FWD:
		{
			// Sense population density along axis of last movement direction, mapped
			// to sensor range 0.0..1.0
			unsigned CountPopulation = 0;

			TArray<FHitResult> Hit;

			UKismetSystemLibrary::LineTraceMulti(
				this,
				GetActorLocation(),
				GetActorLocation() + (GetActorRotation().Vector() * MaxSensorRange),
				UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_WorldStatic),
				false,
				TArray<AActor*>({this}),
				Debug,
				Hit,
				true
			);

			for (FHitResult HitResult : Hit)
			{
				if (Cast<AAIEntityCharacter>(HitResult.GetActor())) // Change this to your AI class
				{
					CountPopulation++;
				}
			}
			SensorValue = CountPopulation / PopulationRef.Num();
			break;
		}
	case EAISensory::POPULATION_LR:
		{
			// Sense population density along an axis 90 degrees from last movement direction
			unsigned CountPopulation = 0;

			TArray<FHitResult> HitL, HitR;

			UKismetSystemLibrary::LineTraceMulti(
				this,
				GetActorLocation(),
				GetActorLocation() + FRotator(0, -90, 0).RotateVector(GetActorRotation().Vector() * MaxSensorRange),
				UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_WorldStatic),
				false,
				TArray<AActor*>({this}),
				Debug,
				HitL,
				true
			);

			UKismetSystemLibrary::LineTraceMulti(
				this,
				GetActorLocation(),
				GetActorLocation() + FRotator(0, 90, 0).RotateVector(GetActorRotation().Vector() * MaxSensorRange),
				UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_WorldStatic),
				false,
				TArray<AActor*>({this}),
				Debug,
				HitR,
				true
			);

			for (FHitResult HitResult : HitL)
			{
				if (Cast<AAIEntityCharacter>(HitResult.GetActor())) // Change this to your AI class
				{
					CountPopulation++;
				}
			}

			for (FHitResult HitResult : HitR)
			{
				if (Cast<AAIEntityCharacter>(HitResult.GetActor())) // Change this to your AI class
				{
					CountPopulation++;
				}
			}

			SensorValue = CountPopulation / PopulationRef.Num();
			break;
		}
	case EAISensory::BARRIER_FWD:
		{
			// Sense the nearest barrier along axis of last movement direction, mapped
			// to sensor range 0.0..1.               0

			FHitResult Hit;

			FVector EndLocation = GetActorLocation() + (MaxSensorRange * GetActorRotation().Vector());

			UKismetSystemLibrary::LineTraceSingle(
				this,
				GetActorLocation(),
				EndLocation,
				UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
				false,
				TArray<AActor*>({this}),
				Debug,
				Hit,
				true
			);

			float DistanceBarrier = Hit.bBlockingHit ? Hit.Distance : MaxSensorRange;

			SensorValue = DistanceBarrier / MaxSensorRange;
			break;
		}
	case EAISensory::BARRIER_LR:
		{
			// Sense the nearest barrier along axis perpendicular to last movement direction, mapped
			// to sensor range 0.0..1.0
			FHitResult HitR, HitL;

			FVector EndLocationR = GetActorLocation() + FRotator(0, 90, 0).RotateVector(
				MaxSensorRange * GetActorRotation().Vector());
			FVector EndLocationL = GetActorLocation() + FRotator(0, -90, 0).RotateVector(
				MaxSensorRange * GetActorRotation().Vector());

			UKismetSystemLibrary::LineTraceSingle(
				this,
				GetActorLocation(),
				EndLocationR,
				UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
				false,
				TArray<AActor*>({this}),
				Debug,
				HitR,
				true
			);

			UKismetSystemLibrary::LineTraceSingle(
				this,
				GetActorLocation(),
				EndLocationL,
				UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
				false,
				TArray<AActor*>({this}),
				Debug,
				HitL,
				true
			);

			float DistanceBarrier = HitL.bBlockingHit ? HitL.Distance : MaxSensorRange;
			DistanceBarrier = HitR.bBlockingHit ? FMath::Min(HitR.Distance, DistanceBarrier) : DistanceBarrier;

			SensorValue = DistanceBarrier / MaxSensorRange;
			break;
		}
	case EAISensory::RANDOM:
		{
			// Returns a random sensor value in the range 0.0..1.0.
			SensorValue = FMath::RandRange(0, 100) / 100;
			break;
		}
	case EAISensory::PHEROMONE_IP:
		{
			// Returns magnitude of signal0 in the local neighborhood, with
			// 0.0..maxSignalSum converted to sensorRange 0.0..1.0
			SensorValue = 0;
			break;
		}
	case EAISensory::PHEROMONE_FWD:
		{
			// Sense signal0 density along axis of last movement direction
			SensorValue = 0;
			break;
		}
	case EAISensory::PHEROMONE_LR:
		{
			// Sense signal0 density along an axis perpendicular to last movement direction
			SensorValue = 0;
			break;
		}
	case EAISensory::GENETIC_SIM_FWD:
		{
			// Return minimum sensor value if nobody is alive in the forward adjacent location,
			// else returns a similarity match in the sensor range 0.0..1.0

			FHitResult Hit;
			unsigned SimilarCount = 0, TotalComparison = CharacterStats.Genome.Num();

			FVector EndLocation = GetActorLocation() + (MaxSensorRange * GetActorRotation().Vector()); // East (X+)

			UKismetSystemLibrary::LineTraceSingle(
				this,
				GetActorLocation(),
				EndLocation,
				UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
				false,
				TArray<AActor*>({this}),
				Debug,
				Hit,
				true
			);

			if (Cast<AAIEntityCharacter>(Hit.GetActor()) && Cast<AAIEntityCharacter>(Hit.GetActor())->CharacterStats.
				Alive)
			{
				for (int i = 0; i < TotalComparison; i++)
				{
					if (
						CharacterStats.Genome[i].Weight == Cast<AAIEntityCharacter>(Hit.GetActor())->
						                                   CharacterStats.Genome[i].Weight &&
						CharacterStats.Genome[i].SinkNum == Cast<AAIEntityCharacter>(Hit.GetActor())->
						                                    CharacterStats.Genome[i].SinkNum &&
						CharacterStats.Genome[i].SinkType == Cast<AAIEntityCharacter>(Hit.GetActor())->
						                                     CharacterStats.Genome[i].SinkType &&
						CharacterStats.Genome[i].SourceNum == Cast<AAIEntityCharacter>(Hit.GetActor())->
						                                      CharacterStats.Genome[i].SourceNum &&
						CharacterStats.Genome[i].SourceType == Cast<AAIEntityCharacter>(Hit.GetActor())->
						                                       CharacterStats.Genome[i].SourceType
					)
						SimilarCount++;
				}
			}
			else
			{
				SensorValue = 0;
				break;
			}

			SensorValue = SimilarCount / TotalComparison;
			break;
		}
	default:
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Unkown type");
			break;
		}
	}

	return SensorValue;
}

void AAIEntityCharacter::UpdateEntity(unsigned CurrStep)
{
	CharacterStats.Age++;
	TMap<EAIActions, float> ActionLevels = SensorToAction(CurrStep);
	ExecuteAction(ActionLevels);
}

TArray<FAIGene> AAIEntityCharacter::RandomGenomeGenerator()
{
	TArray<FAIGene> Genome;

	unsigned length = FMath::RandRange(GenomeInitialLengthMin, GenomeInitialLengthMax);

	for (unsigned i = 0; i < length; i++)
	{
		Genome.Push(FAIGene(
			FMath::RandRange(0, 1), // 1-bit unsigned
			FMath::RandRange(0, 32767), // 16-bit unsigned
			FMath::RandRange(0, 1), // 1-bit unsigned
			FMath::RandRange(0, 32767), // 16-bit unsigned
			FMath::RandRange(-32768, 32768) // 16-bit signed
		));
	}

	return Genome;
}

void AAIEntityCharacter::WireGenomes()
{
	TArray<FAIGene> ConnectionList;
	TMap<uint16_t, FNeuron> NeuronMap;

	// Setup list of genes on array
	ConnectionList.Empty();
	for (auto const& Gene : CharacterStats.Genome)
	{
		ConnectionList.Push(Gene);
		auto& Connection = ConnectionList.Last();

		if (Connection.SourceType == NEURON) Connection.SourceNum %= MaxNumberNeurons;
		else Connection.SourceNum %= StaticEnum<EAISensory>()->NumEnums() - 1;

		if (Connection.SinkType == NEURON) Connection.SinkNum %= MaxNumberNeurons;
		else Connection.SinkNum %= StaticEnum<EAIActions>()->NumEnums() - 1;
	}

	NeuronMap.Empty();
	for (auto const& Connection : ConnectionList)
	{
		if (Connection.SinkType == NEURON)
		{
			auto curr = NeuronMap.Find(Connection.SinkNum);

			if (curr == nullptr)
			{
				NeuronMap.Add(Connection.SinkNum, {});
				curr = NeuronMap.Find(Connection.SinkNum);
				curr->NumOutputs = 0;
				curr->NumSelfInputs = 0;
				curr->NumInputsFromSensorsOrOtherNeurons = 0;
			}

			if (Connection.SourceType == NEURON && (Connection.SourceNum == Connection.SinkNum))
				(curr->NumSelfInputs)++;
			else
				(curr->NumInputsFromSensorsOrOtherNeurons)++;
		}

		if (Connection.SourceType == NEURON)
		{
			auto curr = NeuronMap.Find(Connection.SourceNum);

			if (curr == nullptr)
			{
				NeuronMap.Add(Connection.SourceNum, {});
				curr = NeuronMap.Find(Connection.SourceNum);
				curr->NumOutputs = 0;
				curr->NumSelfInputs = 0;
				curr->NumInputsFromSensorsOrOtherNeurons = 0;
			}
			(curr->NumOutputs)++;
		}
	}
	// Remove extra neurons
	bool Cleaned = false;
	while (!Cleaned)
	{
		Cleaned = true;

		// Collect keys to remove first to avoid modifying the map while iterating
		TArray<uint16_t> NeuronsToRemove;

		for (const auto& Elem : NeuronMap)
		{
			uint16_t NeuronKey = Elem.Key;
			const FNeuron& Neuron = Elem.Value;

			if (Neuron.NumOutputs == Neuron.NumSelfInputs)
			{
				// Could be 0
				Cleaned = false;
				NeuronsToRemove.Add(NeuronKey);
			}
		}

		// Remove collected neurons
		for (uint16_t NeuronKey : NeuronsToRemove)
		{
			CutNeuron(NeuronKey, ConnectionList, NeuronMap);
			NeuronMap.Remove(NeuronKey);
		}
	}

	CharacterStats.NeuralNet.Connections.Empty();

	// Setup neurons and sensors connection
	for (auto const& Connection : ConnectionList)
	{
		if (Connection.SourceType == NEURON)
		{
			CharacterStats.NeuralNet.Connections.Push(Connection);
			auto& NewNeuron = CharacterStats.NeuralNet.Connections.Last();

			// Setup destination
			if (NeuronMap.Find(NewNeuron.SinkNum))
				NewNeuron.SinkNum = NeuronMap.Find(NewNeuron.SinkNum)->RemappedNumber;
			// Setup source
			if (NewNeuron.SourceType == NEURON)
				NewNeuron.SourceNum = NeuronMap.Find(NewNeuron.SourceNum)->RemappedNumber;
		}
	}

	// Connect sensors and neurons to actions
	for (auto const& Connection : ConnectionList)
	{
		if (Connection.SinkType == ACTION)
		{
			CharacterStats.NeuralNet.Connections.Push(Connection);
			auto& NewNeuron = CharacterStats.NeuralNet.Connections.Last();
			// Setup source
			if (NewNeuron.SourceType == NEURON)
				NewNeuron.SourceNum = NeuronMap.Find(NewNeuron.SourceNum)->RemappedNumber;
		}
	}

	// Create neural node list
	CharacterStats.NeuralNet.Neurons.Empty();
	for (auto CurrentNeuron : NeuronMap)
	{
		CharacterStats.NeuralNet.Neurons.Push({});
		CharacterStats.NeuralNet.Neurons.Last().Output = 0.5;
		CharacterStats.NeuralNet.Neurons.Last().Driven = (CurrentNeuron.Value.NumInputsFromSensorsOrOtherNeurons != 0);
	}
}

void AAIEntityCharacter::CutNeuron(uint16_t NeuronNum, TArray<FAIGene>& Connections, TMap<uint16_t, FNeuron> NeuronMap)
{
	for (int32 i = 0; i < Connections.Num();)
	{
		if (Connections[i].SinkType == NEURON && Connections[i].SinkNum == NeuronNum)
		{
			// Remove the connection. If the connection source is from another
			// neuron, also decrement the other neuron's numOutputs:
			if (Connections[i].SourceType == NEURON)
			{
				if (NeuronMap.Find(Connections[i].SourceNum))
					(NeuronMap.Find(Connections[i].SourceNum)->NumOutputs)--;
			}
			Connections.RemoveAt(i); // Removes the element at index i
		}
		else i++;
	}
}