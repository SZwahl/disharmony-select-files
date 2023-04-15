// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaverManager.h"

#include "DisharmonyGameMode.h"
#include "Kismet/KismetMathLibrary.h"
#include "Managers/AgentsBehaviorManager.h"
#include "Managers/PipelineManager.h"
#include "Managers/WombManager.h"
#include "Mechs/Mech_Weaver.h"

void UWeaverManager::PerformTick(float DeltaTime)
{
	
}
 
void UWeaverManager::CreateActionObject(FPlanAction action, AActor* agent)
{
	AMech_Weaver* weav = Cast<AMech_Weaver>(agent);
	
	switch (action.Type)
	{
	case AT_LayRail:
		{
			LayingRails.Add(weav, action);
			break;
		}
	case AT_CheckNewUmbJob:
		{
			CheckingUmbJob.Add(weav, action);
			break;
		}
	case AT_SpawnUmbilicals:
		{
			SpawningUmbilicals.Add(weav, action);
			break;
		}
	case AT_FinishUmbilicals:
		{
			FinishingUmiblicals.Add(weav, action);
			break;
		}
	case AT_SeedWomb:
		{
			SeedingWombs.Add(weav, action);
			break;
		}
	case AT_BecomeIdleAttendant:
		{
			BecomingIdle.Add(weav, action);
			break;
		}
	case AT_LayPipe:
		{
			LayingPipe.Add(weav, action);
			break;
		}
	case AT_CheckNextPipe:
		{
			CheckingNextPipe.Add(weav, action);
			break;
		}
	case AT_BuildEndPump:
		{
			BuildingPump.Add(weav, action);
			break;
		}
	case AT_CheckNewPumpJob:
		{
			CheckPumpJob.Add(weav, action);
			break;
		}
	default:
		break;
	}
}

