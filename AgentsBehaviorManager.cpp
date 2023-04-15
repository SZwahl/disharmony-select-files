// Fill out your copyright notice in the Description page of Project Settings.


#include "AgentsBehaviorManager.h"

#include "AgentNavigationManager.h"
#include "DisharmonyGameMode.h"
#include "TransportManager.h"
#include "Managers/Mechs/CuttersManagerAgain.h"
#include "Managers/Mechs/LimbsManager.h"
#include "Managers/Mechs/WeaverManager.h"
#include "Mechs/Mech_Cutter_Limb.h"
#include "Mechs/Mech_Weaver.h"
#include "Mechs/Mech_Cutter.h"

// Sets default values
AAgentsBehaviorManager::AAgentsBehaviorManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

TArray<AMech_Cutter*>* AAgentsBehaviorManager::GetJoblessCutters()
{
	return &CutterManager->JoblessCutters;
}


void AAgentsBehaviorManager::InitializeSystem()
{
	if (!WOrkIt)
		return;

	CutterManager = NewObject<UCuttersManagerAgain>(this, CutterManagerBP, FName("CutterManager"));
	CutterManager->GameMode = GameMode;
	CutterManager->BehaviorManager = this;

	LimbsManager = NewObject<ULimbsManager>(this, LimbsManagerBP, FName("LimbsManager"));
	LimbsManager->GameMode = GameMode;
	LimbsManager->BehaviorManager = this;

	WeaverManager = NewObject<UWeaverManager>(this, WeaverManagerBP, FName("WeaverManager"));
	WeaverManager->GameMode = GameMode;
	WeaverManager->BehaviorManager = this;
	
	for (AActor* cut : StartingCutters)
	{
		FActorSpawnParameters spawnParams;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		FTransform spawnT = FTransform(cut->GetActorLocation());
		AMech_Cutter* SpawnedUnit = Cast<AMech_Cutter>(GetWorld()->SpawnActor(CutterBP, &spawnT, spawnParams));

		CutterManager->InitializeCutter(SpawnedUnit);
	}

	for (AMech_Weaver* weav : StartingWeavers)
	{
		WeaverManager->InitializeWeaver(weav);
	}

}

void AAgentsBehaviorManager::UpdateNeedsPlanInitiation()
{
	//Until the plans are empty
	while (NeedPlanInitiate.Num() > 0)
	{
		//Iterate through every agent that needs to act
		for (auto It = NeedPlanInitiate.CreateConstIterator(); It; ++It)
		{
			auto agent = *It;
			//Peek top action and skip if no plan slot
			auto thisPlan = AgentPlans.Find(agent);
			if (!thisPlan)
			{
				UE_LOG(LogTemp, Error, TEXT("Agent has no plan?"))
				continue;
			}
			//None left means plan's over
			else if (thisPlan->PlanStack.Num() == 0)
			{
				continue;
			}

			//Get action
			auto action = thisPlan->PlanStack.Pop();

			//Record actions
			if (!ActionLogs.Contains(agent))
			{
				ActionLogs.Add(agent, FActionLog());
			}
			ActionLogs[agent].PrevActions.Insert(action.Type, 0);
			if (ActionLogs[agent].PrevActions.Num() > 10) ActionLogs[agent].PrevActions.Pop();

			//Switch on type, perform according function
			switch(action.Type)
			{
			case AT_GoToLoc:
				{
					GoingToLoc.Add(agent, action);
					break;
				}
			case AT_GoToObj:
				{
					GoingToObj.Add(agent, action);
					break;
				}
			default:
				{
					auto agClass = agent->GetClass();
					if (agClass->IsChildOf(AMech_Cutter::StaticClass()))
					{
						CutterManager->CreateActionObject(action, agent);
					}
					else if (agClass->IsChildOf(AMech_Cutter_Limb::StaticClass()))
					{
						LimbsManager->CreateActionObject(action, agent);
					}
					else if (agClass->IsChildOf(AMech_Weaver::StaticClass()))
					{
						WeaverManager->CreateActionObject(action, agent);
					}
					break;
				}
			}
		}
		
		NeedPlanInitiate.Empty();
		
		//Execute all actions and gather leftovers
		TSet<AActor*> LeftoverAgents;
		LeftoverAgents.Append(ExecuteGenericActions());
		LeftoverAgents.Append(CutterManager->ExecuteAllActions());
		LeftoverAgents.Append(LimbsManager->ExecuteAllActions());
		LeftoverAgents.Append(WeaverManager->ExecuteAllActions());
		
		NeedPlanInitiate = LeftoverAgents;
	}
	
	
}

