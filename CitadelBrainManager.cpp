// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/CitadelBrainManager.h"

#include "AABB.h"
#include "DisharmonyGameMode.h"
#include "DrawDebugHelpers.h"
#include "AgentsBehaviorManager.h"
#include "MoriManager.h"
#include "PipelineManager.h"
#include "TransportManager.h"
#include "WombManager.h"
#include "Managers/Mechs/CuttersManagerAgain.h"
#include "Mechs/LimbsManager.h"
#include "Mechs/Mech_Cutter.h"
#include "Mechs/Mech_Weaver.h"
#include "Mechs/WeaverManager.h"

// Sets default values
ACitadelBrainManager::ACitadelBrainManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ACitadelBrainManager::InitializeSystem()
{
	//Get starting cell
	int32 ix = GameMode->SiteManager->StartingCell;

	//Add to finished sites
	FinishedSites.Add(ix);
	FNeighborList neighborList;
	//FinishedNeighbors.Add(ix, neighborList);

	//Load up neighbors to frontier
	AddNeighborSitesToFrontier(ix);
}

// Called every frame
void ACitadelBrainManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bWork)
		return;

	RunningTime += DeltaTime;

	UpdateSiteDemand();	

	TArray<AMech_Cutter*> Removed;
	UpdateJoblessCutters(Removed);


	UpdateUnitDemand();

	UpdateForlornPioneers(DeltaTime);
	
	
	
	//If so steep, expand distribution
	//Add weaver demand to meet expansion	
	
	UpdateWeaverCreationInstructions();

	//UpdateSitePreppingWeavers();
	
}


//
// int32 ACitadelBrainManager::GetClosestFinishedSiteCell(int32 x, int32 y)
// {
// 	AdjustCellCoordsToNearestSite(x, y);
// 	int32 ix = y * PGridSize + x;
// 	
// 	if (!SiteIDMap.Contains(ix))
// 		return -1;
// 	
// 	if (!FinishedNeighbors.Contains(ix))
// 	{
// 		FVector baseLoc = SiteIDMap[ix].WorldLocation;
// 		float lowestDist = 999999;
// 		int32 bestIx = -1;
// 		//Check adjacent if this one isn't finished
// 		for (int32 i = -1; i <=1; i++)
// 		{
// 			for (int32 j = -1; j <=1; j++)
// 			{
// 				if (i == 0 && j == 0) continue;
//
// 				//Closest neighbor is chosen
// 				int32 curIx = (y+j) * PGridSize + (x+i);
// 				if (FinishedNeighbors.Contains(curIx) && SiteIDMap.Contains(curIx))
// 				{
// 					auto site = SiteIDMap[curIx];
// 					float curDist = FVector::Dist(site.WorldLocation, baseLoc);
// 					if (curDist < lowestDist)
// 					{
// 						lowestDist = curDist;
// 						bestIx = curIx;
// 					}
// 				}
// 			}
// 		}
//
// 		return bestIx;
// 	}
// 	else return -1;
// }
//
// TArray<FVector> ACitadelBrainManager::AStarFinishedSites(int32 startIx, int32 endIx)
// {
// 	//Open and closed data
// 	TArray<int32> open;
// 	TArray<int32> open_f;
// 	
// 	TSet<int32> Closed;
//
// 	TMap<int32, int32> Parents;
// 	Parents.Add(startIx, -1);
// 	TMap<int32, int32> g;
//
// 	open.Add(startIx);
// 	open_f.Add(0);
// 	
// 	//While start of Open queue is not the goal
// 	while (open[0] != endIx)
// 	{
// 		//Get current
// 		auto current = open[0];
// 		open.RemoveAt(0);
// 		open_f.RemoveAt(0);
//
// 		int32 cost;
// 		//Cost is g(current)... 
// 		if (!g.Contains(current))
// 			cost = 0;
// 		else cost = g[current];
//
// 		// ... + the cost of next (always 1)
// 		cost += 1;
//
// 		
// 		TArray<int32> neighbors;
// 		if (FinishedNeighbors.Contains(current))
// 			neighbors = FinishedNeighbors[current].Neighbors;
//
// 		//for each neighbor of current
// 		for (int32 neighbor : neighbors)
// 		{
// 			//If neighbor in open and costs less than g(neighbor)
// 			if (open.Contains(neighbor))
// 			{
// 				//remove neighbor from open, new path is better
// 				int32 ix = open.IndexOfByKey(neighbor);
// 				open.RemoveAt(ix);
// 				open_f.RemoveAt(ix);
// 			}
// 			//If neighbor in closed and cost less than g(neighbor)
// 			if (Closed.Contains(neighbor) && g.Contains(neighbor) && cost < g[neighbor])
// 			{
// 				//remove neighbor from closed
// 				Closed.Remove(neighbor);
// 			}
// 			//if neighbor not in open and not in closed
// 			if (!open.Contains(neighbor) && !Closed.Contains(neighbor))
// 			{
// 				//set g(neighbor) to cost
// 				g.Add(neighbor, cost);
//
// 				//Get heuristic
// 				int32 heuristic = WaypointHeuristic(current, neighbor);
// 				
// 				//add neighbor to open with f of g + h
// 				int32 f = cost + heuristic;
// 				//Insert priority-wise
// 				for (int32 i = 0; i < open_f.Num(); i++)
// 				{
// 					if (open_f[i] >= f)
// 					{
// 						open_f.Insert(f, i);
// 						open.Insert(neighbor, i);
// 					}
// 				}
// 				
// 				//set neighbor's parent to current
// 				Parents.Add(neighbor, current);
// 			}
// 				
// 		}
// 	}
//
// 	//Reconstruct path
// 	TArray<FVector> path;
// 	path.Add(SiteIDMap[endIx].Spires[0]->GetActorLocation());
// 	int32 current = endIx;
// 	while (current != -1)
// 	{
// 		path.Insert(SiteIDMap[Parents[current]].WorldLocation, 0);
// 		current = Parents[current];
// 	}
// 	//return path
// 	return path;
// }
//
// int32 ACitadelBrainManager::WaypointHeuristic(int32 curIx, int32 neighIx)
// {
// 	//Manhattan distance = dX + dY
// 	auto curSite = SiteIDMap[curIx];
// 	auto neighSite = SiteIDMap[neighIx];
// 	return FMath::Abs(curSite.CenterX - neighSite.CenterX) + FMath::Abs(curSite.CenterY - neighSite.CenterY);
// }

