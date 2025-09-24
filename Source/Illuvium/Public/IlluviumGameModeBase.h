// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "IlluviumGameModeBase.generated.h"

class UIlluviumSimulationComponent;

UCLASS()
class ILLUVIUM_API AIlluviumGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:

	AIlluviumGameModeBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Simulation")
	TObjectPtr<UIlluviumSimulationComponent> SimulationComp;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleUnitSpawned(int32 UnitId, FIntPoint SpawnPos, bool bIsRed);
	
};
