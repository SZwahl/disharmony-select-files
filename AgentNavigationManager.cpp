// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/AgentNavigationManager.h"

#include "CitadelBrainManager.h"
#include "DisharmonyGameMode.h"
#include "NavigationSystem.h"
#include "EngineUtils.h"
#include "AgentsBehaviorManager.h"
#include "Mechs/GenericMech.h"

// Sets default values
AAgentNavigationManager::AAgentNavigationManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AAgentNavigationManager::InitializeSystem()
{
	//Get Navigation System
	NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
		UE_LOG(LogTemp, Warning, TEXT("NavSys didn't get right"))
}

void AAgentNavigationManager::UpdateSimpleMoves(float DeltaTime)
{
	for (auto& elem : AgentPathMap)
	{
		AActor* actor = elem.Key;
		FVector curLoc = elem.Value.CurrentLocation;
		float movement = AgentProperties[elem.Value.Type].Speed * DeltaTime;
		
		float useRadius = elem.Value.AcceptanceRadius + AgentProperties[elem.Value.Type].AgentRadius;
		

		//Loop through up to the amount of points in the path
		for (int32 i = 0; i < elem.Value.RemainingPoints.Num(); i++)
		{
			//If editor fast as fuck, teleport to true dest
			if (bSuperFastMode)
			{
				curLoc = elem.Value.TrueEndLocation;
				ReachedAgents.Add(actor);
				break;
			}
			
			//TargetTransform is first point, find distance
			FVector curDest = elem.Value.RemainingPoints[0];
			float nextDist = FVector::Dist(curLoc, curDest);
			
			//if moves further than next point
			if (movement >= nextDist)
			{
				//update point and remaining distance
				curLoc = curDest;
				movement -= nextDist ;
				elem.Value.RemainingPoints.RemoveAt(0);
			}
			else
			{
				//If not, update location towards next point and continue;
				FVector dV = curDest - curLoc;
				dV.Normalize();
				dV *= movement;
				curLoc += dV;
			}
			

			//Finish path early if inside radius
			bool pFinish = FVector::Dist(curLoc, elem.Value.TrueEndLocation) <= useRadius;
			if (pFinish)
			{
				//else finish path
				ReachedAgents.Add(actor);
				break;
			}

			//Else consider closeness to end of points
			bool wFinish;
			//If points remaining
			if (!elem.Value.RemainingPoints.Num() == 0)
			{
				//Finish is if agent within acceptance radius
				wFinish = FVector::Dist(curLoc, elem.Value.TrueEndLocation) <= useRadius;
			}
			else wFinish = true;
			
			if (wFinish)
			{
				//Waypoint if not truly done
				PseudoFinishedAgents.Add(actor);
				break;
			}
		}
		AgentPathMap[actor].CurrentLocation = curLoc;
		actor->SetActorLocation(curLoc);
	}
}

void AAgentNavigationManager::UpdateReachedAgents()
{
	for (auto actor : ReachedAgents)
	{
		auto pData = AgentPathMap[actor];
		AgentPathMap.Remove(actor); 

		if (!AgentWaypointsMap.Contains(actor) || AgentWaypointsMap[actor].Location.Num() == 0)
		{
			AgentWaypointsMap.Remove(actor);
			GameMode->BehaviorManager->MarkAgentDoneMoving(actor);
			//UE_LOG(LogTemp, Warning, TEXT("Finished pathing for %s!"), *actor->GetName())
			continue;
		}
		
		//Pop
		AgentWaypointsMap[actor].Location.Pop();
		AgentWaypointsMap[actor].AcceptanceRadius.Pop();
		
		//If more locations, try generate path again
		if (AgentWaypointsMap[actor].Location.Num() > 0)
		{
			TryGeneratePath(actor, pData.Type);
			//UE_LOG(LogTemp, Warning, TEXT("Targeting next waypoint for %s!"), *actor->GetName())
		}
		//Otherwise remove from map
		else
		{
			AgentWaypointsMap.Remove(actor);
			GameMode->BehaviorManager->MarkAgentDoneMoving(actor);
			//UE_LOG(LogTemp, Warning, TEXT("Finished pathing for %s!"), *actor->GetName())
		}
	}
	ReachedAgents.Empty();
}

void AAgentNavigationManager::UpdatePseudoFinishedAgents()
{
	for (auto actor : PseudoFinishedAgents)
	{
		auto pData = AgentPathMap[actor];
		AgentPathMap.Remove(actor); 
		
		TryGeneratePath(actor, pData.Type);
	}
	PseudoFinishedAgents.Empty();
}