void ACitadelBrainManager::UpdateJoblessCutters(TArray<AMech_Cutter*>& Removed)
{
	auto JoblessCutters = GameMode->BehaviorManager->GetJoblessCutters();
	
	//For every jobless cutter [new or finished] (move from manager)
	for (AMech_Cutter* cut : *JoblessCutters)
	{
		//Move to demanded places in this order: Active sites, active pioneers, pending sites
		if (Demand_C_Site.Num() > 0)
		{
			FVector loc = cut->GetActorLocation();
			int32 x = 0;
			int32 y = 0;
			GameMode->SiteManager->WorldToSiteCoords(loc, x, y);

			//Get closets demand sites
			auto indices = GetClosestDemandSites(x, y, false, 1);
			int32 sIx = indices[0];
			FSite* site = GameMode->SiteManager->GetSite(sIx);

			//Set to pos 1 if empty
			if (!site->Cutters.Contains(cut))
				site->Cutters.Add(cut);
			//Add to site cutter
			SiteCutters.Add(cut, sIx);
			//Remove from demanded sites
			Demand_C_Site.RemoveSingle(sIx);

			auto spires = site->Spires;
			
			//Start the site job
			FPlanStack plan =
				GameMode->BehaviorManager->GetCutterManager()
				->GenerateSiteJob(cut, spires[0], site->CenterX, site->CenterY,
					site->WorldLocation, AT_GetSiteWork);
			GameMode->BehaviorManager->SubmitAgentPlan(cut, plan);
			GameMode->BehaviorManager->MakeAgentAct(cut);
			

			Removed.Add(cut);
			
			continue;
		}
		else if (Demand_C_Pioneer.Num() > 0)
		{
			FVector loc = cut->GetActorLocation();
			int32 pioneerX = 0;
			int32 pioneerY = 0;
			GameMode->SiteManager->WorldToSiteCoords(loc, pioneerX, pioneerY);

			auto indices = GetClosestDemandSites(pioneerX, pioneerY, true, 1);
			int32 sIx = indices[0];
			FSite* closestSite = GameMode->SiteManager->GetSite(sIx);
			
			//Register as pioneering cutter
			PioneerCutterEnds.Add(cut, sIx);
			PendingSites.Add(sIx);
			Demand_C_Pioneer.RemoveSingle(sIx);

			//Get previous site to start from
			int32 startIx = Demand_C_PioneerPrev[sIx];
			Demand_C_PioneerPrev.Remove(sIx);


			FSite* startSite = GameMode->SiteManager->GetSite(startIx);
			auto spires = startSite->Spires;
			auto startX = startSite->CenterX;
			auto startY = startSite->CenterY;

			//Try to get pioneer to work on site
			bool bSiteWorks = GameMode->BehaviorManager->GetCutterManager()->TryStartPioneerJob(cut, spires, startX, startY, closestSite->WorldLocation);
			
			if (bSiteWorks)
			{
				PioneerCutterStarts.Add(cut, startIx);
			}
			else
			{
				// //Try and find adjacent
				// int32 newIx = TryRetargetPioneerStart(startIx, sIx, pioneerX, pioneerY);
				//
				// if (newIx != -1)
				// {
				// 	PioneerCutterStarts.Add(cut, newIx);
				// 	UE_LOG(LogTemp, Warning, TEXT("Found a new site for this forlorn pioneer."));
				// 	//Just start the job with new index
				// 	if (!GameMode->BehaviorManager->TryStartPioneerJob(cut, site->Spires, site->CenterX, site->CenterY, site->WorldLocation, AT_GetPioneerWork))
				// 	{
				// 		UE_LOG(LogTemp, Error, TEXT("THis isn't supposed to happen!"))
				// 	}
				// }
				// else
				// {
					//otherwise add to retry bin
					//UE_LOG(LogTemp, Warning, TEXT("No, this forlorn pioneer needs to retry later."));
					FPioneerRetryData retryData;
					retryData.time = RetryPioneerTimer;
					retryData.startIx = startIx;
					retryData.targetIx = sIx;
					retryData.pioneerX = pioneerX;
					retryData.pioneerY = pioneerY;
					RetryForlornPioneers.Add(cut, retryData);
				//}
			}

			Removed.Add(cut);
			continue;
		}
		//No demands
		break;
	}
	for (AMech_Cutter* cut : Removed)
	{
		JoblessCutters->Remove(cut);
	}
	Removed.Empty();
}

void ACitadelBrainManager::UpdateSiteDemand()
{
	//Check balance of pending pioneers vs active
	//Not enough pending for the time (if it doubles every x minutes or cycles?) : add demands
	int32 ExpectedSites = FMath::CeilToInt(FMath::Pow(RunningTime/60, 1.8) / 10);
	for (int32 i = 0; i < ExpectedSites - (ActiveSites.Num() + PendingSites.Num()); i++)
	{
		if (Frontier.SiteIx.Num() == 0)
		{
			break;
		}
		//Calculate next site
		//UE_LOG(LogTemp, Warning, TEXT("Adding Site to Demand"))
		int32 sIx = Frontier.SiteIx.Pop();
		int32 prevIx = Frontier.PrevSiteID.Pop();
		Frontier.EstimatedMass.Pop();
		//Add to demanded sites
		Demand_C_Pioneer.Add(sIx);
		Demand_C_PioneerPrev.Add(sIx, prevIx);
	}
}

void ACitadelBrainManager::UpdateUnitDemand()
{
	//Assess existing Cutters and demanded to get difference
	int32 totalCutters = SiteCutters.Num() + PioneerCutterEnds.Num() + GameMode->BehaviorManager->GetJoblessCutters()->Num() + FormingCutters;
	int32 demandCutters = Demand_C_PioneerPrev.Num() + Demand_C_Site.Num();//(ActiveSites.Num() + PendingSites.Num() + Frontier.StartSite.Num()) * CuttersPerSite;

	//Add difference to demanded
	if (demandCutters - totalCutters > 0)
	{
		Demand_U_Cutter = (demandCutters - totalCutters);
	}
	//Assess W demand and distribution around map
}

