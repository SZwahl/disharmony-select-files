// Fill out your copyright notice in the Description page of Project Settings.


#include "LimbsManager.h"

#include "CuttersManagerAgain.h"
#include "CuttingTree.h"
#include "DisharmonyGameMode.h"
#include "Data/TreePropertyDatabase.h"
#include "Managers/AgentNavigationManager.h"
#include "Managers/AgentsBehaviorManager.h"
#include "Managers/MoriManager.h"
#include "Mechs/Mech_Cutter.h"
#include "Mechs/Mech_Cutter_Limb.h"

void ULimbsManager::PerformTick(float DeltaTime)
{
	UpdateFollowingLimbs(DeltaTime);
	UpdateGrindingLimbs(DeltaTime);
}

void ULimbsManager::CreateActionObject(FPlanAction action, AActor* agent)
{
	AMech_Cutter_Limb* limb = Cast<AMech_Cutter_Limb>(agent);
	
	switch (action.Type)
	{
	case AT_FollowCutter:
		{
			Follow.Add(limb, action);
			break;
		}
	case AT_GetNextTree:
		{
			NextTree.Add(limb, action);
			break;
		}
	case AT_GrindTree:
		{
			Grind.Add(limb, action);
			break;
		}
	default:
		{
			break;
		}
	}
}

