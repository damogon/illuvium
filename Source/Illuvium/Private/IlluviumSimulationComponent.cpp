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
	//Pathing and attacking
}