void ACitadelBrainManager::UpdateWeaverCreationInstructions()
{

	TArray<AMech_Weaver*> weavers;
	TArray<FVector> wombLocs;
	TArray<FString> names;
	TArray<FPlanStack> startInsts;
	TArray<int32> siteIndices;
	//Get ready weavers with at least one available womb
	TMultiMap<int32, TTuple<AMech_Weaver*, FVector>> SiteReadyWeaverMap = GameMode->WombManager->GetIdleWombWeavers();

	//Get unit demands in order from this
 	int32 num = FMath::Min(SiteReadyWeaverMap.Num(),
									Demand_W_ProductionSite.Num() + Demand_U_Cutter);

	//Go build site if demand
	if (Demand_W_ProductionSite.Num() > 0)
	{
		TArray<FSiteDemandInfo> Leftovers;
		//First priority is womb mitosis
		for (int32 i = 0; i < Demand_W_ProductionSite.Num(); i++)
		{
			FSiteDemandInfo siteDemandInfo = Demand_W_ProductionSite[i];

			
			//If the starting site is available,
			if (num > 0 && SiteReadyWeaverMap.Contains(siteDemandInfo.StartSite))
			{
				//Mark the site weaver as busy
				auto find = SiteReadyWeaverMap.Find(siteDemandInfo.StartSite);
				SiteReadyWeaverMap.RemoveSingle(siteDemandInfo.StartSite, *find);
				GameMode->WombManager->RemoveSiteWeaver(find->Key, siteDemandInfo.StartSite);
				
				//Generate plan for actually making it
				FPlanStack grandRefineryPlan = GameMode->BehaviorManager->GetWeaverManager()->GenerateRefineryPlan(
					find->Key,
					siteDemandInfo.EndLoc,
					siteDemandInfo.StartSite,
					siteDemandInfo.EndSite);

				//Seed stuff
				FPlanStack seedPlan;
				seedPlan = GameMode->BehaviorManager->PlanData->GenPlan_GoSeedWomb(find->Value + FVector(0,0,2400), 1000);
				GameMode->BehaviorManager->GetWeaverManager()->AddSeedInstruction(find->Key, find->Value, "Weaver");
				
				//Combine and submit plans
				grandRefineryPlan.PlanStack.Append(seedPlan.PlanStack);
				GameMode->BehaviorManager->SubmitAgentPlan(find->Key, grandRefineryPlan);
				GameMode->BehaviorManager->MakeAgentAct(find->Key);

				
				// //Decrement ready weavers
				// num--;
				//Create seed instruction
				// auto find = SiteReadyWeaverMap.Find(siteDemandInfo.StartSite);
				// weavers.Add(find->Key);
				// wombLocs.Add(find->Value);
				// names.Add("Weaver");
				// siteIndices.Add(siteDemandInfo.StartSite);
				// startInsts.Add(PlanData->GenAction(AT_BecomeIdleAttendant));
				// SiteReadyWeaverMap.RemoveSingle(siteDemandInfo.StartSite, *find);
				//
				// //Wake up, do all the cool stuff
				// FPlanStack stacc;
				// stacc.PlanStack.Add(PlanData->GenAction(AT_IncrementProductionPlan));
				// startInsts.Add(stacc);
				//
				// FProductionSitePlan sitePlan;
				// sitePlan.DemandInfo = siteDemandInfo;
				//
				// int32 planID = ProdPlansCreated;
				// ProdPlansCreated++;
				// ProductionSitePlan.Add(planID, sitePlan);
				// WombWithSitePlan 
				//
		
		
				//--
		
				// //Use the vector of the pipeline that just finished and extend/rotate it to find the womb pool site
				// FVector wombPumpGuess = FVector::ZeroVector;
				// GameMode->BehaviorManager->ExtendAndRotateVector(wombPumpGuess, pPack.StartPumpLoc, pPack.EndPumpLoc, 10000, -70);
				//
				// //Generate water pipe data
				// TArray<FVector> wPoints = GameMode->PipelineManager->GenPipeSectionPoints(pPack.EndPumpLoc, wombPumpGuess);
				// FPipeBuildInfo waterLayInfo;
				// GameMode->PipelineManager->GenPipeBuildInfo(waterLayInfo, wPoints, false);
				//
				// //Generate womb  package 
				// FPipelineLayPackage wombPack;
				// wombPack.StartSiteIx = pPack.EndPumpIx;
				// wombPack.StartPumpLoc = pPack.EndPumpLoc;
				// wombPack.EndSite = pPack.EndSite;
				// wombPack.EndPumpLoc = wombPumpGuess;
				// wombPack.EndPumpType = PT_WombWater;
				// wombPack.bWater = true;
				// wombPack.waterLay = waterLayInfo;
		
				//--
		
		
				
				//Submit to citadel
				//SubmitSiteWorkWeaver(siteP, weav);
			}
			else
			{
				Leftovers.Add(siteDemandInfo);
			}
		}
		Demand_W_ProductionSite = Leftovers;
	}


	//Next priority is cutters
	TArray<int32> options;
	int32 added = 0;
	SiteReadyWeaverMap.GenerateKeyArray(options);
	for (int32 i = 0; i < Demand_U_Cutter; i++)
	{
		if (num > 0 && options.Num() > 0)
		{
			auto find = SiteReadyWeaverMap.Find(options[0]);
			//Naive approach for now and fill in any order
			weavers.Add(find->Key);
			wombLocs.Add(find->Value);
			names.Add("Cutter");
			startInsts.Add(FPlanStack());
			siteIndices.Add(options[0]);

			//Remove weaver option
			SiteReadyWeaverMap.RemoveSingle(options[0], *find);
			options.RemoveAt(0);

			//Change cutter production #s
			FormingCutters++;
			added++;
			num--;
		}
	}
	Demand_U_Cutter -= added;
	GameMode->BehaviorManager->GetWeaverManager()->EnactBuildInstructions(weavers, wombLocs, names, startInsts, siteIndices);
}

