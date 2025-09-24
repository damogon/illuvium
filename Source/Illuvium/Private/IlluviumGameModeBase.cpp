// Fill out your copyright notice in the Description page of Project Settings.


#include "IlluviumGameModeBase.h"
#include "IlluviumSimulationComponent.h"
#include "IlluviumUnitActor.h"

AIlluviumGameModeBase::AIlluviumGameModeBase()
{
	SimulationComp = CreateDefaultSubobject<UIlluviumSimulationComponent>(TEXT("SimulationComp"));
}

void AIlluviumGameModeBase::BeginPlay()
{
	if (SimulationComp)
	{
		SimulationComp->OnUnitSpawned.AddDynamic(this, &AIlluviumGameModeBase::HandleUnitSpawned);
		SimulationComp->StartSimulation();
	}

	Super::BeginPlay();
}

void AIlluviumGameModeBase::HandleUnitSpawned(int32 UnitId, FIntPoint SpawnPos, bool bIsRed)
{
	if (!UnitActorClass)
	{
		return;
	}

	FVector WorldLocation(SpawnPos.X * 100.0f, SpawnPos.Y * 100.0f, 0.0f);
	FActorSpawnParameters Params;
	AIlluviumUnitActor* NewActor = GetWorld()->SpawnActor<AIlluviumUnitActor>(UnitActorClass, WorldLocation, FRotator::ZeroRotator, Params);

	if (NewActor && SimulationComp)
	{
		NewActor->InitializeVisuals(UnitId, bIsRed, SimulationComp, SpawnPos);
	}
}
