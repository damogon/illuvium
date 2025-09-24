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
	// Precompute nearest target per unit
	TArray<TPair<FTestUnit*, FTestUnit*>> NearestTarget; // Attacker, Target
	for (auto& Unit : Units)
	{
		if (!Unit.Value.bIsAlive)
		{
			continue;
		}

		FTestUnit* Best = nullptr;
		int32 BestDist = INT32_MAX;
		for (auto& OtherUnit : Units)
		{
			if (Unit.Key == OtherUnit.Key || Unit.Value.bIsRed == OtherUnit.Value.bIsRed || !OtherUnit.Value.bIsAlive)
			{
				continue;
			}
			int32 Dist = FMath::Abs(Unit.Value.Position.X - OtherUnit.Value.Position.X) + FMath::Abs(Unit.Value.Position.Y - OtherUnit.Value.Position.Y);
			if (Dist < BestDist)
			{
				Best = &OtherUnit.Value;
				BestDist = Dist;
			}
		}
		if (Best != nullptr)
		{
			NearestTarget.Add(TPair<FTestUnit*, FTestUnit*>(&Unit.Value, Best));
		}
	}

	TArray<TPair<int32, FIntPoint>> MovesToBroadcast;
	for (auto& AttackerToTarget : NearestTarget)
	{
		// Has target?
		int32 TargetId = AttackerToTarget.Value->ID;
		if (TargetId == -1)
		{
			continue;
		}
		FTestUnit& Attacker = *AttackerToTarget.Key;
		FTestUnit& Target = *AttackerToTarget.Value;
		int32 Dist = FMath::Abs(Attacker.Position.X - Target.Position.X) + FMath::Abs(Attacker.Position.Y - Target.Position.Y);
		// In attack range?
		if (Dist <= SquaresAttackRange)
		{
			// Attack countdown
			if (Attacker.StepsUntilNextAttack <= 0)
			{
				// Attack happens
				OnUnitAttacked.Broadcast(Attacker.ID, TargetId);
				Target.Health -= 1;
				if (Target.Health <= 0)
				{
					Target.bIsAlive = false;
					OnUnitDied.Broadcast(Target.ID);
				}
				Attacker.StepsUntilNextAttack = TimeStepsPerAttack;
			}
			else
			{
				Attacker.StepsUntilNextAttack--;
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

			if (!(NewPos == Attacker.Position))
			{
				Attacker.Position = NewPos;
				MovesToBroadcast.Add(TPair<int32, FIntPoint>(Attacker.ID, Attacker.Position));
			}
		}
	}

	// Broadcast movements after all decisions so simulation ordering is deterministic
	for (auto& Pair : MovesToBroadcast)
	{
		OnUnitMoved.Broadcast(Pair.Key, Pair.Value);
	}
}