void ACitadelBrainManager::UpdateForlornPioneers(float DeltaTime)
{
	TArray<AMech_Cutter*> cutters;
	RetryForlornPioneers.GenerateKeyArray(cutters);
	for (auto cut : cutters)
	{
		//Update time
		FPioneerRetryData* retryData = &RetryForlornPioneers[cut];
		retryData->time += DeltaTime;

		//If time to retry
		if (retryData->time > RetryPioneerTimer)
		{
			//Find the closest adjacent finished sites
			TArray<int32> closestSites = GameMode->SiteManager->GetAdjacentSitesExcludingOne(retryData->startIx, retryData->targetIx, retryData->pioneerX, retryData->pioneerY);

			//Loop through and try to find one that will work for pioneering
			int32 foundIx = -1;
			for (int32 ix : closestSites)
			{
				//Not finished don't consider
				if (!FinishedSites.Contains(ix)) continue;

				//Check if start pioneer job works
				FSite* site = GameMode->SiteManager->GetSite(ix);
				FVector targetLoc = GameMode->SiteManager->GetSite(retryData->targetIx)->WorldLocation;
				if (GameMode->BehaviorManager->GetCutterManager()->TryStartPioneerJob(cut, site->Spires, site->CenterX, site->CenterY, targetLoc))
				{
					foundIx = ix;
					break;
				}
			}

			//Remove from retry if successful
			if (foundIx != -1)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Found a new site for this %s pioneer."), *cut->GetName());	
				PioneerCutterStarts.Add(cut, foundIx);
				RetryForlornPioneers.Remove(cut);
			}
			else
			{
				//none of them work, reset timer
				retryData->time = 0;
				//UE_LOG(LogTemp, Warning, TEXT("Retry pioneer didn't work for now: %s"), *cut->GetName());
			}
		}
	}
}



/**
 * @brief Takes a cutter's site location and finds the closest in-demand site
 * @param x : site X of cutter
 * @param y : site Y of cutter
 * @param bIsPioneer : whether to search pioneer demand or regular
 * @return : index of closest in-demand site
 */
TArray<int32> ACitadelBrainManager::GetClosestDemandSites(int32 x, int32 y, bool bIsPioneer, int32 considerNum)
{
	TArray<int32> closeCells;
	TArray<float> cellDists;

	//Relevant demand based on pioneer or not
	TArray<int32>* relevantDemand;
	if (bIsPioneer)
		relevantDemand = &Demand_C_Pioneer;
	else relevantDemand = &Demand_C_Site;

	
	//Consider either the first X plumpest sites or amount of demanded pioneer sites there are, whichever is lower
	int32 length = FMath::Min(6, relevantDemand->Num());

	//For the length...
	for (int32 i = 0; i < length; i++)
	{
		//Get site x and y of site
		FSite* site = GameMode->SiteManager->GetSite( (*relevantDemand)[i]);
		int32 sX = site->CenterX;
		int32 sY = site->CenterY;
		//Get simple distance from cutter site
		//float distance = FMath::Sqrt(FMath::Square(sX - x) + FMath::Square(sY - y));
		float distance = FMath::Max(FMath::Abs(sX - x), FMath::Abs(sY - y));
		
		if (closeCells.Num() == 0)
		{
			closeCells.Add((*relevantDemand)[i]);
			cellDists.Add(distance);
		}
		else
		{
			int32 checkLen =  FMath::Min(considerNum, closeCells.Num());
			for (int32 j = 0; j < checkLen; j++)
			{
				//If it's the closest, record that
				if (distance < cellDists[j])
				{
					//Insert at this
					closeCells.Insert((*relevantDemand)[i], j);
					cellDists.Insert(distance, j);

					if (closeCells.Num() == considerNum + 1)
					{
						closeCells.RemoveAt(considerNum);
						cellDists.RemoveAt(considerNum);
					}
				}
			}
		}
	}	
	
	return closeCells;
}


/**
 * @brief This method adds every un-claimed neighbor of this site to the frontier
 * @param cX : center X of the site
 * @param cY : center Y of the site
 * 
 */
int32 ACitadelBrainManager::AddNeighborSitesToFrontier(int32 sIx)
{
	int32 bigNeighborsAdded = 0;

	//Get un-added site neighbor indices/previous
	TArray<int32> indices;
	TArray<int32> prevIndices;
	GameMode->SiteManager->InitializeSiteNeighbors(sIx, indices, prevIndices);

	//for each, add to frontier
	for (int32 i = 0; i < indices.Num(); i++)
	{
		//Adding the site will input get the estimated mass and stuff
		int32 estMass;
		AddSiteToFrontier(indices[i], prevIndices[i], estMass);

		if (estMass > BigNeighborTreeCount)
			bigNeighborsAdded++;
	}

	return bigNeighborsAdded;
}


/**
 * @brief This method adds a center cell for a new site to the frontier, recording its previous site
 * @param x : site space x
 * @param y : site space y
 * @param prevIx : index of site this branched from
 */
void ACitadelBrainManager::AddSiteToFrontier(int32 sIx, int32 prevIx, int32 &estMass)
{
	//Find estimated mass
	int32 x;
	int32 y;
	GameMode->SiteManager->GetCenterXYFromSiteIX(sIx, x, y);
	TArray<int32> CellTrees = GameMode->MoriManager->CreateTreePatchRes(x, y, ETreePatchDirection::TPD_BottomTop, ProgressGridData->CellResolution,  ProgressGridData->CellResolution, 0);
	estMass = CellTrees.Num();
	
	//For each site in list...
	for (int32 i = 0; i < Frontier.SiteIx.Num(); i++)
	{
		//Insert and return if mass is less
		if (estMass < Frontier.EstimatedMass[i])
		{
			Frontier.SiteIx.Insert(sIx, i);
			Frontier.EstimatedMass.Insert(estMass, i);
			Frontier.PrevSiteID.Insert(prevIx, i);
			//UE_LOG(LogTemp, Warning, TEXT("Added neighbor with %d trees"), estMass)
			return;
		}
	}

	//Insert at end if get this far
	Frontier.SiteIx.Push(sIx);
	Frontier.EstimatedMass.Push(estMass);
	Frontier.PrevSiteID.Push(prevIx);
	//UE_LOG(LogTemp, Warning, TEXT("Added neighbor with %d trees"), estMass)
}

/**
 * @brief This method finds the closest cells to the start and end, gets all the waypoints between it, and returns that path
 * @param start : location of unit looking for a path
 * @param end : end point of path
 * @param simplePathRange : range under which the path doesn't look for waypoints
 * @return : a set of waypoints leading to the end
 */
