// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIDataTypes.h"
#include "../Movement-Setup/ActionSetup.h"
#include "Kismet/GameplayStatics.h"
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

	virtual void BeginPlay() override;

	FDisabledGenes DisabledGenes;

	FAICharacterStats CharacterStats;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAILikenessComponents FAILikenessComponents;

	float SuccessRate;

	EAISensory SensoryType;
	EAIActions ActionType;

private:
	void ExecuteAction(TMap<EAIActions, float> &ActionLevels);

	TMap<EAIActions, float> SensorToAction(unsigned CurrStep);

	bool ActionEnabled(EAIActions Action);

	FHitResult DistanceObjectHit(EAIDirections Direction, ECollisionChannel Channel, EDrawDebugTrace::Type Debug,
	                             FColor TraceColor = FColor::Red, FColor TraceHitColor = FColor::Green);

	float GetSensor(EAISensory Sensor, unsigned CurrStep, EDrawDebugTrace::Type Debug = EDrawDebugTrace::None);

	void UpdateEntity(unsigned CurrStep);

    TArray<FAIGene> RandomGenomeGenerator();

	void WireGenomes();

	void CutNeuron(uint16_t NeuronNum,TArray<FAIGene>& Connections, TMap<uint16_t, FNeuron> NeuronMap);

	static constexpr uint8_t ACTION = 1, SENSOR = 1, NEURON = 0;
	float MaxSensorRange = 3000.0f;
	TArray<AActor*> PopulationRef;

	int GenomeInitialLengthMin;
	int GenomeInitialLengthMax;
	unsigned GenomeMaxLength;
	unsigned StepsPerGeneration;
	unsigned MaxNumberNeurons;
	double PointMutationRate;
	double GeneInsertionDeletionRate;
	double DeletionRatio;
	float Responsiveness;
};
