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
	void InitializeVisuals(int32 InUnitId, bool bIsRedTeam, UIlluviumSimulationComponent* InSimComp, const FIntPoint& StartPos);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(BlueprintReadWrite, Category = "Simulation")
	int32 UnitId = -1;

	UPROPERTY(BlueprintReadWrite, Category = "Simulation")
	TObjectPtr<UIlluviumSimulationComponent> SimulationComp = nullptr;

	UPROPERTY(EditAnywhere, Category = "Simulation")
	float SquareSize = 100.0f;

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

	// Helper
	FVector GridToWorld(const FIntPoint& Pos) const;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynMat;

	FVector PrevWorldPos;
	FVector TargetWorldPos;
	float LerpAlpha = 1.0f;
	bool bAlive = true;
};
