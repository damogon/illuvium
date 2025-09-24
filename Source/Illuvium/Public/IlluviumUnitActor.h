// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IlluviumUnitActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UIlluviumSimulationComponent;

UCLASS()
class ILLUVIUM_API AIlluviumUnitActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AIlluviumUnitActor();

	virtual void Tick(float DeltaTime) override;
	void InitializeVisual(int32 InUnitId, bool bIsRedTeam, UIlluviumSimulationComponent* InSim, const FIntPoint& StartPos);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(BlueprintReadWrite, Category = "Simulation")
	int32 UnitId;

	UPROPERTY(BlueprintReadWrite, Category = "Simulation")
	TObjectPtr<UIlluviumSimulationComponent> SimulationComp;

	UPROPERTY(EditAnywhere, Category = "Simulation")
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Simulation")
	float LerpSpeed = 8.0f;

private:

	// Handlers
	UFUNCTION()
	void HandleMoved(int32 Id, FIntPoint NewPos);
	UFUNCTION()
	void HandleAttacked(int32 AttackerId, int32 TargetId);
	UFUNCTION()
	void HandleDied(int32 Id);

	// Helpers
	FVector GridToWorld(const FIntPoint& Pos) const;

	UMaterialInstanceDynamic* DynMat;
	FVector PrevWorldPos;
	FVector TargetWorldPos;
	float LerpAlpha;
	bool bAlive;
};