TSet<AActor*> ULimbsManager::ExecuteAllActions()
{
	TSet<AActor*> Leftovers;

	//Limbs start follow
	for (auto& Elem : Follow)
	{
		if (FollowCutter(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	Follow.Empty();

	//Get next tree
	for (auto& Elem : NextTree)
	{
		TArray<AActor*> leftover = TryGetNextTree(Elem.Value, Elem.Key);
		for (AActor* l : leftover)
		{
			Leftovers.Add(l);
		}
	}
	NextTree.Empty();

	//Grind tree
	for (auto& Elem : Grind)
	{
		if (GrindTree(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	Grind.Empty();
	
	return Leftovers;
}

/*Action Functions*/

//------------------------------------------------
//------------------------------------------------


bool ULimbsManager::FollowCutter(FPlanAction action, AMech_Cutter_Limb* limb)
{
	//Follow cutter and await re-instruction
	FollowingLimbs.Add(Cast<AMech_Cutter_Limb>(limb), Cast<USceneComponent>(action.RelevantObject));

	return false;
}

TArray<AActor*> ULimbsManager::TryGetNextTree(FPlanAction action, AMech_Cutter_Limb* limb)
{
	TArray<AActor*> leftovers;
	
	FCutterQueue* qPtr = LimbQueues.Find(limb);

	//No trees left
	if (!qPtr || qPtr->trees.Num() == 0)
	{
		//Remove work queue
		LimbQueues.Remove(limb);

		//Mark stopped as in finished
		AMech_Cutter* parent = LimbParents[limb];
		bool bAllDone = BehaviorManager->GetCutterManager()->StopAndCheckDone(parent, limb, false);
		if (bAllDone)
		{
			//BehaviorManager->MakeAgentAct(LimbParents[limb]);
			leftovers.Add(parent);
			return leftovers;
		}
		else
			return leftovers;
	}
	//If queue is working and has trees left
	else if (qPtr->trees.Num() > 0)
	{
		int32 ix = qPtr->trees[0];
		FVector tLoc = GameMode->MoriManager->GetWorldLocByIndex(ix);
		
		//Next tree too far from cutter...
		float cutterTreeDistance = FVector::Distance(tLoc, LimbParents[limb]->GetActorLocation());
		if (cutterTreeDistance > LimbMaxDistance)
		{
			//If the limb is behind, move as close to the tree as possible and then try again to make re-orientation useful
			float limbTreeDistance = FVector::Distance(tLoc, limb->GetActorLocation());
			if (limbTreeDistance > cutterTreeDistance)
			{
				FPlanStack plan;
				plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_GetNextTree));
				plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_GoToLoc, FTransform(tLoc), cutterTreeDistance - LimbMaxDistance));
				BehaviorManager->SubmitAgentPlan(limb, plan);
				leftovers.Add(limb);
				return leftovers;
			}
			
			//If it's ahead, simply stopped as in stretched
			AMech_Cutter* parent = LimbParents[limb];
			bool bAllDone = BehaviorManager->GetCutterManager()->StopAndCheckDone(parent, limb, true);
			if (bAllDone)
			{
				leftovers.Add(parent);
				return leftovers;
			}
			else
				return leftovers;
		}
			
		//Next tree is good...
		//increment next tree
		qPtr->CurrentTree = ix;
		qPtr->TreeLoc = tLoc;
		qPtr->trees.RemoveAt(0);
		
		//generate instructions
		FPlanStack plan = BehaviorManager->PlanData->GenPlan_GoAndGrindTree(tLoc, 1000);
		BehaviorManager->SubmitAgentPlan(limb, plan);
		leftovers.Add(limb);
		return leftovers;
	}
	UE_LOG(LogTemp, Warning, TEXT("Limb stuck"));
	return leftovers;
}

bool ULimbsManager::GrindTree(FPlanAction action, AMech_Cutter_Limb* limb)
{
	bool bSuccess = TryStartGrinding(limb);

	if (!bSuccess)
		return true;
				
	return false;
}


/*Related Functions*/

//------------------------------------------------
//------------------------------------------------



bool ULimbsManager::TryStartGrinding(AMech_Cutter_Limb* cut)
{
	FCutterQueue* qPtr = LimbQueues.Find(cut);

	if (!qPtr) return false;

	//Meshify tree
	int32 type = 0;
	float age = 0;
	//Remove old HISM tree
	bool bSuccess = GameMode->MoriManager->KillTree(qPtr->CurrentTree, type, age);

	if (type == -1 || !bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Cut a bad tree!"))
		//qPtr->trees.Empty();
		return false;
	}

	//Set spawning parameters and create tree mesh
	FActorSpawnParameters spawnParams;
	FVector loc = qPtr->TreeLoc;
	FRotator rot = FRotator(0,0,0);
	ACuttingTree* TreeMesh = Cast<ACuttingTree>(GetWorld()->SpawnActor(TreeBP, &loc, &rot, spawnParams));

	//Get tree data
	const auto treeData = TreePropData->GetItem(type);
	const float r1 = treeData->MaxRadius * age;
	const float Height = treeData->MaxHeight * age;
	const float Taper = treeData->Taper;
	const float BranchMod = treeData->BranchModifier * age;

	const float r2 = r1 * Taper;
	//Volume is 1/3pi * r1sq+r1r2+r2sq * height, multiply by density for mass, times branch modifier for extra mass
	const float TotalMassKG = 1.05 * ((r1 * r1) + (r1 * r2) + (r2 * r2)) * Height * treeData->Density_KG_m * BranchMod;

	//Insatntiate tree changes
	TreeMesh->InstantiateMesh(type, Height);

	//Create Grinding Entry
	GrindingTrees.Cutters.Add(cut);
	GrindingTrees.LastSent.Add(0);
	GrindingTrees.Mesh.Add(TreeMesh);
	GrindingTrees.TreeLoc.Add(qPtr->TreeLoc);
	GrindingTrees.Height.Add(Height);
	GrindingTrees.KG_Total.Add(TotalMassKG);
	GrindingTrees.KG_Remaining.Add(TotalMassKG);
	GrindingTrees.SectionAmount.Add(4);
	GrindingTrees.HiddenSections.Add(0);

	if (bUltraFastCutting)
	{
		for (int32 tree : qPtr->trees)
		{
			//Meshify tree
			int32 treeType = 0;
			float treeAge = 0;
			//Remove old HISM tree
			GameMode->MoriManager->KillTree(tree, treeType, treeAge);
		}
		qPtr->trees.Empty();
	
		//return false;
	}

	return true;
}

void ULimbsManager::RemoveGrindingTreeAtIndex(int32 ix)
{
	GrindingTrees.Cutters.RemoveAtSwap(ix);
	GrindingTrees.Mesh[ix]->Destroy();
	GrindingTrees.Mesh.RemoveAtSwap(ix);
	GrindingTrees.TreeLoc.RemoveAtSwap(ix);
	GrindingTrees.Height.RemoveAtSwap(ix);
	GrindingTrees.KG_Total.RemoveAtSwap(ix);
	GrindingTrees.KG_Remaining.RemoveAtSwap(ix);
	GrindingTrees.SectionAmount.RemoveAtSwap(ix);
	GrindingTrees.HiddenSections.RemoveAtSwap(ix);
}

/*Update Functions*/

//------------------------------------------------
//------------------------------------------------

void ULimbsManager::UpdateFollowingLimbs(float DeltaTime)
{
	LimbAnchorTimer += DeltaTime;
	if (LimbAnchorTimer > LimbAnchorUpdateTime)
	{
		//For each cutter...
		for (auto& Elem : FollowingLimbs)
		{
			//Gen path to given scene location
			// AgentPlans[Elem.Key].PlanStack.Empty();
			// AgentPlans[Elem.Key].PlanStack.Add(PlanData->GenAction(AT_GoToObj, 1000, Elem.Value));
			
			GameMode->AgentNavManager->InitiatePath(Elem.Key, Elem.Value->GetComponentLocation(), Elem.Key->GetClass(),false);
		}
		LimbAnchorTimer = 0;
	}
}


void ULimbsManager::UpdateGrindingLimbs(float DeltaTime)
{
	TArray<AMech_Cutter_Limb*> removeThis;
	
	//Update grinding trees
	for (int32 i = 0; i < GrindingTrees.Cutters.Num(); i++)
	{
		AMech_Cutter_Limb* limb = GrindingTrees.Cutters[i];
		AMech_Cutter* parent = LimbParents[limb];
		
		//Get amount removed
		float deltaKG = GrindRate_KGperSecond * DeltaTime;
		if (GrindingTrees.KG_Remaining[i] - deltaKG <= 0)
		{
			deltaKG = GrindingTrees.KG_Remaining[i];
			removeThis.Add(limb);
		}
		GrindingTrees.KG_Remaining[i] -= deltaKG;

		//Dust flies out or something if no connection
		if (!parent)
		{
			UE_LOG(LogTemp, Warning, TEXT("grinding wasted!"));
		}
		//Else send to parent via connection
		else
		{
			BehaviorManager->GetCutterManager()->SendChips(LimbParents[limb], deltaKG, GrindingTrees.LastSent[i]);
		}

		const float remainingPercent = GrindingTrees.KG_Remaining[i]/GrindingTrees.KG_Total[i];
		//No need to do calculations if destroyed next frame
		if (remainingPercent == 0)
		{
			continue;
		}
		
		//Calculate mesh sections to hide
		const int32 toHide = static_cast<int32>((1- remainingPercent) * GrindingTrees.SectionAmount[i]);
		if (toHide > GrindingTrees.HiddenSections[i])
		{
			//Remove a mesh section until it's good
			int32 currentHid = GrindingTrees.HiddenSections[i];
			for (int32 j = 0; j < toHide - currentHid; j++)
			{
				GrindingTrees.Mesh[i]->RemoveMeshSection();
				GrindingTrees.HiddenSections[i]++;
			}
		}
		
		//Move tree down
		GrindingTrees.TreeLoc[i] = GrindingTrees.TreeLoc[i] + FVector(0,0,-remainingPercent * GrindingTrees.Height[i]);
		GrindingTrees.Mesh[i]->SetActorLocation(GrindingTrees.TreeLoc[i]);
	}

	//Each finished tree should move cutter to done grinding, remove grind tree
	for (AMech_Cutter_Limb* limb : removeThis)
	{
		int32 treeIx = GrindingTrees.Cutters.IndexOfByKey(limb);
		RemoveGrindingTreeAtIndex(treeIx);
		BehaviorManager->MakeAgentAct(limb);
	}
	removeThis.Empty();
}