// TArray<FVector> ACitadelBrainManager::GetWaypointPath(FVector start, FVector end, float simplePathRange)
// {
// 	if (FVector::Dist(start, end) < simplePathRange)
// 	{
// 		TArray<FVector> dumbPath;
// 		dumbPath.Add(end);
// 		return dumbPath;
// 	}
// 	
// 	//Get end closest cell
// 	int32 endX = end.X;
// 	int32 endY = end.Y;
// 	int32 endIx;
// 	GameMode->MoriManager->WorldToMapCoords(endX, endY, endIx);
// 	endX/= ProgressGridData->CellResolution;
// 	endY/= ProgressGridData->CellResolution;
// 	endIx = GetClosestFinishedSiteCell(endX, endY);
//
// 	
// 	//Get start closest cell
// 	int32 startX = start.X;
// 	int32 startY = start.Y;
// 	int32 startIx;
// 	GameMode->MoriManager->WorldToMapCoords(startX, startY, startIx);
// 	startX/= ProgressGridData->CellResolution;
// 	startY/= ProgressGridData->CellResolution;
// 	startIx = GetClosestFinishedSiteCell(startX, startY);
// 	
// 	//ensure both in finished category (redundant now, ensures connected)
// 	if (startIx == -1 && endIx != -1)
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("Can't find start for waypoint, going to end waypoint then dest"))
// 		TArray<FVector> path;
// 		path.Add(SiteIDMap[endIx].WorldLocation);
// 		path.Add(end);
// 		return path;
// 	}
// 	if (startIx != -1 && endIx == -1)
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("Can't find end for waypoint, going to start and then dest"))
// 		TArray<FVector> path;
// 		path.Add(SiteIDMap[startIx].WorldLocation);
// 		path.Add(end);
// 		return path;
// 	}
// 	if (startIx == -1 && endIx == -1)
// 	{
// 		//UE_LOG(LogTemp, Warning, TEXT("Can't find any close cells, doing dumb path."))
// 		TArray<FVector> path;
// 		path.Add(end);
// 		return path;
// 	}
// 	
// 	//A* through the frontier to find it
// 	TArray<FVector> waypointPath = AStarFinishedSites(startIx, endIx);
// 	UE_LOG(LogTemp, Warning, TEXT("AStar making path with %d waypoint(s)."), waypointPath.Num())
// 	
// 	//Return with end attached
// 	waypointPath.Push(end);
// 	return waypointPath;
// }


void ACitadelBrainManager::MarkSiteHalfFinished(int32 ix)
{
	FSite* site = GameMode->SiteManager->GetSite(ix);
	
	int32 neighborsAdded = AddNeighborSitesToFrontier(ix);

	bool bConsiderFactory = false;
	if (site->PreviousSite != -1)
		bConsiderFactory = !site->bFactorySite;
	if (bAllSitesCanBeFactories)
		bConsiderFactory = true;
		
	//Check if this is a factory candidate by # of future big neighbors
	if (neighborsAdded >= FactoryCandidateBigNeighborThreshold && bConsiderFactory)
	{
		GameMode->TransportManager->SetSpireAsSpecial(site->Spires[0]);
		
		UE_LOG(LogTemp, Warning, TEXT("Site at %s needs a factory!"), *site->WorldLocation.ToString())
		DrawDebugSphere(GetWorld(), site->WorldLocation, 4000, 8, FColor::Orange, true);
		site->bFactorySite = true;

		GameMode->WombManager->AddSiteWombSet(ix, FWombSet());
		
		//Generate demand
		FSiteDemandInfo wDem;
			
		//Get start location by previous site with womb
		int32 wombStartIx = GameMode->SiteManager->GetPreviousFactorySite(site->PreviousSite);
		int32 waterPump = GameMode->WombManager->GetSiteWombs(wombStartIx)->WaterPumpIx;
		wDem.StartSite = wombStartIx;
		wDem.EndSite = ix;
		//wDem.StartPump = waterPump;
		wDem.StartLoc = GameMode->PipelineManager->GetWaterPumpLoc(waterPump);
			
		//Get end location by getting a point in a direction and raycasting I guess
		FVector oppositeDir = GameMode->SiteManager->GetSite(site->PreviousSite)->WorldLocation - site->WorldLocation;
		oppositeDir.Z = 0;
		oppositeDir.Normalize();
		oppositeDir *= 5000;
		FVector guessPoint = site->WorldLocation + oppositeDir.RotateAngleAxis(70, FVector(0,0,1));
			
		//Raycast down to terrain
		FHitResult hit;
		//FCollisionQueryParams params;
		FVector adjust = FVector(0,0,2000);
		bool bSuccess = GetWorld()->LineTraceSingleByChannel(hit, guessPoint + adjust, guessPoint - adjust, ECollisionChannel::ECC_Visibility);
		//Replace if successful
		if (bSuccess)
		{
			guessPoint = hit.Location;
			DrawDebugLine(GetWorld(), guessPoint + adjust, hit.Location, FColor::Green, true, -1, 0, 100);
		}
		else
		{
			//Draw debug line if not
			DrawDebugLine(GetWorld(), guessPoint + adjust, guessPoint - adjust, FColor::Red, true, -1, 0, 100);
		}
		wDem.EndLoc = guessPoint;
			
		//Add demand to list
		Demand_W_ProductionSite.Add(wDem);
	}
}

void ACitadelBrainManager::MarkSiteFullyFinished(int32 ix)
{
	ActiveSites.Remove(ix);
	FinishedSites.Add(ix);
}

void ACitadelBrainManager::MarkPioneeringFinished(AMech_Cutter* cut, AActor* attachedSpire, int32 &centerX, int32 &centerY, FVector &worldLoc)
{
	int32 sIx = PioneerCutterEnds[cut];
	FSite* site = GameMode->SiteManager->GetSite(sIx);

	//Site is now active
	PendingSites.Remove(sIx);
	ActiveSites.Add(sIx);

	//Generate demand for site 
	for (int32 i = 0; i < CuttersPerSite-1; i++)
	{
		Demand_C_Site.Add(sIx);
	}

	//Remove relevant pioneer info
	site->PreviousSite = PioneerCutterStarts[cut];
	PioneerCutterEnds.Remove(cut);
	PioneerCutterStarts.Remove(cut);

	//Site spire set to whichever the pioneer is attached to now
	site->Spires.Add(attachedSpire);

	
	//////Pepare pioneer to attend site
	//Add to site map as cutter
	site->Cutters.Add(cut);
	//Add to site cutters
	SiteCutters.Add(cut, sIx);

	//returny stuff
	centerX = site->CenterX;
	centerY = site->CenterY;
	worldLoc = site->WorldLocation;
}

