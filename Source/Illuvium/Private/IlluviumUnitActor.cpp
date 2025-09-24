// Fill out your copyright notice in the Description page of Project Settings.


#include "IlluviumUnitActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "IlluviumSimulationComponent.h"
#include "IlluviumGameModeBase.h"

AIlluviumUnitActor::AIlluviumUnitActor()
{
    PrimaryActorTick.bCanEverTick = true;
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    RootComponent->SetupAttachment(MeshComp);
    UnitId = -1;
    SimulationComp = nullptr;
    CellSize = 100.0f;
    LerpSpeed = 8.0f;
    LerpAlpha = 1.0f;
    bAlive = true;
}

void AIlluviumUnitActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Interpolate
    LerpAlpha = FMath::Clamp(LerpAlpha + DeltaTime * LerpSpeed, 0.0f, 1.0f);
    FVector NewPos = FMath::Lerp(PrevWorldPos, TargetWorldPos, LerpAlpha);
    SetActorLocation(NewPos);

    // Damage flash
    if (DynMat)
    {
        float Curr = 0.0f;
        DynMat->GetScalarParameterValue(TEXT("EmissiveStrength"), Curr);
        float New = FMath::FInterpTo(Curr, 0.0f, DeltaTime, 3.0f);
        DynMat->SetScalarParameterValue(TEXT("EmissiveStrength"), New);
    }
}

void AIlluviumUnitActor::InitializeVisual(int32 InUnitId, bool bIsRedTeam, UIlluviumSimulationComponent* InSim, const FIntPoint& StartPos)
{
    UnitId = InUnitId;
    SimulationComp = InSim;

    PrevWorldPos = GridToWorld(StartPos);
    TargetWorldPos = PrevWorldPos;
    SetActorLocation(PrevWorldPos);

    DynMat = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
    if (DynMat)
    {
        FLinearColor BaseColor = bIsRedTeam ? FLinearColor::Red : FLinearColor::Blue;
        DynMat->SetVectorParameterValue(TEXT("BaseColor"), BaseColor);
        DynMat->SetScalarParameterValue(TEXT("EmissiveStrength"), 0.0f);
    }

    if (SimulationComp)
    {
        SimulationComp->OnUnitMoved.AddDynamic(this, &AIlluviumUnitActor::HandleMoved);
        SimulationComp->OnUnitAttacked.AddDynamic(this, &AIlluviumUnitActor::HandleAttacked);
        SimulationComp->OnUnitDied.AddDynamic(this, &AIlluviumUnitActor::HandleDied);
    }
}

void AIlluviumUnitActor::HandleMoved(int32 Id, FIntPoint NewPos)
{
    if (Id != UnitId)
    {
        return;
    }

    PrevWorldPos = GetActorLocation();
    TargetWorldPos = GridToWorld(NewPos);
    LerpAlpha = 0.0f;
}

void AIlluviumUnitActor::HandleAttacked(int32 AttackerId, int32 TargetId)
{
    if (AttackerId == UnitId)
    {
        // Attack flash (increase emissive briefly)
        if (DynMat) DynMat->SetScalarParameterValue(TEXT("EmissiveStrength"), 2.0f);
    }
    if (TargetId == UnitId)
    {
        // Hit flash (set color or emissive)
        if (DynMat) DynMat->SetScalarParameterValue(TEXT("EmissiveStrength"), 4.0f);
    }
}

void AIlluviumUnitActor::HandleDied(int32 Id)
{
    if (Id != UnitId)
    {
        return;
    }
    bAlive = false;

    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
}

FVector AIlluviumUnitActor::GridToWorld(const FIntPoint& Pos) const
{
    return FVector(Pos.X * CellSize, Pos.Y * CellSize, 0.0f);
}

