// Fill out your copyright notice in the Description page of Project Settings.


#include "IlluviumGameModeBase.h"
#include "IlluviumSimulationComponent.h"

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

}