void ACitadelBrainManager::MarkSiteCutterFinished(AMech_Cutter* cut)
{
	//Relevant site info
	int32 ix = SiteCutters[cut];
	FSite* site = GameMode->SiteManager->GetSite(ix);

	//if this is the last cutter working on it, we mark the site finished
	if (site->Cutters.Num() == 1)
	{
		MarkSiteFullyFinished(ix);
	}
	
	site->Cutters.Remove(cut);
	SiteCutters.Remove(cut);
}

/**
 * @brief Adds spire to site's list if raised by a cutter at a site
 * @param spire : spire to add
 * @param mech : mech adding the spire
 */
void ACitadelBrainManager::TryAddSpireToSite(AActor* spire, AMech_Cutter* mech)
{
	if (mech == nullptr) return;
	if (SiteCutters.Contains(mech))
	{
		GameMode->SiteManager->AddSpireToSite(spire, SiteCutters[mech]);
		
	}
	else if (PioneerCutterEnds.Contains(mech))
	{
		GameMode->SiteManager->AddSpireToSite(spire, PioneerCutterEnds[mech]);
	}
}

/**
 * @brief This method checks for a new cell to devour, updating the given direction and attend location if successful
 * @param cut : cutter in question
 * @param attendLoc : ends up giving the cutter where to stand (middle before the patch)
 * @param prevX : previous site X that gets updated if there's a new one
 * @param prevY : previous site Y that gets updated if there's a new one
 * @param dir : returned direction of cell based on previous visited
 * @return : returns true if there's another cell to chop, false if the site is cleared
 */
bool ACitadelBrainManager::CutterSiteHasNext(AMech_Cutter* cut, FVector& attendLoc, int32& prevX, int32& prevY, ETreePatchDirection& dir)
{
	int32 cellIx = 0;
	int32 x;
	int32 y;
	FSite* site = GameMode->SiteManager->GetSite(SiteCutters[cut]);
	//If there are still cells in the site, get the next one for this cutter and return true
	if (site->InnerCells.Num() + site->OuterCells.Num() > 0)
	{
		//Check half finished now
		if (site->InnerCells.Num() == 0 && !site->bHalfFinished)
		{
			MarkSiteHalfFinished(SiteCutters[cut]);
			site->bHalfFinished = true;
		}
		
		//Check if inner
		if (site->InnerCells.Num() > 0)
		{
			//Give first if position 1 cutter
			if (site->Cutters[0] == cut)
			{
				cellIx = site->InnerCells[0];
			}
			else
			{
				//Quick and dirty; backwards-winding cutter should get the left cell before the top left one so it can be accessed
				if (site->InnerCells.Num() >= 7)
				{
					cellIx = site->InnerCells[site->InnerCells.Num() - 2];
				}
				else
					cellIx = site->InnerCells.Last();
			}

			site->InnerCells.Remove(cellIx);
		}
		//Check if outer
		else if (site->OuterCells.Num() > 0)
		{
			//Give first if position 1 cutter
			if (site->Cutters[0] == cut)
			{
				cellIx = site->OuterCells[0];
			}
			else
			{
				cellIx = site->OuterCells.Last();
			}

			site->OuterCells.Remove(cellIx);
		}

		//Direction retrieved; 
		//x = cellIx % PGridSize;
		//y = (cellIx - x) / PGridSize;
		GameMode->SiteManager->GetCenterXYFromSiteIX(cellIx, x, y);
		
		//Cell retrieved; find direction
		int32 dX = x - prevX;
		int32 dY = y - prevY;
		if (dY > 0)
			dir = ETreePatchDirection::TPD_BottomTop;
		else if (dY < 0)
			dir = ETreePatchDirection::TPD_TopBottom;
		else if (dX > 0)
			dir = ETreePatchDirection::TPD_LeftRight;
		else if (dX < 0)
			dir = ETreePatchDirection::TPD_RightLeft;


		//Update previous
		prevX = x;
		prevY = y;

		int32 res = ProgressGridData->CellResolution;
		x *= res;
		y *= res;
		switch (dir)
		{
		case ETreePatchDirection::TPD_BottomTop:
			{
				x += res/2;
				y-= 2;
				break;
			}
		case ETreePatchDirection::TPD_LeftRight:
			{
				y += res/2;
				x -= 2;
				break;
			}
		case ETreePatchDirection::TPD_RightLeft:
			{
				y += res/2;
				x += res + 2;
				break;
			}
		case ETreePatchDirection::TPD_TopBottom:
			{
				x += res/2;
				y += res + 2;
				break;
			}
		}
		attendLoc = GameMode->MoriManager->GetWorldLocByCoords(x, y);
		
		return true;
	}
	return false;
}

/**
 * @brief This method takes the current limb and offset information to ask the mori manager for the trees in an appropriate amount of lanes
 * @param limbIx : current limb ix
 * @param limbTotal : total limbs
 * @param x : x in site space (full res / 16)
 * @param y : y in site space (full res / 16)
 * @param dir : direction of patch
 * @param trees : data to be filled out with trees
 */
void ACitadelBrainManager::GetCutterSiteWork(int32 limbIx, int32 limbTotal, int32 x, int32 y, ETreePatchDirection dir, TArray<int32>& trees)
{
	float div = ProgressGridData->CellResolution / limbTotal;

	int32 width = FMath::Floor(div);

	//If last one...
	if (limbIx == limbTotal-1)
	{
		//Set width to remainder of columns
		width = ProgressGridData->CellResolution - (width  * limbIx);
	}

	//Get tree array and return
	//UE_LOG(LogTemp, Warning, TEXT("Getting trees at PGrid x: %d y: %d"), x, y);
	trees = GameMode->MoriManager->CreateTreePatchRes(x, y, dir, ProgressGridData->CellResolution, width, width * limbIx);
}


void ACitadelBrainManager::PopulateCornerIndices(int32 dX, int32 dY, int32 cornerX, int32 cornerY, TArray<int32>& indicesX, TArray<int32>& indicesY, int32 span)
{
	//Add X row
	if (dX > 0)
	{
		for (int32 i = cornerX - span; i < cornerX; i++)
		{
			indicesX.Add(i);
			indicesY.Add(cornerY);
		}
	}
	else
	{
		for (int32 i = cornerX + span; i > cornerX; i--)
		{
			indicesX.Add(i);
			indicesY.Add(cornerY);
		}
	}
	
	//Add middle
	indicesX.Add(cornerX);
	indicesY.Add(cornerY);
	
	//Add Y
	if (dY > 0)
	{
		for (int32 i = cornerY - 1; i >= cornerY - span; i--)
		{
			indicesX.Add(cornerX);
			indicesY.Add(i);
		}
	}
	else
	{
		for (int32 i = cornerY + 1; i <= cornerY + span; i++)
		{
			indicesX.Add(cornerX);
			indicesY.Add(i);
		}
	}
}

