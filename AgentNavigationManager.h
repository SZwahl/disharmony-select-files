// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AgentNavigationManager.generated.h"


class ADisharmonyGameMode;
class UNavigationSystemV1;
class UNavigationQueryFilter;
class AGenericMech;

USTRUCT()
struct FRetryPackage
{
	GENERATED_BODY()

	UPROPERTY()
	float time;
	UPROPERTY()
	UClass* type;
};

USTRUCT()
struct FWaypointNodes
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FVector> Location;
	UPROPERTY()
	TArray<float> AcceptanceRadius;
};

USTRUCT()
struct FAgentQueryInfo
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* Actor = nullptr;
	UPROPERTY()
	UClass* Type;
	UPROPERTY(EditAnywhere)
	float AcceptanceRadius = 10;
	UPROPERTY(EditAnywhere)
	FVector TrueEndLocation;
};

USTRUCT()
struct FAgentPathData
{
	GENERATED_BODY()

	UPROPERTY()
	UClass* Type;
	UPROPERTY(EditAnywhere)
	float AcceptanceRadius = 10;
	UPROPERTY()
	FVector CurrentLocation;
	UPROPERTY(EditAnywhere)
	FVector TrueEndLocation;
	UPROPERTY()
	TArray<FVector> RemainingPoints;
};

USTRUCT()
struct FAgentData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float Speed = 800;
	UPROPERTY(EditAnywhere)
	float AgentHeight = 300;
	UPROPERTY(EditAnywhere)
	float AgentRadius = 450;
};

UCLASS()
class UNSATISFACTORY_API AAgentNavigationManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAgentNavigationManager();
	

protected:

	UPROPERTY()
	ADisharmonyGameMode* GameMode;

	//Editor tools
	UPROPERTY(EditAnywhere, Category = "Editor")
	bool bSuperFastMode = false;

	//
	UPROPERTY(EditAnywhere)
	TSubclassOf<UNavigationQueryFilter> FilterClass;

	//Parameters
	UPROPERTY(EditAnywhere)
	float WaypointAcceptanceRadius = 10000;
	UPROPERTY(EditAnywhere)
	float PseudoRetryTime = 2;
	
	//References
	UPROPERTY()
	UNavigationSystemV1* NavSys;

	UPROPERTY(EditAnywhere)
	TMap<UClass*, FAgentData> AgentProperties;

	//The Data
	UPROPERTY()
	TMap<int32, FAgentQueryInfo> QueryActorMap;
	UPROPERTY()
	TMap<AActor*, FVector> AsyncPendingAgents;
	UPROPERTY()
	TMap<AActor*, FAgentPathData> AgentPathMap;
	UPROPERTY()
	TMap<AActor*, FWaypointNodes> AgentWaypointsMap;
	UPROPERTY()
	TMap<AActor*, FRetryPackage> RetryActorList;
	UPROPERTY()
	TArray<AActor*> ReachedAgents;
	UPROPERTY()
	TArray<AActor*> PseudoFinishedAgents;
	
	void UpdateSimpleMoves(float DeltaTime);
	void UpdateReachedAgents();
	void UpdatePseudoFinishedAgents();

	void TryGeneratePath(AActor* actor, UClass* type);
	void PathFound(uint32 aPathId, ENavigationQueryResult::Type aResultType, FNavPathSharedPtr aNavPointer);

public:
	//Initialzied by game mode
	void InitializeSystem();
	void UpdateStuckRetryAgents(float DeltaTime);
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	void RegisterGameMode(ADisharmonyGameMode* gm)
	{
		GameMode = gm;
	}
	
	FVector GetActorTrueDestination(AActor* agent)
	{
		if (AgentPathMap.Find(agent))
			return AgentPathMap[agent].TrueEndLocation;
		return agent->GetActorLocation();
	}
	
	bool InitiatePath(AActor* actor, FVector endLocation, UClass* type, bool btryUseWaypoints, float acceptanceRadius = 10);
	bool TryProjectToNavmesh(FVector &v);
	void WipeAgentPathfinding(AActor* actor);
};