void AAgentNavigationManager::UpdateStuckRetryAgents(float DeltaTime)
{
	TArray<AActor*> Remove;
	for (auto& elem : RetryActorList)
	{
		elem.Value.time += DeltaTime;
		if (elem.Value.time >= PseudoRetryTime)
		{
			//Project actor to navigation to get "unstuck"
			const FVector extent =  FVector(500, 500, 4000);
			FVector thisLoc = elem.Key->GetActorLocation();
			FNavLocation newLoc;
			bool bSuccess = NavSys->ProjectPointToNavigation(thisLoc, newLoc, extent);

			if (bSuccess)
			{
				elem.Key->SetActorLocation(newLoc.Location);
			}
			
			//Do it again
			if (AgentWaypointsMap.Contains(elem.Key))
			{
				TryGeneratePath(elem.Key, elem.Value.type);
			}
			Remove.Add(elem.Key);
			//UE_LOG(LogTemp, Warning, TEXT("Retrying this %s!"), *elem.Key->GetName())
		}
	}
	for (auto actor : Remove)
	{
		RetryActorList.Remove(actor);
	}
}

// Called every frame
void AAgentNavigationManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	UpdateSimpleMoves(DeltaTime);

	UpdatePseudoFinishedAgents();

	UpdateReachedAgents();

	UpdateStuckRetryAgents(DeltaTime);
}

//Returns true if already reached, otherwise starts up a pathfinding
bool AAgentNavigationManager::InitiatePath(AActor* actor, FVector endLocation, UClass* type, bool btryUseWaypoints, float acceptanceRadius)
{
	WipeAgentPathfinding(actor);

	
	//Don't work if already near
	float dist = FVector::Dist(actor->GetActorLocation(), endLocation);
	if (dist < acceptanceRadius)
	{
		//GameMode->BehaviorManager->MakeAgentAct(actor);
		return true;
	}
	

	
	FWaypointNodes wNodes;
	///looool
	btryUseWaypoints = false;
	if (btryUseWaypoints)
	{
		// //Get waypoint path
		// FVector start = actor->GetActorLocation();
		// auto wPoints = GameMode->CitadelManager->GetWaypointPath(start, endLocation, 20000);
		//
		// //if successful...
		// if (wPoints.Num() > 0)
		// {
		// 	wNodes.Location = wPoints;
		// 	//Set acceptance radii to default, and end to the input radius
		// 	for (int32 i = 0; i <= wPoints.Num() -2; i++)
		// 	{
		// 		wNodes.AcceptanceRadius.Add(WaypointAcceptanceRadius);
		// 	}
		// 	wNodes.AcceptanceRadius.Add(acceptanceRadius);
		//
		// 	//Store waypoint path
		// 	AgentWaypointsMap.Add(actor, wNodes);
		// 	//Call trygenerate
		// 	TryGeneratePath(actor, type);
		// }
		// else
		// {
		// 	UE_LOG(LogTemp, Warning, TEXT("Can't initialize path for %s, waypoints totally empty?"), *actor->GetName());
		// }
	}

	wNodes.Location.Add(endLocation);
	wNodes.AcceptanceRadius.Add(acceptanceRadius);
	AgentWaypointsMap.Add(actor, wNodes);
	//Generate path normally
	TryGeneratePath(actor, type);
	return false;
}


/**
 * @brief : Attempt to project the vector to navmesh and update value if successful
 * @param v : Location we need to project
 * @return : A boolean for if the projection was succesful and the vector was updated
 */
bool AAgentNavigationManager::TryProjectToNavmesh(FVector& v)
{
	//Setup
	const FVector extent =  FVector(3000, 3000, 8000);
	FNavLocation endLoc;

	//Try to project
	bool bProjectSuccess = NavSys->ProjectPointToNavigation(v, endLoc, extent);
	
	//Update value if it worked
	if (bProjectSuccess)
	{
		v = endLoc.Location;
		return true;
	}
	return false;
}