void ACitadelBrainManager::PopulateOrthogonalIndices(int32 dX, int32 dY, int32 cornerX, int32 cornerY, TArray<int32>& indicesX, TArray<int32>& indicesY, int32 span)
{
	//get middle Ys
	if (dY == 0)
	{
		for (int32 i = cornerY; i < cornerY + span; i++)
		{
			indicesX.Add(cornerX);
			indicesY.Add(i);
		}
		

	}
	//Get middle Xs
	else if (dX == 0)
	{
		for (int32 i = cornerX; i < cornerX + span; i++)
		{
			indicesX.Add(i);
			indicesY.Add(cornerY);
		}
	}

}

void ACitadelBrainManager::GatherScrapTrees(bool diagonal, bool firstHalf, TArray<FCutterQueue>& cutterQueues, int32 dX, int32 dY, int32 span, TArray<int32> halfIxX, TArray<int32> halfIxY)
{
	TArray<int32> giveX;
	TArray<int32> giveY;
	
	int32 cut1;
	int32 cut2;
	if (firstHalf)
	{
		cut1 = 2;
		cut2 = 3;
	}
	else
	{
		cut1 = 1;
		cut2 = 0;
	}
	
	//operate
	for (int32 i = 0; i < halfIxX.Num(); i++)
	{
		//First half has length of half span
		giveX.Empty();
		giveY.Empty();
		giveX.Add(halfIxX[i]);
		giveY.Add(halfIxY[i]);

		int32 offset =0;

		if (diagonal && i %2 != 0)
		{
			offset += 1;
			
		}

		int32 length = ProgressGridData->CellResolution;
		if (diagonal)
		{
			length -= span;
			length -= i;
		}
		cutterQueues[cut1].trees.Append(GameMode->MoriManager->CreatePioneerTreeLanes(giveX, giveY, dX, dY, length / 2 + offset));

		//Second half starts after offset 
		offset += length/2;

		giveX.Empty();
		giveY.Empty();
		giveX.Add(halfIxX[i] + dX * offset);
		giveY.Add(halfIxY[i] + dY * offset);
		cutterQueues[cut2].trees.Append(GameMode->MoriManager->CreatePioneerTreeLanes(giveX, giveY, dX, dY, length / 2));
	}
}

/**
 * @brief This method finds the corner and direction to the pioneer's target site and asks the Mori Manager for the trees to fill each limb queue
 * @param cut : the pioneer cutter 
 * @param cutterQueues : the data to be filled out with target trees
 */
void ACitadelBrainManager::GetCutterPioneerWork(AMech_Cutter* cut, TArray<FCutterQueue>& cutterQueues, FVector& attendLoc)
{
	//Get direction
	int32 startIx = PioneerCutterStarts[cut];
	int32 endIx = PioneerCutterEnds[cut];

	int32 startX;// = startIx % PGridSize;
	int32 startY;// = (startIx - startX) / PGridSize;
	GameMode->SiteManager->GetCenterXYFromSiteIX(startIx, startX, startY);

	int32 endX;// = endIx % PGridSize;
	int32 endY;// = (endIx - endX) / PGridSize;
	GameMode->SiteManager->GetCenterXYFromSiteIX(endIx, endX, endY);
	
	int32 dX = (endX - startX) / 5;
	int32 dY = (endY - startY) / 5;
	
	//Use direction to find cell and correct corner ix in tree space
	int32 cellX = startX + (dX * 2);
	int32 cellY = startY + (dY * 2);

	//Corner indices are the at the edge of the starting cell corner
	int32 cornerX = cellX * ProgressGridData->CellResolution;
	int32 cornerY = cellY * ProgressGridData->CellResolution;
	if (dX > 0)
	{
		cornerX += ProgressGridData->CellResolution - 1;
	}
	else if (dX == 0)
	{
		cornerX += ProgressGridData->CellResolution/2-2;
	}
	if (dY > 0)
	{
		cornerY += ProgressGridData->CellResolution - 1;
	}
	else if (dY == 0)
	{
		cornerY += ProgressGridData->CellResolution/2-2;
	}
	
	
	//////populate starting position indexes
	TArray<int32> indicesX;
	TArray<int32> indicesY;
	int32 span = ProgressGridData->PioneerPathWidth;

	if (dX == 0 || dY == 0)
	{
		PopulateOrthogonalIndices(dX, dY, cornerX, cornerY, indicesX, indicesY, span);
	}
	else
		PopulateCornerIndices(dX, dY, cornerX, cornerY, indicesX, indicesY, span);

		
	//for each limb, give mori an amount of them
	int32 limbLanes = indicesX.Num() / cutterQueues.Num();
	for (int32 i = cutterQueues.Num()-1; i >= 0; i--)
	{
		TArray<int32> giveX;
		TArray<int32> giveY;
		//If last, give remaining
		if (i == 0)
		{
			giveX = indicesX;
			giveY = indicesY;
		}
		//Otherwise add amount according to division
		else	
		{
			for (int32 j = 0; j < limbLanes; j++)
			{
				giveX.Add(indicesX.Pop());
				giveY.Add(indicesY.Pop());
			}
		}

		cutterQueues[i].trees = GameMode->MoriManager->CreatePioneerTreeLanes(giveX, giveY, dX, dY, ProgressGridData->CellResolution*3);
	}

	//Attend location before changing cell focus
	attendLoc = GameMode->MoriManager->GetWorldLocByCoords(cornerX + (2*-dX), cornerY + (2*-dY));
	
	//////Get scrap middle pieces
	TArray<int32> firstHalfX;
	TArray<int32> firstHalfY;
	TArray<int32> secondHalfX;
	TArray<int32> secondHalfY;
	bool diagonal = true;
	cellX = startX + (dX * 5);
	cellY = startY + (dY * 5);
	cornerX = cellX * ProgressGridData->CellResolution;
	cornerY = cellY * ProgressGridData->CellResolution;

	//Offset corner according to direction
	if (dX < 0)
	{
		cornerX += ProgressGridData->CellResolution - 1;
	}
	if (dY < 0)
	{
		cornerY += ProgressGridData->CellResolution - 1;
	}

	//Orthogonal
	if (dX == 0 || dY == 0)
	{
		int32 halfDist = (ProgressGridData->CellResolution-span)/2;
		diagonal = false;
		if (dY == 0)
		{
			for (int32 i = halfDist-1; i >= 0; i--)
			{
				firstHalfX.Add(cornerX - (dX * 1));
				firstHalfY.Add(cornerY + i);
			}
			for (int32 i = halfDist + span; i < ProgressGridData->CellResolution; i++)
			{
				secondHalfX.Add(cornerX - (dX * 1));
				secondHalfY.Add(cornerY + i);
			}

		}
		//Get middle Xs
		else if (dX == 0)
		{
			for (int32 i = halfDist-1; i >= 0; i--)
			{
				firstHalfX.Add(cornerX + i);
				firstHalfY.Add(cornerY - (dY * 1));
			}
			for (int32 i = halfDist + span; i < ProgressGridData->CellResolution; i++)
			{
				firstHalfX.Add(cornerX + i);
				firstHalfY.Add(cornerY - (dY * 1));
			}
		}
	}
	//Diagonal
	else
	{
		//Populate half indices
		for (int32 i = 0; i < ProgressGridData->CellResolution - span; i++)
		{
			firstHalfX.Add(cornerX + dX * ((span+1) + i - 1));
			firstHalfY.Add(cornerY - dY * 1);
		}

		for (int32 i = 0; i < ProgressGridData->CellResolution - span; i++)
		{
			secondHalfX.Add(cornerX - dX * 1);
			secondHalfY.Add(cornerY + dY * ((span+1) + i - 1));
		}
	}

	//Give trees to first and second half respectively
	bool xFirst = true;
	if (dX == dY || dY == 0 && dX > 0 || dY < 0 && dX == 0)
		xFirst = false;
	
	GatherScrapTrees(diagonal, xFirst, cutterQueues, dX, dY, span, firstHalfX, firstHalfY);
	GatherScrapTrees(diagonal, !xFirst, cutterQueues, dX, dY, span, secondHalfX, secondHalfY);	
}


