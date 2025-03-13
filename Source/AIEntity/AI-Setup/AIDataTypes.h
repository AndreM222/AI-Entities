#pragma once

#include "AIDataTypes.generated.h"

UENUM(BlueprintType)
enum class EAIEntityState : uint8
{
	Interested,
	Neutral,
	Wary
};

UENUM()
enum class EAIActions : uint8
{
	MOVE_X,
	MOVE_Y,
	MOVE_FORWARD,
	MOVE_RL,
	MOVE_RANDOM,
	SET_OSCILLATOR_PERIOD,
	SET_SIGHT_DIST,
	SET_RESPONSIVENESS,
	EMIT_PHEROMONE,
	JUMP,
	MOVE_EAST,
	MOVE_WEST,
	MOVE_NORTH,
	MOVE_SOUTH,
	MOVE_LEFT,
	MOVE_RIGHT,
	MOVE_BACKWARD,
	TOUCH_FORWARD,
	KILL_FORWARD
};

ENUM_RANGE_BY_FIRST_AND_LAST(EAIActions, EAIActions::MOVE_X, EAIActions::KILL_FORWARD)

UENUM()
enum class EAISensory : uint8
{
	LOC_X,
	LOC_Y,
	BOUNDARY_DIST_X,
	BOUNDARY_DIST,
	BOUNDARY_DIST_Y,
	GENETIC_SIM_FWD,
	LAST_MOVE_DIR_X,
	LAST_MOVE_DIR_Y,
	LONGPROBE_POP_FWD,
	LONGPROBE_BAR_FWD,
	POPULATION_IP,
	POPULATION_FWD,
	POPULATION_LR,
	OSC1,
	AGE,
	BARRIER_FWD,
	BARRIER_LR,
	RANDOM,
	PHEROMONE_IP,
	PHEROMONE_FWD,
	PHEROMONE_LR
};

UENUM()
enum EAIDirections
{
	NORTH,
	SOUTH,
	EAST,
	WEST
};

USTRUCT(BlueprintType)
struct FAILikenessObjects
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSoftObjectPtr<UObject>> InterestedObjects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSoftObjectPtr<UObject>> NeutralObjects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSoftObjectPtr<UObject>> WaryObjects;
};

USTRUCT(BlueprintType)
struct FAILikenessLocations
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> InterestedLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> NeutralLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> WaryLocations;
};

USTRUCT(BlueprintType)
struct FAILikenessComponents : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAILikenessObjects Objects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAILikenessLocations Locations;
};

USTRUCT()
struct FAIGene
{
	GENERATED_BODY()

	uint16_t SourceType:1;

	uint16_t SourceNum:7;

	uint16_t SinkType:1;

	uint16_t SinkNum:7;

	int16_t Weight;
};

USTRUCT()
struct FAINeuralNet
{
	GENERATED_BODY()

	TArray<FAIGene> Connections;

	struct Neuron
	{
		float Output;
		bool Driven;
	};

	TArray<Neuron> Neurons;
};

USTRUCT()
struct FAIDIrection
{
	GENERATED_BODY()
	
	FRotator Rotation;

	FVector Location;
};

USTRUCT()
struct FAICharacterStats
{
	GENERATED_BODY()

	bool Alive;
	
	uint16_t index;
	
	FVector location;
	
	FVector StartLocation;

	unsigned Age;

	TArray<FAIGene> Genome;

	FAINeuralNet NeuralNet;

	float Responsiveness;

	unsigned OscillationPeriod;

	unsigned LongProbesDistance;

	FAIDIrection LastMovementDirection;

	unsigned SuccessRate;

	FVector KnownSpaceMin;
	
	FVector KnownSpaceMax;
};

USTRUCT()
struct FDisabledGenes
{
	GENERATED_BODY()

	TArray<EAIActions> DisabledActions;

	TArray<EAISensory> DisabledSenses;
};

USTRUCT()
struct FNeuron
{
	GENERATED_BODY()

	uint16_t RemappedNumber;
	
	uint16_t NumOutputs;
	
	uint16_t NumSelfInputs;
	
	uint16_t NumInputsFromSensorsOrOtherNeurons;
};