TSet<AActor*> AAgentsBehaviorManager::ExecuteGenericActions()
{
	TSet<AActor*> Leftovers;

	//Go to loc
	for (auto& Elem : GoingToLoc)
	{
		if (GoToLoc(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	GoingToLoc.Empty();

	for (auto& Elem : GoingToObj)
	{
		if (GoToObj(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	GoingToObj.Empty();

	return Leftovers;
}

bool AAgentsBehaviorManager::GoToLoc(FPlanAction action, AActor* agent)
{
	bool bUseWaypoints = true;
	auto agClass = agent->GetClass();
				
	if (agClass->IsChildOf(AMech_Cutter_Limb::StaticClass()))
		bUseWaypoints = false;

	//Try initiate path
	bool bAlreadyReached = 
		GameMode->AgentNavManager->InitiatePath(agent, action.TargetTransform.GetLocation(), agClass, bUseWaypoints, action.EndRadius);	

	//Add to leftover if already close enough
	if (bAlreadyReached)
		return true;

	AgentsPendingMovement.Add(agent);
	
	return false;
}

bool AAgentsBehaviorManager::GoToObj(FPlanAction action, AActor* agent)
{
	bool bUseWaypoints = true;
	auto agClass = agent->GetClass();
				
	if (agClass == AMech_Cutter_Limb::StaticClass())
		bUseWaypoints = false;

	//Try initiate path
	AActor* actor = Cast<AActor>(action.RelevantObject);
	bool bAlreadyReached = 
		GameMode->AgentNavManager->InitiatePath(agent, actor->GetActorLocation(), agClass, bUseWaypoints, action.EndRadius);

	//Leftover if already reached
	if (bAlreadyReached)
		return true;

	AgentsPendingMovement.Add(agent);
				
	return false;
}

// Called every frame
void AAgentsBehaviorManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!WOrkIt) return;
	
	if (CutterManager)
		CutterManager->PerformTick(DeltaTime);
	if (LimbsManager)
		LimbsManager->PerformTick(DeltaTime);
	//if (WeaverManager)
	//	WeaverManager->PerformTick(DeltaTime);
	
	UpdateNeedsPlanInitiation();
}


void AAgentsBehaviorManager::MakeAgentAct(AActor* agent)
{
	if (GameMode->TransportManager->HasStaticSpline(agent))
	{
		UE_LOG(LogTemp, Error, TEXT("hold the fuck up"))
	}
	else if (AgentsPendingMovement.Contains(agent))
	{
		//UE_LOG(LogTemp, Error, TEXT("hold the hot heck up"))
		GameMode->AgentNavManager->WipeAgentPathfinding(agent);
		AgentsPendingMovement.Remove(agent);
	}
	NeedPlanInitiate.Add(agent);
}

void AAgentsBehaviorManager::MarkAgentDoneMoving(AActor* agent)
{
	if (AgentsPendingMovement.Contains(agent))
	{
		AgentsPendingMovement.RemoveSwap(agent);
		MakeAgentAct(agent);
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("Eating unsynced move"));
	}
}

void AAgentsBehaviorManager::AppendMechInstructions(TArray<FPlanAction> plan, AActor* agent)
{
	AgentPlans[agent].PlanStack.Append(plan);
}


bool AAgentsBehaviorManager::ExtendAndRotateVector(FVector& result, FVector start, FVector end, float rExtent, float rAngle)
{
	//Get direction from two points, normalize, and make 2D
	FVector dir = end - start;
	dir.Z = 0;
	dir.Normalize();
	dir *= rExtent;
	
	//Append direction to the end point, then rotate around Z axis by angle
	FVector GuessPoint = end + dir.RotateAngleAxis(rAngle, FVector(0,0,1));
	
	//Raycast down to terrain to attempt correction
	FHitResult hit;
	FVector adjust = FVector(0,0,2000);
	bool bSuccess = GetWorld()->LineTraceSingleByChannel(hit, GuessPoint + adjust, GuessPoint - adjust, ECollisionChannel::ECC_Visibility);
	
	//Assign if finds the ground
	if (bSuccess)
		GuessPoint = hit.Location;
	else
		UE_LOG(LogTemp, Warning, TEXT("Extended/Rotated vector was unable to snap to terrain."))

	//Result will be the guess point
	result = GuessPoint;
	return bSuccess;
}