//
// FPlanStack ACitadelBrainManager::GetNextProductionPlan(int32 planID, AMech_Weaver* weaver)
// {
// 	FProductionSitePlan plan = ProductionSitePlan[planID];
//
// 	//Build the initial dual pipe
// 	if (plan.Progress == 0)
// 	{
// 		FSiteDemandInfo dInfo = plan.DemandInfo;
// 		
// 		//Generate pipe section points between start and end location
// 		TArray<FVector> points = GameMode->PipelineManager->GenPipeSectionPoints(dInfo.StartLoc, dInfo.EndLoc);
// 				
// 		//Generate build info
// 		FPipeBuildInfo waterPipeInfo;
// 		FPipeBuildInfo oilPipeInfo;
// 		GameMode->PipelineManager->GenDoublePipeBuildInfo(waterPipeInfo, oilPipeInfo, points);
// 				
// 		//Initialize biuld package with site and build info
// 		FPipelineLayPackage pPack;
// 		pPack.StartSiteIx = dInfo.StartSiteIx;
// 		pPack.StartPumpLoc = dInfo.StartLoc;
// 		pPack.EndSite = dInfo.EndSite;
// 		pPack.EndPumpLoc = dInfo.EndLoc;
// 		pPack.waterLay = waterPipeInfo;
// 		pPack.oilLay = oilPipeInfo;
// 		pPack.bWater = true;
// 		pPack.bOil = true;
// 		//Add to weaver map
// 		GameMode->BehaviorManager->SetWeaverPipeInfo(weaver, pPack);
//
// 		FPlanStack stacc;
// 		stacc.PlanStack.Push(PlanData->GenAction(AT_IncrementProductionPlan));
// 		stacc.PlanStack.Append(PlanData->GenPlan_GoLayPipe(dInfo.StartLoc, dInfo.EndLoc, 1000).PlanStack);
// 		//Startup instruction readied 
// 		return stacc;
// 	}
// 	//Build the bones
// 	// else if (plan.Progress == 1)
// 	// {
// 	// 	//Plan
// 	// 	FRefineryBones rBones = GameMode->WombManager->PlanOutRefineryBones(StartSpireLoc, 35, plan.DemandInfo.EndSite, FVector(0,0,0));
// 	//
// 	// 	FRailLayInfo rInfo;
// 	// 	rInfo.Destinations = rBones.RailTransforms;
// 	// 	PlanData->GenPlan_GoLayRail(dInfo.StartLoc, dInfo.EndLoc, 1000);
// 	// 	//rBones.
// 	// 	
// 	// 	// //Use the vector of the pipeline that just finished and extend/rotate it to find the womb pool site
// 	// 	// FVector factoryPumpGuess = FVector::ZeroVector;
// 	// 	// GameMode->BehaviorManager->ExtendAndRotateVector(factoryPumpGuess, pPack.StartPumpLoc, pPack.EndPumpLoc, 10000, 10);
// 	// 	// 		
// 	// 	// //Generate water pipe data
// 	// 	// TArray<FVector> fPoints = GameMode->PipelineManager->GenPipeSectionPoints(pPack.EndPumpLoc, factoryPumpGuess);
// 	// 	// FPipeBuildInfo factoryLayInfo;
// 	// 	// GameMode->PipelineManager->GenPipeBuildInfo(factoryLayInfo, fPoints, true);
// 	// 	//
// 	// 	// //Generate refinery package 
// 	// 	// FPipelineLayPackage factoryPack;
// 	// 	// factoryPack.StartSiteIx = pPack.EndPumpIx;
// 	// 	// factoryPack.StartPumpLoc = pPack.EndPumpLoc;
// 	// 	// factoryPack.EndSite = pPack.EndSite;
// 	// 	// factoryPack.EndPumpLoc = factoryPumpGuess;
// 	// 	// factoryPack.EndPumpType = PT_FactoryOil;
// 	// 	// factoryPack.bOil = true;
// 	// 	// factoryPack.oilLay = factoryLayInfo;
// 	// 	//
// 	// 	// //Create the package
// 	// 	// FProductionSitePlan siteP;
// 	// 	// siteP.FactoryBranch = factoryPack;
// 	// 	// //siteP.WombBranch = wombPack;
// 	// }
// 	// else
// 	// {
// 	// 	MarkSiteWorkWeaverReady(weaver);
// 	// 	return FPlanStack();
// 	// }
// }
//
//

