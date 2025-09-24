// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IlluviumSimulationComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnUnitSpawned, int32, UnitId, FIntPoint, SpawnPos, bool, bIsRed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitMoved, int32, UnitId, FIntPoint, NewPos);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitAttacked, int32, AttackerId, int32, TargetId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitDied, int32, UnitId);

struct ILLUVIUM_API FTestUnit
{
	FIntPoint Position;
	int ID;
	int Health;
	int MaxHealth;
	int StepsUntilNextAttack;
	bool bIsAlive;
	bool bIsRed;

	FTestUnit() : Position(0, 0), ID(-1), Health(1), MaxHealth(1), StepsUntilNextAttack(0), bIsAlive(true), bIsRed(true) {}

	//Override the comparison operator
	bool operator==(const FTestUnit& Other) const
	{
		return Position == Other.Position && ID == Other.ID && Health == Other.Health && MaxHealth == Other.MaxHealth 
			&& StepsUntilNextAttack == Other.StepsUntilNextAttack && bIsAlive == Other.bIsAlive && bIsRed == Other.bIsRed;
	}
};

FORCEINLINE uint32 GetTypeHash(const FTestUnit& Other)
{
	uint32 Hash = FCrc::MemCrc32(&Other, sizeof(FTestUnit));
	return Hash;
}

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ILLUVIUM_API UIlluviumSimulationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UIlluviumSimulationComponent();

	UFUNCTION(Category = "Simulation")
	void StartSimulation();

	UFUNCTION(Category = "Simulation")
	void StopSimulation();

	void CreateUnits();
	int32 SpawnUnit(bool bIsRed, const FIntPoint& Pos);

	UPROPERTY(EditAnywhere, Category = "Simulation")
	int32 GridWidth = 100;

	UPROPERTY(EditAnywhere, Category = "Simulation")
	int32 GridHeight = 100;

	UPROPERTY(EditAnywhere, Category = "Simulation")
	int32 TimeStepMs = 100;

	UPROPERTY(EditAnywhere, Category = "Simulation")
	int32 TimeStepsPerAttack = 2; // Attacks every N steps when in range

	UPROPERTY(EditAnywhere, Category = "Simulation")
	int32 SquaresAttackRange = 1;

	UPROPERTY(EditAnywhere, Category = "Simulation")
	int32 SquaresSpeedPerTimeStep = 1; // How many grid squares a unit moves per step

	UPROPERTY(EditAnywhere, Category = "Simulation")
	int32 Seed = 12345;

	UPROPERTY(EditAnywhere, Category = "Simulation")
	int32 RedUnits = 1;

	UPROPERTY(EditAnywhere, Category = "Simulation")
	int32 BlueUnits = 1;

	// Events

	UPROPERTY(BlueprintAssignable, Category = "Simulation|Events")
	FOnUnitMoved OnUnitMoved;

	UPROPERTY(BlueprintAssignable, Category = "Simulation|Events")
	FOnUnitAttacked OnUnitAttacked;

	UPROPERTY(BlueprintAssignable, Category = "Simulation|Events")
	FOnUnitDied OnUnitDied;

	UPROPERTY(BlueprintAssignable, Category = "Simulation|Events")
	FOnUnitSpawned OnUnitSpawned;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void StepSimulation();

	FTimerHandle Timer_Step;
	FRandomStream RandomStream;
	TMap<int32, FTestUnit> Units;
	int32 NextUnitId;
	bool bRunning;
};