void AAgentNavigationManager::TryGeneratePath(AActor* actor, UClass* type)
{
	if (NavSys)
	{
		auto waypoints = AgentWaypointsMap[actor];

		if (!AgentProperties.Contains(type))
		{
			UE_LOG(LogTemp, Error, TEXT("Type %s isn't defined in Agent Properties! Cannot pathfind."), type)
			return;
		}
		
		//Set agent properties
		FNavAgentProperties navAgProps;
		navAgProps.AgentHeight = AgentProperties[type].AgentHeight;
		navAgProps.AgentRadius = AgentProperties[type].AgentRadius;
		navAgProps.AgentStepHeight = 900;
		navAgProps.bCanWalk = true;

		//Set locations
		FPathFindingQuery query;
		query.StartLocation = actor->GetActorLocation();

		TryProjectToNavmesh(waypoints.Location.Top());
		FVector endTarget = waypoints.Location.Top();

		//If end location further than navigation invoker,
		//get location distance away, project onto nav mesh
		if (FVector::Dist(query.StartLocation, endTarget) > 3000)
		{
			FVector dV = endTarget - query.StartLocation;
			dV.Normalize();
			dV *= 3000;
			dV += query.StartLocation;
			const FVector extent =  FVector(3000, 3000, 8000);
			FNavLocation endLoc;
			bool bSuccess = NavSys->ProjectPointToNavigation(dV, endLoc, extent);

			if (bSuccess)
			{
				query.EndLocation = endLoc.Location;
				//UE_LOG(LogTemp, Warning, TEXT("Found pseudo point at %s."), *endLoc.Location.ToString())
			}
			else
			{
				//UE_LOG(LogTemp, Warning, TEXT("Unable to find navigable pseudo point for %s at %s near %s."), *actor->GetName(), *dV.ToString(), *endTarget.ToString())
			}
		}
		else
			query.EndLocation = endTarget;

		

		//Set nav data and query filter
		ANavigationData* navData = NavSys->MainNavData;
		if (!navData || !NavSys->MainNavData)
		{
			UE_LOG(LogTemp, Error, TEXT("No nav data found for %s! Skipping pathing."), *actor->GetName())
			return;
		}
		query.QueryFilter = UNavigationQueryFilter::GetQueryFilter(*navData, actor, FilterClass);
		query.NavData = navData;
		query.bAllowPartialPaths = true;
		query.SetAllowPartialPaths(true);

		//Bind path found delegate
		FNavPathQueryDelegate del;
		del.BindUObject(this, &AAgentNavigationManager::PathFound);

		//Start finding a path
		int32 qID = NavSys->FindPathAsync(navAgProps, query, del, EPathFindingMode::Regular);

		//Add query to map for later reference
		FAgentQueryInfo mqInfo;
		mqInfo.Actor = actor;
		mqInfo.Type = type;
		mqInfo.AcceptanceRadius = waypoints.AcceptanceRadius.Top();
		mqInfo.TrueEndLocation = endTarget;
		QueryActorMap.Add(qID, mqInfo);

		AsyncPendingAgents.Add(actor, endTarget);
	}
}

void AAgentNavigationManager::PathFound(uint32 aPathId, ENavigationQueryResult::Type aResultType, FNavPathSharedPtr aNavPointer)
{
	switch (aResultType)
	{
		case ENavigationQueryResult::Type::Success:
			{
				auto points = aNavPointer->GetPathPoints();
				auto mqInfo = *QueryActorMap.Find(aPathId);
				auto name = mqInfo.Actor->GetName();
				//UE_LOG(LogTemp, Warning, TEXT("Path found for actor %s via query %d with %d points!"), *name, aPathId, points.Num())

				//Check if this was abandoned
				if (!AsyncPendingAgents.Contains(mqInfo.Actor))
				{
					//UE_LOG(LogTemp, Error, TEXT("Nullifying found path - can't find agent"))
					break;
				}
				FVector susLoc = AsyncPendingAgents[mqInfo.Actor];
				if (!susLoc.Equals(mqInfo.TrueEndLocation))
				{
					//UE_LOG(LogTemp, Error, TEXT("Nullifying found path - was overriden"))
					break;
				}
				//Otherwise you can now remove
				AsyncPendingAgents.Remove(mqInfo.Actor);
					
				//Check if path is empty?
				if (points.Num() == 0)
				{
					UE_LOG(LogTemp, Error, TEXT("Path query success, but no points!"))
					break;
				}
				
				FAgentPathData pData;
				//Immediately log current location
				pData.CurrentLocation = mqInfo.Actor->GetActorLocation();
				pData.Type = mqInfo.Type;
				pData.AcceptanceRadius = mqInfo.AcceptanceRadius;
				pData.TrueEndLocation = mqInfo.TrueEndLocation;
				//Get points as vectors
				for (FNavPathPoint p : points)
				{
					pData.RemainingPoints.Add(p.Location);
				}
				
				AgentPathMap.Add(mqInfo.Actor, pData);
				break;
			}


		case ENavigationQueryResult::Type::Fail:
			{
				UE_LOG(LogTemp, Warning, TEXT("Path query %d failed!"), aPathId)
				break;
			}

		case ENavigationQueryResult::Type::Error:
			{
				//UE_LOG(LogTemp, Error, TEXT("Path query %d made an error!!"), aPathId)
				//Likely tried reaching out of nav mesh bounds, allot a retry
				auto mqInfo = *QueryActorMap.Find(aPathId);
				FRetryPackage retry;
				retry.time = 0;
				retry.type = mqInfo.Type;
				RetryActorList.Add(mqInfo.Actor, retry);
				break;
			}

		case ENavigationQueryResult::Type::Invalid:
			{
				UE_LOG(LogTemp, Error, TEXT("Path query %d is invalid"))
				break;
			}

		default: break;
	}	
	QueryActorMap.Remove(aPathId);
}

void AAgentNavigationManager::WipeAgentPathfinding(AActor* actor)
{
	AsyncPendingAgents.Remove(actor);
	ReachedAgents.RemoveSwap(actor);
	AgentWaypointsMap.Remove(actor);
	AgentPathMap.Remove(actor);
	RetryActorList.Remove(actor);
	PseudoFinishedAgents.Remove(actor);
}
