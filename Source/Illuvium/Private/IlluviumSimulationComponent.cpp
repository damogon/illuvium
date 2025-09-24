// Fill out your copyright notice in the Description page of Project Settings.


#include "IlluviumSimulationComponent.h"

UIlluviumSimulationComponent::UIlluviumSimulationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	NextUnitId = 0;
	bRunning = false;
}


void UIlluviumSimulationComponent::BeginPlay()
{
	Super::BeginPlay();

	RandomStream.Initialize(Seed);
	CreateUnits();
}

void UIlluviumSimulationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopSimulation();
	Super::EndPlay(EndPlayReason);
}

void UIlluviumSimulationComponent::CreateUnits()
{
	for (int i = 0; i < RedUnits; ++i)
	{
		SpawnUnit(true, FIntPoint(RandomStream.RandRange(0, GridWidth - 1), RandomStream.RandRange(0, GridWidth - 1)));
	}

	for (int i = 0; i < BlueUnits; ++i)
	{
		SpawnUnit(false, FIntPoint(RandomStream.RandRange(0, GridWidth - 1), RandomStream.RandRange(0, GridWidth - 1)));
	}
}

int32 UIlluviumSimulationComponent::SpawnUnit(bool bIsRed, const FIntPoint& Pos)
{
	FTestUnit New;
	New.ID = NextUnitId++;
	New.bIsRed = bIsRed;
	New.Position = Pos;
	New.MaxHealth = RandomStream.RandRange(2, 5);
	New.Health = New.MaxHealth;
	New.StepsUntilNextAttack = 0;
	New.bIsAlive = true;

	Units.Add(New.ID, New);
	OnUnitSpawned.Broadcast(New.ID, New.Position, bIsRed);

	return New.ID;
}

void UIlluviumSimulationComponent::StartSimulation()
{
	if (bRunning)
	{
		return;
	}

	bRunning = true;
	float Interval = TimeStepMs / 1000.0f;
	GetWorld()->GetTimerManager().SetTimer(Timer_Step, this, &UIlluviumSimulationComponent::StepSimulation, Interval, true);
}

void UIlluviumSimulationComponent::StopSimulation()
{
	if (!bRunning)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(Timer_Step);
	bRunning = false;
}

void UIlluviumSimulationComponent::StepSimulation()
{
	// Collect alive unit IDs
	TArray<int32> AliveIds;
	AliveIds.Reserve(Units.Num());
	for (const auto& Pair : Units)
	{
		if (Pair.Value.bIsAlive)
		{
			AliveIds.Add(Pair.Key);
		}
	}

	// Precompute nearest target per unit
	TMap<int32 /*AttackerId*/, int32 /*TargetId*/> NearestTarget;
	NearestTarget.Reserve(AliveIds.Num());

	for (int32 Id : AliveIds)
	{
		const FTestUnit& Attacker = Units[Id];
		int32 BestId = -1;
		int32 BestDist = INT32_MAX;

		for (int32 OtherId : AliveIds)
		{
			if (OtherId == Id)
				continue;

			const FTestUnit& Other = Units[OtherId];
			if (Other.bIsRed == Attacker.bIsRed)
				continue;

			const int32 Dist = ManhattanDist(Attacker.Position, Other.Position);
			if (Dist < BestDist)
			{
				BestId = OtherId;
				BestDist = Dist;
			}
		}

		if (BestId != -1)
		{
			NearestTarget.Add(Id, BestId);
		}
	}

	// Create an array of moves to ensure determinism
	TArray<TPair<int32, FIntPoint>> MovesToBroadcast;
	MovesToBroadcast.Reserve(AliveIds.Num());

	// Set of occuped positions for avoidance
	TSet<FIntPoint> OccupiedPositions;
	for (int32 Id : AliveIds)
	{
		OccupiedPositions.Add(Units[Id].Position);
	}

	for (int32 Id : AliveIds)
	{
		FTestUnit& Attacker = Units[Id];

		// Attack cooldown
		if (Attacker.StepsUntilNextAttack > 0)
		{
			Attacker.StepsUntilNextAttack--;
		}

		const int32* TargetIdPtr = NearestTarget.Find(Id);
		if (!TargetIdPtr)
		{
			continue;
		}

		FTestUnit& Target = Units[*TargetIdPtr];
		if (!Target.bIsAlive)
		{
			continue;
		}

		const int32 Dist = ManhattanDist(Attacker.Position, Target.Position);

		// In attack range?
		if (Dist <= SquaresAttackRange)
		{
			if (Attacker.StepsUntilNextAttack == 0)
			{
				// Attack
				OnUnitAttacked.Broadcast(Attacker.ID, Target.ID);

				Target.Health--;
				if (Target.Health <= 0)
				{
					Target.bIsAlive = false;
					OnUnitDied.Broadcast(Target.ID);
					OccupiedPositions.Remove(Target.Position);
				}

				Attacker.StepsUntilNextAttack = TimeStepsPerAttack;
			}
		}
		else
		{
			// Move toward target by up to SquaresSpeedPerTimeStep along Manhattan path
			int32 StepsLeft = SquaresSpeedPerTimeStep;
			FIntPoint NewPos = Attacker.Position;

			while (StepsLeft > 0 && NewPos != Target.Position)
			{
				if (NewPos.X != Target.Position.X)
				{
					NewPos.X += (Target.Position.X > NewPos.X) ? 1 : -1;
				}
				else if (NewPos.Y != Target.Position.Y)
				{
					NewPos.Y += (Target.Position.Y > NewPos.Y) ? 1 : -1;
				}
				StepsLeft--;
			}

			// Ensure not stepping onto tile occupied by same team unit
			if (!OccupiedPositions.Contains(NewPos) && NewPos != Attacker.Position)
			{
				OccupiedPositions.Remove(Attacker.Position);
				Attacker.Position = NewPos;
				OccupiedPositions.Add(NewPos);
				MovesToBroadcast.Emplace(Attacker.ID, NewPos);
			}
		}
	}

	// Broadcast movements after all decisions so simulation ordering is deterministic
	for (const auto& Pair : MovesToBroadcast)
	{
		OnUnitMoved.Broadcast(Pair.Key, Pair.Value);
	}
}

int32 UIlluviumSimulationComponent::ManhattanDist(const FIntPoint& A, const FIntPoint& B)
{
	return FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y);
}