TSet<AActor*> UWeaverManager::ExecuteAllActions()
{
	TSet<AActor*> Leftovers;

	//Lay Rail
	for (auto& Elem : LayingRails)
	{
		if (LayRail(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	LayingRails.Empty();

	//Check Umb Jobs
	for (auto& Elem : CheckingUmbJob)
	{
		if (CheckUmbJob(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	CheckingUmbJob.Empty();

	//Spawn umbilicals
	for (auto& Elem : SpawningUmbilicals)
	{
		if (SpawnUmbilicals(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	SpawningUmbilicals.Empty();

	//Finish Umbilicals
	for (auto& Elem : FinishingUmiblicals)
	{
		if (FinishUmbilicals(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	FinishingUmiblicals.Empty();

	//Seed womb
	for (auto& Elem : SeedingWombs)
	{
		if (SeedWomb(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	SeedingWombs.Empty();

	//Become idle
	for (auto& Elem : BecomingIdle)
	{
		if (BecomeIdleAttendant(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	BecomingIdle.Empty();

	//Laying pipe
	for (auto& Elem : LayingPipe)
	{
		if (LayPipe(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	LayingPipe.Empty();

	//Check next pipe
	for (auto& Elem : CheckingNextPipe)
	{
		if (CheckNextPipe(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	CheckingNextPipe.Empty();

	//Build pump
	for (auto& Elem : BuildingPump)
	{
		if (BuildPump(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	BuildingPump.Empty();

	//Check Pump job
	for (auto& Elem : CheckPumpJob)
	{
		if (CheckPumpJobs(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	CheckPumpJob.Empty();
	
	return Leftovers;
}

void UWeaverManager::InitializeWeaver(AMech_Weaver* weav)
{
	//FWeaverInfos info;
	//WeaverMap.Add(weav, info);
	BehaviorManager->RegisterAgent(weav);
	int32 ix = GameMode->SiteManager->WorldToSiteIx(weav->GetActorLocation());
	GameMode->WombManager->MarkWeaverIdle(weav, ix);
}

/*Action Functions*/

//------------------------------------------------
//------------------------------------------------


bool UWeaverManager::LayRail(FPlanAction action, AMech_Weaver* weav)
{
	//FVector spawnDest = RailWeaverMap[weav].Destinations.Pop();

	//Spawn it
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	//float xScale = action.TargetTransform.GetScale3D().X;
	AActor* rail = GetWorld()->SpawnActor(RailBP, &action.TargetTransform, spawnParams);
	rail->SetActorScale3D(action.TargetTransform.GetScale3D());
	// //update info
	// RailWeaverMap[weav].CurrentRails.Add(rail);
	// RailWeaverMap[weav].Placed.Insert(rail->GetActorLocation(), 0);
	//
	// auto rInfo = RailWeaverMap[weav];
	// //Spawn womb and initialize if all rails lain
	// if (rInfo.Destinations.Num() == 0 && rInfo.CurrentRails.Num() == 5)
	// {
	// 	//Initialize womb with manager
	// 	int32 wID = GameMode->WombManager->InitializeUnitWomb(rInfo.Placed, rInfo.SiteIx, true);
	//
	// 	//Find umbilical transform
	// 	FVector railPos = rInfo.CurrentRails.Top()->GetActorLocation();
	// 	FRotator umbDir = UKismetMathLibrary::FindLookAtRotation(rInfo.PumpLoc, railPos);
	// 	float scale = ((railPos - rInfo.PumpLoc)).Size()/100;
	//
	// 	//Load up info to map
	// 	FUmbilicalLayInfo uInfo;
	// 	uInfo.WombID = wID;
	// 	uInfo.UmTransform = FTransform(umbDir, rInfo.PumpLoc, FVector(scale, 1, 1));
	// 	UmbilicalWeaverMap.Add(weav, uInfo);
	// 	
	// 	//Forget the rail placement info
	// 	RailWeaverMap.Remove(weav);
	//
	// 	//Generate plan
	// 	FPlanStack plan = BehaviorManager->PlanData->GenPlan_GoSpawnUmbilicals(rInfo.PumpLoc, railPos, 1000);
	// 	BehaviorManager->SubmitAgentPlan(weav, plan);		
	// }
	// else
	// {
	// 	FPlanStack plan = BehaviorManager->PlanData->GenPlan_GoLayRail(RailWeaverMap[weav].Destinations.Top(), 10);
	// 	BehaviorManager->SubmitAgentPlan(weav, plan);	
	// }
	return true;
}

bool UWeaverManager::CheckUmbJob(FPlanAction action, AMech_Weaver* weav)
{
	//Check for more laying jobs
	FWeaverUmbilicalJobs* jobs = &WeaverUmbilicalJobs[weav];
	if (jobs)
	{
		//if it has one, generate a plan and submit it
		if (jobs->Jobs.Num() > 0)
		{
			FUmbLayInfo pInfo = jobs->Jobs.Pop();
			FPlanStack layPlan = BehaviorManager->PlanData->GenPlan_GoSpawnUmbilicals(pInfo.T, 1000);
			WeaverUmbilicalInfo.Add(weav, pInfo);
			
			BehaviorManager->SubmitAgentPlan(weav, layPlan);
		}
		else WeaverUmbilicalJobs.Remove(weav);
	}
	
	return true;
}

bool UWeaverManager::SpawnUmbilicals(FPlanAction action, AMech_Weaver* weav)
{
	auto info = WeaverUmbilicalInfo[weav];
	GameMode->WombManager->SpawnFactoryUmbilicals(info.SiteID, info.T);
	
	return true;
}

bool UWeaverManager::FinishUmbilicals(FPlanAction action, AMech_Weaver* weav)
{
	//Get info
	auto info = WeaverUmbilicalInfo[weav];
	//Mark refinery wombs ready
	GameMode->WombManager->FinishUmbConnection(info.SiteID);
	//Remove from map
	WeaverUmbilicalInfo.Remove(weav);
	

	//AgentPlans[weaver].PlanStack.Push(PlanData->GenAction(AT_BecomeIdleAttendant));
	//GameMode->CitadelManager->MarkSiteWorkWeaverReady(weaver);
	return true;
}

bool UWeaverManager::SeedWomb(FPlanAction action, AMech_Weaver* weav)
{
	if (SeedWeaverMap.Contains(weav))
	{
		auto info = SeedWeaverMap[weav];
		AActor* newUnit = GameMode->WombManager->SeedWomb(info.DestTransform.GetLocation(), info.Name, info.WombID);

		//If born with instructions ( a weaver ) then be off
		if (info.StartupInstructions.PlanStack.Num() > 0)
		{
			// int32 planID = WombWithSitePlan[info.WombID];
			// WeaverProductionPlans.Add(Cast<AMech_Weaver>(newUnit), planID);
			// NeedPlanInitiate.Add(newUnit);
		}
		
		// //Uhhhh if this has pipe laying info, transfer it to the new one (lol)
		// //dude this is pretty spicy, not a good look
		// //This is why we stop coding before 5!!!
		// if (newUnit && PipeWeaverMap.Contains(weaver))
		// {
		// 	FPipelineLayPackage pPack;
		// 	PipeWeaverMap.RemoveAndCopyValue(weaver, pPack);
		// 	PipeWeaverMap.Add(Cast<AMech_Weaver>(newUnit), pPack);
		// }
		
		if (!newUnit)
		{
			UE_LOG(LogTemp, Error, TEXT("Uh boss, we couldn't spawn that"));
		}
		
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Attempting to seed womb with no SeedWeaverMap instruction?"))
	}
	return true;
}

bool UWeaverManager::BecomeIdleAttendant(FPlanAction action, AMech_Weaver* weav)
{
	//Make idle by getting site ix
	FVector loc = weav->GetActorLocation();
	int32 ix = GameMode->SiteManager->WorldToSiteIx(loc);
	GameMode->WombManager->MarkWeaverIdle(weav, ix);
	//Remove from seed map
	SeedWeaverMap.Remove(weav);
	return false;
}

bool UWeaverManager::LayPipe(FPlanAction action, AMech_Weaver* weav)
{
	bool lay = true;
	bool bWater = PipeWeaverMap[weav].bWater;
	bool bOil = PipeWeaverMap[weav].bOil;
	bool bAllOut = false;
	while (lay)
	{
		FVector loc;
		FRotator rot;
		float scale;
		int32 type;

		//If water, lay corresponding piece
		if (bWater)
		{
			loc = PipeWeaverMap[weav].waterLay.Location.Pop();
			rot = PipeWeaverMap[weav].waterLay.Rotation.Pop();
			scale = PipeWeaverMap[weav].waterLay.Scale.Pop();
			type = PipeWeaverMap[weav].waterLay.Type.Pop();

			//Add pipe if it's the base
			if (type == 0)
			{
				int32 ix = GameMode->PipelineManager->AddPipeBase(loc, rot, scale, PipeWeaverMap[weav].waterLay.PipeFluid);
				PipeWeaverMap[weav].waterLay.pLink.PipePieces.Add(ix);

				
				//Normal speed, this is when it would stop
				if (!BehaviorManager->WorkSuperFast)
					lay = false;
			}
			//Otherwise add accessory
			else
				GameMode->PipelineManager->AddPipeAccessory(loc, rot, type, scale, PipeWeaverMap[weav].waterLay.PipeFluid);

			if (PipeWeaverMap[weav].waterLay.Location.Num() == 0)
				bAllOut = true;
		}
		
		//If water, lay corresponding piece
		if (bOil)
		{
			loc = PipeWeaverMap[weav].oilLay.Location.Pop();
			rot = PipeWeaverMap[weav].oilLay.Rotation.Pop();
			scale = PipeWeaverMap[weav].oilLay.Scale.Pop();
			type = PipeWeaverMap[weav].oilLay.Type.Pop();

			//Add pipe if it's the base
			if (type == 0)
			{
				int32 ix = GameMode->PipelineManager->AddPipeBase(loc, rot, scale, PipeWeaverMap[weav].oilLay.PipeFluid);
				PipeWeaverMap[weav].oilLay.pLink.PipePieces.Add(ix);

				
				//Normal speed, this is when it would stop
				if (!BehaviorManager->WorkSuperFast)
					lay = false;

			}
			//Otherwise add accessory
			else
				GameMode->PipelineManager->AddPipeAccessory(loc, rot, type, scale, PipeWeaverMap[weav].oilLay.PipeFluid);

			if (PipeWeaverMap[weav].oilLay.Location.Num() == 0)
				bAllOut = true;
		}


		

		//return if all out
		if (bAllOut)
			return true;
	}

	return true;
}

bool UWeaverManager::CheckNextPipe(FPlanAction action, AMech_Weaver* weav)
{
	bool bNotDone = false;
	FVector lastLoc;

	//Check if not done for the oil or water it has
	if (PipeWeaverMap[weav].bWater && PipeWeaverMap[weav].waterLay.Location.Num() > 0)
	{
		bNotDone = true;
		lastLoc = PipeWeaverMap[weav].waterLay.Location.Last();
	}
	else if (PipeWeaverMap[weav].bOil && PipeWeaverMap[weav].oilLay.Location.Num() > 0)
	{
		bNotDone = true;
		lastLoc = PipeWeaverMap[weav].oilLay.Location.Last();
	}
				
	//If not done,
	if (bNotDone)
	{
		FPlanStack plan;
		
		//And check again
		plan.PlanStack.Add(BehaviorManager->PlanData->GenAction(AT_CheckNextPipe));
		//Then lay next
		plan.PlanStack.Add(BehaviorManager->PlanData->GenAction(AT_LayPipe));
		//First go to next location
		plan.PlanStack.Add(BehaviorManager->PlanData->GenAction(AT_GoToLoc, FTransform(lastLoc), 1000));

		BehaviorManager->SubmitAgentPlan(weav, plan);
	}
	return true;
}

bool UWeaverManager::BuildPump(FPlanAction action, AMech_Weaver* weav)
{
	//Get info
	FPipelineLayPackage* layInfo = &PipeWeaverMap[weav];

	//Spawn pump
	int32 newPump = GameMode->PipelineManager->AddFluidPump(action.TargetTransform, layInfo->EndPumpType);
	layInfo->EndPumpIx = newPump;
	
	//Initialize pipeline with pressure
	GameMode->PipelineManager->InitializeNewPipeline(layInfo);

	WeaverPumpJobs[weav].lastPumpLain = newPump;
	
	//Set site pump if so
	if (layInfo->EndPumpType == PT_Site)
	{
		//Set the site pump
		GameMode->SiteManager->RegisterSitePump(layInfo->EndSiteIx, newPump);
		GameMode->WombManager->RegisterSitePump(layInfo->EndSiteIx, newPump);
		//
	}
	else if (layInfo->EndPumpType == PT_Refinery)
	{
		//Register as site pump
		GameMode->WombManager->RegisterRefineryPump(layInfo->EndSiteIx, newPump);
		//UE_LOG(LogTemp, Warning, TEXT("Sjnorp"));

		// //Ask womb manager to generate good bones
		// FRailLayInfo rInfo = GameMode->WombManager->PlanOutWombBones(pPack);
		// 			
		// RailWeaverMap.Add(weav, rInfo);
		// 	
		// //Generate womb lay instructions then connect bridge
		// FPlanStack plan = BehaviorManager->PlanData->GenPlan_GoLayRail(rInfo.Destinations.Top(), 10);
		// BehaviorManager->SubmitAgentPlan(weav, plan);
	}

	PipeWeaverMap.Remove(weav);

	LastPumpBuilt.Add(weav, newPump);
	
	return true;
}

bool UWeaverManager::CheckPumpJobs(FPlanAction action, AMech_Weaver* weav)
{
	//Check for more laying jobs
	FWeaverPumpJobs* jobs = &WeaverPumpJobs[weav];
	if (jobs)
	{
		//if it has one, generate a plan and submit it
		if (jobs->Jobs.Num() > 0)
		{
			FPumpLayStartingInfo pInfo = jobs->Jobs.Pop();
			FPlanStack layPlan = GeneratePumpPlan(
				weav,
				pInfo.PumpType,
				pInfo.EndLoc,
				pInfo.StartSiteIx,
				pInfo.StartSiteIx,
				pInfo.bIsOil,
				pInfo.bIsWater
			);

			BehaviorManager->SubmitAgentPlan(weav, layPlan);
		}
		else WeaverPumpJobs.Remove(weav);
	}
	
	return true;
}

/*Related Functions*/

//------------------------------------------------
//------------------------------------------------


// bool UWeaverManager::RegisterFinishedPump(AMech_Weaver* weav)
// {
// 	FPipelineLayPackage* pPack = &PipeWeaverMap[weav];
// 	
// 	switch (pPack->EndPumpType)
// 	{
// 	case PT_WombWater:
// 		{
//
// 				
// 			return true;
// 		}
// 	case PT_FactoryOil:
// 		{
// 			//GameMode->CitadelManager->MarkSiteWorkWeaverReady(weav);
// 			return false;
// 		}
// 	default:
// 		return false;
// 	}
// }

//Rando public functions

void UWeaverManager::EnactBuildInstructions(TArray<AMech_Weaver*> weavers, TArray<FVector> destinations,
	TArray<FString> names, TArray<FPlanStack> startInsts, TArray<int32> siteIx)
{
	for (int32 i = 0; i < weavers.Num(); i++)
	{
		//Add to map
		FSeedInstruction seedInst;
		seedInst.DestTransform = FTransform(FRotator(), destinations[i]);
		seedInst.Name = names[i];
		seedInst.WombID = -1;
		SeedWeaverMap.Add(weavers[i], seedInst);

		GameMode->WombManager->RemoveSiteWeaver(weavers[i], siteIx[i]);
	
		//go to and seed plan
		FPlanStack plan;
		plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_BecomeIdleAttendant));
		plan.PlanStack.Append(BehaviorManager->PlanData->GenPlan_GoSeedWomb(destinations[i] + FVector(0,0,600), 1000).PlanStack);
		BehaviorManager->SubmitAgentPlan(weavers[i], plan);
		BehaviorManager->MakeAgentAct(weavers[i]);
	}
}

void UWeaverManager::AddSeedInstruction(AMech_Weaver* weav, FVector dest, FString name)
{
	//Add to map
	FSeedInstruction seedInst;
	seedInst.DestTransform = FTransform(FRotator(), dest);
	seedInst.Name = name;
	seedInst.WombID = -1;
	SeedWeaverMap.Add(weav, seedInst);

	// GameMode->WombManager->RemoveSiteWeaver(weav, siteIx);
	//
	// //go to and seed plan
	// FPlanStack plan = BehaviorManager->PlanData->GenPlan_GoSeedWomb(dest + FVector(0,0,600), 1000);
	// BehaviorManager->SubmitAgentPlan(weav, plan);
	// BehaviorManager->MakeAgentAct(weav);
}

FPlanStack UWeaverManager::GenerateRefineryPlan(AMech_Weaver* weav, FVector sitePumpTarget, int32 startSite, int32 endSite)
{
	FPlanStack plan;

	FRefineryBones rBones = GameMode->WombManager->PlanOutRefineryBones(
		GameMode->SiteManager->GetSite(endSite)->WorldLocation,
		35,
		startSite,
		FVector(0,0,0));

	GameMode->WombManager->InitializeRefineryWomb(rBones, endSite, false, GameMode->SiteManager->GetSite(endSite)->Spires[0]->GetActorLocation());

	/*Reverse order because plan stack*/
	
	//Become idle

	//Finish umbilicals
	plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_FinishUmbilicals));
	
	//Umbilicals
	FWeaverUmbilicalJobs umbJobs;
	umbJobs.Jobs.Add(FUmbLayInfo(rBones.UmbilicalTransforms[0], endSite));
	umbJobs.Jobs.Add(FUmbLayInfo(rBones.UmbilicalTransforms[1], endSite));
	umbJobs.Jobs.Add(FUmbLayInfo(rBones.UmbilicalTransforms[2], endSite));
	WeaverUmbilicalJobs.Add(weav, umbJobs);

	plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_CheckNewUmbJob));
	plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_CheckNewUmbJob));
	plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_CheckNewUmbJob));

	//Rails
	for (auto t : rBones.RailTransforms)
	{
		plan.PlanStack.Append(BehaviorManager->PlanData->GenPlan_GoLayRail(t, 1000).PlanStack);
	}
	
	//Pump instructions
	FWeaverPumpJobs pumpJobs;
	pumpJobs.Jobs.Add(FPumpLayStartingInfo(PT_Refinery, rBones.PumpLocs[2], endSite, false, true));
	pumpJobs.Jobs.Add(FPumpLayStartingInfo(PT_Refinery, rBones.PumpLocs[1], endSite, false, true));
	pumpJobs.Jobs.Add(FPumpLayStartingInfo(PT_Refinery, rBones.PumpLocs[0], endSite, false, true));
	pumpJobs.lastPumpLain = startSite;
	WeaverPumpJobs.Add(weav, pumpJobs);

	plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_CheckNewPumpJob));
	plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_CheckNewPumpJob));
	plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_CheckNewPumpJob));
		
	//Build site pump
	plan.PlanStack.Append(
		GeneratePumpPlan(weav,
		                 PT_Site,
		                 sitePumpTarget,
		                 startSite,
		                 endSite,
		                 true,
		                 true)
		.PlanStack);

	return plan;
}

FPlanStack UWeaverManager::GeneratePumpPlan(AMech_Weaver* weav, EPumpType type, FVector endPoint, int32 startSite, int32 endSite, bool bOil, bool bWater)
{
	FPlanStack plan;

	FTransform startWater = FTransform();
	FTransform startOil = FTransform();;

	int32 startPumpIx = GameMode->SiteManager->GetSitePump(startSite);

	if (type == PT_Site)
	{
		GameMode->PipelineManager->GetWaterAndOil(startPumpIx, startWater, startOil);
	}
	else if (type == PT_Refinery)
	{
		startWater = GameMode->PipelineManager->GetPumpWaterOut(LastPumpBuilt[weav]);
	}
	else
	{
		if (bWater)
			startWater = GameMode->PipelineManager->GetPumpWaterOut(startPumpIx);
		//if (bOil)
		//	startOil = GameMode->
	}
	
	//First build pipe info and plan
	FRotator pumpRot;
	FPipeBuildInfo waterPipeInfo;
	FPipeBuildInfo oilPipeInfo;
	if (bOil)
	{
		//Generate pipe section points between start and end location
		TArray<FVector> points = GameMode->PipelineManager->GenPipeSectionPoints(startOil.GetLocation(), endPoint + FVector (0,0,500), 500);
		pumpRot = UKismetMathLibrary::FindLookAtRotation(startOil.GetLocation(), endPoint);
		
		GameMode->PipelineManager->GenPipeBuildInfo(oilPipeInfo, points, FT_Oil);
	}
	if (bWater)
	{
		//Generate pipe section points between start and end location
		TArray<FVector> points = GameMode->PipelineManager->GenPipeSectionPoints(startWater.GetLocation(), endPoint);
		pumpRot = UKismetMathLibrary::FindLookAtRotation(startWater.GetLocation(), endPoint);
		
		GameMode->PipelineManager->GenPipeBuildInfo(waterPipeInfo, points, FT_MinWater);
	}
	//else if (bOil && bWater)
	//	GameMode->PipelineManager->GenDoublePipeBuildInfo(waterPipeInfo, oilPipeInfo, points);
	// else
	// 	UE_LOG(LogTemp, Error, TEXT("Neither oil nor water?"));

	//Finally build pump info
	plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_BuildEndPump, FTransform(pumpRot, endPoint), 1000));
	
	//Initialize build package with site and build info
	FPipelineLayPackage pPack;
	pPack.StartPumpIx = startPumpIx;
	pPack.StartPumpLoc = startWater.GetLocation();
	pPack.EndPumpLoc = endPoint;
	pPack.EndSiteIx = endSite;
	pPack.waterLay = waterPipeInfo;
	pPack.oilLay = oilPipeInfo;
	pPack.bWater = bWater;
	pPack.bOil = bOil;
	pPack.EndPumpType = type;

	//Add to weaver jobs
	PipeWeaverMap.Add(weav, pPack);
	
	//Then check for next action
	plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_CheckNextPipe));
	//Then lay the pipe piece
	plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_LayPipe));
	//First go to the designated destination
	plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_GoToLoc, FTransform(startWater)));
	
	return plan;
}


