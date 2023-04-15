// Fill out your copyright notice in the Description page of Project Settings.


#include "CuttersManagerAgain.h"

#include "AABB.h"
#include "AABB.h"
#include "DisharmonyGameMode.h"
#include "DrawDebugHelpers.h"
#include "LimbsManager.h"
#include "Buildings/LinkSpire.h"
#include "Buildings/TransportSpire.h"
#include "Buildings/TransportSplineActor.h"
#include "Managers/AgentNavigationManager.h"
#include "Managers/AgentsBehaviorManager.h"
#include "Managers/CitadelBrainManager.h"
#include "Managers/TransportManager.h"
#include "Mechs/Mech_Cutter_Limb.h"

void UCuttersManagerAgain::PerformTick(float DeltaTime)
{
	TArray<AMech_Cutter*> cutterRemoves;
	UpdateCheckSpireLOS(cutterRemoves);
	UpdateHookWaiters();
}

void UCuttersManagerAgain::CreateActionObject(FPlanAction action, AActor* agent)
{
	AMech_Cutter* cut = Cast<AMech_Cutter>(agent);

	//UE_LOG(LogTemp, Warning, TEXT("%s is doing a %s action"), *agent->GetName(), *UEnum::GetValueAsString(action.Type));
	
	switch (action.Type)
	{
	case AT_GetFreeArm:
		{
			GetArm.Add(cut, action);
			break;
		}
	case AT_ConnectSplines:
		{
			ConnectSpline.Add(cut, action);
			break;
		}
	case AT_DisconnectSplines:
		{
			DisconnectSpline.Add(cut, action);
			break;
		}
	case AT_GrabLinkSpireOutgoing:
		{
			GrabLinkIn.Add(cut, action);
			break;
		}
	case AT_MobileAfterHooks:
		{
			MobileAfterHooks.Add(cut, action);
			break;
		}
	case AT_GetPioneerWork:
		{
			PioneerWork.Add(cut, action);
			break;
		}
	case AT_GetSiteWork:
		{
			SiteWork.Add(cut, action);
			break;
		}
	case AT_SendOutLimbs:
		{
			LimbSenders.Add(cut, action);
			break;
		}
	case AT_ReorientForLimbs:
		{
			Reorient.Add(cut, action);
			break;
		}
	case AT_InitiateWrapUp:
		{
			InitWrap.Add(cut, action);
			break;
		}
	case AT_WrapUpPioneerWork:
		{
			WrapPioneer.Add(cut, action);
			break;
		}
	case AT_WrapUpSiteWork:
		{
			WrapSite.Add(cut, action);
			break;
		}
	case AT_LayLinkSpire:
		{
			LayLink.Add(cut, action);
			break;
		}
	case AT_LaySiteSpire:
		{
			LaySite.Add(cut, action);
			break;
		}
	default:
		{
			break;
		}
	}
}

TSet<AActor*> UCuttersManagerAgain::ExecuteAllActions()
{
	TSet<AActor*> Leftovers;

	//Get arms
	for (auto& Elem : GetArm)
	{
		if (GetFreeArm(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	GetArm.Empty();

	//Connect Splines
	for (auto& Elem : ConnectSpline)
	{
		if (ConnectSplines(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	ConnectSpline.Empty();

	//Disconnect Splines
	for (auto& Elem : DisconnectSpline)
	{
		if (DisconnectSplines(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	DisconnectSpline.Empty();
	
	//Grab incoming spline from link spire
	for (auto& Elem : GrabLinkIn)
	{
		if (GrabLinkSpireOutgoing(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	GrabLinkIn.Empty();
	
	//MobileAfterHooks
	for (auto& Elem : MobileAfterHooks)
	{
		if (IsMobileAfterHooks(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	MobileAfterHooks.Empty();

	//GetPioneerWork
	for (auto& Elem : PioneerWork)
	{
		if (GetPioneerWork(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	PioneerWork.Empty();

	//GetSiteWork
	for (auto& Elem : SiteWork)
	{
		if (GetSiteWork(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	SiteWork.Empty();
	
	//Send limbs
	for (auto& Elem : LimbSenders)
	{
		TArray<AActor*> leftover = SendOutLimbs(Elem.Value, Elem.Key);
		for (AActor* l : leftover)
		{
			Leftovers.Add(l);
		}
	}
	LimbSenders.Empty();

	//Reorient for limbs
	for (auto& Elem : Reorient)
	{
		if (ReorientForLimbs(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	Reorient.Empty();

	//Initiate wrapup
	for (auto& Elem : InitWrap)
	{
		if (InitiateWrapUp(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	InitWrap.Empty();	

	//Wrap up pioneer work
	for (auto& Elem : WrapPioneer)
	{
		TArray<AActor*> leftover = WrapUpPioneerWork(Elem.Value, Elem.Key);
		for (AActor* l : leftover)
		{
			Leftovers.Add(l);
		}
	}
	WrapPioneer.Empty();

	//Wrap up site work
	for (auto& Elem : WrapSite)
	{
		TArray<AActor*> leftover = WrapUpSiteWork(Elem.Value, Elem.Key);
		for (AActor* l : leftover)
		{
			Leftovers.Add(l);
		}
	}
	WrapSite.Empty();

	//Lay link spire
	for (auto& Elem : LayLink)
	{
		if (LayLinkSpire(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	LayLink.Empty();

	//Lay site spire
	for (auto& Elem : LaySite)
	{
		if (LaySiteSpire(Elem.Value, Elem.Key)) {Leftovers.Add(Elem.Key);}
	}
	LaySite.Empty();
	
	return Leftovers;
}

void UCuttersManagerAgain::InitializeCutter(AMech_Cutter* cut)
{
	FCutterInfos info;
	info.ChipHoldings = 0;
	CutterMap.Add(cut, info);
	BehaviorManager->RegisterAgent(cut);

	auto compos = cut->GetLimbStartComponents();
	
	FSplineMechInfo splInfo;
	splInfo.LeftSocketRef = compos[0];
	splInfo.RightSocketRef = compos[1];
	SplineMechMap.Add(cut, splInfo);

	//Make limbs, set to follow
	SpawnRegisterCutterLimbs(cut, compos);
	
	TArray<AActor*> limbs;
	MakeAllLimbsFollow(cut, limbs);
	for (auto limb : limbs)
	{
		BehaviorManager->MakeAgentAct(limb);
	}
	
	//unemployabl
	JoblessCutters.Add(cut);
}

void UCuttersManagerAgain::SpawnRegisterCutterLimbs(AMech_Cutter* cut, TArray<USceneComponent*> scenes)
{
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	for (USceneComponent* scene : scenes)
	{
		//Try projecting location
		FVector spawnLocation = scene->GetComponentLocation();
		GameMode->AgentNavManager->TryProjectToNavmesh(spawnLocation);
		FTransform spawnT = FTransform(spawnLocation);

		//Spawn
		AMech_Cutter_Limb* limb = Cast<AMech_Cutter_Limb>(GetWorld()->SpawnActor(CutterLimbBP, &spawnT, spawnParams));

		//Add to maps
		CutterMap[cut].Limbs.Add(limb);
		CutterMap[cut].LimbAnchors.Add(scene);
		BehaviorManager->GetLimbsManager()->LimbParents.Add(limb, cut);
		BehaviorManager->RegisterAgent(limb);
	}
}

/*Action Functions*/

//------------------------------------------------
//------------------------------------------------


bool UCuttersManagerAgain::GetFreeArm(FPlanAction action, AMech_Cutter* cut)
{
	AActor* spire = Cast<AActor>(action.RelevantObject);
				
	USceneComponent* arm = GameMode->TransportManager->GetAvailableArm(spire);
	RegisterCurrentArm(Cast<AGenericMech>(cut), arm);
	return true;
}

bool UCuttersManagerAgain::ConnectSplines(FPlanAction action, AMech_Cutter* cut)
{
	//Connect splines w/ function
	ConnectSplines(Cast<AGenericMech>(cut), Cast<AActor>(action.RelevantObject));
	return true;
}

bool UCuttersManagerAgain::DisconnectSplines(FPlanAction action, AMech_Cutter* cut)
{
	auto spInfo = SplineMechMap[cut];
	auto spActor = GameMode->TransportManager->RemoveMobileSpline(cut);
	GameMode->TransportManager->DeregisterSpline(spActor);
	GameMode->TransportManager->FreeExistingArm(spInfo.CurrentArmReference, spInfo.SpireRef);
	SplineMechMap[cut].CurrentLAnch = nullptr;
	SplineMechMap[cut].CurrentRAnch = nullptr;
	SplineMechMap[cut].SpireRef = nullptr;
	SplineMechMap[cut].CurrentArmReference = nullptr;
	return true;
}

bool UCuttersManagerAgain::GrabLinkSpireOutgoing(FPlanAction action, AMech_Cutter* cut)
{
	AActor* spire = Cast<AActor>(action.RelevantObject);

	TArray<FVector> leftSplinePoints;
	TArray<FVector> rightSplinePoints;

	//Get spire's incoming spline location
	auto spInfo = GameMode->TransportManager->GetSpireInfo(spire);
	auto beforeSpline = spInfo->OutgoingSpline;

	if (beforeSpline)
	{
		//Add necessary points
		leftSplinePoints.Add(beforeSpline->GetLeftSpline()->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World));
		rightSplinePoints.Add(beforeSpline->GetRightSpline()->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World));
		leftSplinePoints.Add(SplineMechMap[cut].RightSocketRef->GetComponentLocation());
		rightSplinePoints.Add(SplineMechMap[cut].LeftSocketRef->GetComponentLocation());

		//forgot to get arms...
		TArray<USceneComponent*> children;
		USceneComponent* outwardArm = spInfo->OutgoingArm;
		outwardArm->GetChildrenComponents(true, children);
		for (auto child : children)
		{
			if (child->ComponentTags.Num() > 0 && child->ComponentTags[0] == "L")
			{
				//leftSplinePoints.Add(child->GetComponentLocation());
				SplineMechMap[cut].CurrentLAnch = child;
			}
			else if (child->ComponentTags.Num() > 0 && child->ComponentTags[0] == "R")
			{
				//rightSplinePoints.Add(child->GetComponentLocation());
				SplineMechMap[cut].CurrentRAnch = child;
			}
		}

		//Assign spline points
		beforeSpline->GetLeftSpline()->SetSplinePoints(leftSplinePoints, ESplineCoordinateSpace::World, true);
		beforeSpline->GetRightSpline()->SetSplinePoints(rightSplinePoints, ESplineCoordinateSpace::World, true);

		//Spire reference set
		SplineMechMap[cut].SpireRef = spInfo->OutgoingSpire;
		
		//Remove incoming, remove from static spline lists
		GameMode->TransportManager->RemoveIncomingSpline(spInfo->OutgoingSpire, beforeSpline);
		GameMode->TransportManager->RemoveOutgoingSpline(spire, beforeSpline);
		GameMode->TransportManager->RemoveStaticSpline(beforeSpline);

		SplineMechMap[cut].CurrentArmReference = outwardArm;

		//Cutter should... not check LOS right now. 
		/////////CheckLOSCutters.Add(cut);
		

			//Register
		GameMode->TransportManager->RegisterMovingSpline(cut,
			SplineMechMap[cut].LeftSocketRef,
			SplineMechMap[cut].RightSocketRef,
			SplineMechMap[cut].CurrentLAnch,
			SplineMechMap[cut].CurrentRAnch,
			beforeSpline,
			outwardArm);
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("Unable to get link spire incoming spline with %s"), *cut->GetName());
	return false;
}

bool UCuttersManagerAgain::IsMobileAfterHooks(FPlanAction action, AMech_Cutter* cut)
{
	//Check if spline has hooks
	HookWaiters.Add(cut);
	return false; 
}
 
bool UCuttersManagerAgain::GetPioneerWork(FPlanAction action, AMech_Cutter* cut)
{
	FVector attendLoc;

	//Load pioneer work
	LoadPioneerWork(Cast<AMech_Cutter>(cut), attendLoc);
	
	//Generate go-to with attend loc
	FPlanStack plan = BehaviorManager->PlanData->GenPlan_AttendPatch(attendLoc, 500);
	BehaviorManager->SubmitAgentPlan(cut, plan);
				
	return true;
}

bool UCuttersManagerAgain::GetSiteWork(FPlanAction action, AMech_Cutter* cut)
{
	FVector attendLoc;

	//Load pioneer work
	bool bMoreWork = LoadSiteWork(Cast<AMech_Cutter>(cut), attendLoc);

	//If more work, get to it and queue GetSiteWork for when that's done
	if (bMoreWork)
	{
		FPlanStack plan;
		//Load more site work when done with attend
		plan.PlanStack.Add(BehaviorManager->PlanData->GenAction(AT_GetSiteWork));
		//First generate go-to with attend loc
		plan.PlanStack.Append(BehaviorManager->PlanData->GenPlan_AttendPatch(attendLoc, 500).PlanStack);
		BehaviorManager->SubmitAgentPlan(cut, plan);
	}
				
	return true;
}

TArray<AActor*> UCuttersManagerAgain::SendOutLimbs(FPlanAction action, AMech_Cutter* cut)
{
	TArray<AActor*> leftoverAgents;
	
	CheckLOSCutters.Remove(cut);

	auto spActor = GameMode->TransportManager->GetConnectedSpline(cut);
	if (!spActor)
	{
		UE_LOG(LogTemp, Error, TEXT("No current spline!"));
		return TArray<AActor*>();
	}
	//Set spline to static via TransportManager
	GameMode->TransportManager->RemoveMobileSpline(cut);
	GameMode->TransportManager->RegisterStaticSpline(spActor, cut);
	GameMode->TransportManager->RegisterIncomingSpline(SplineMechMap[cut].CurrentArmReference->GetAttachmentRootActor(), spActor);

	auto cutInfo = CutterMap[cut];
	//For each cutter
	for (int32 i = 0; i < cutInfo.Limbs.Num(); i++)
	{
		//Generate GetNext instruction
		auto limb = cutInfo.Limbs[i];
		FPlanAction newAct = BehaviorManager->PlanData->GenAction(EActionTypes::AT_GetNextTree);
		BehaviorManager->SubmitAgentPlan(limb, FPlanStack(newAct));

		//Stop Following
		BehaviorManager->GetLimbsManager()->FollowingLimbs.Remove(limb);

		//Add this limb to needs plan initiate
		leftoverAgents.Add(cutInfo.Limbs[i]);

		//Set to working cutters
		CutterMap[cut].WorkingLimbs.Add(limb);
	}

	return leftoverAgents;
}

bool UCuttersManagerAgain::ReorientForLimbs(FPlanAction action, AMech_Cutter* cut)
{
	//Find point to go to
	FVector attendLoc;

	float cumX = 0;
	float cumY = 0;
	float cumZ = 0;
	int32 num = CutterMap[cut].StretchedLimbs.Num();

	//Find median point between stretched cutter(s)
	for (int32 i = 0; i < CutterMap[cut].Limbs.Num(); i++)
	{
		auto limb = CutterMap[cut].Limbs[i];

		if (CutterMap[cut].StretchedLimbs.Contains(limb))
		{
			FVector thisLoc = limb->GetActorLocation();
			cumX += thisLoc.X;
			cumY += thisLoc.Y;
			cumZ += thisLoc.Z;
		}
	}

	//Empty that out
	CutterMap[cut].StretchedLimbs.Empty();

	attendLoc = FVector(cumX/num, cumY/num, cumZ/num);

	//Generate attend patch instruction close to stretched limb(s)
	FPlanStack plan = BehaviorManager->PlanData->GenPlan_AttendPatch(attendLoc, 1000);
	BehaviorManager->SubmitAgentPlan(cut, plan);
	return true;
}

TArray<FPlanAction> UCuttersManagerAgain::RecurseUnwind(AActor* tailSpire)
{
	auto spInfo = GameMode->TransportManager->GetSpireInfo(tailSpire);
	bool bTailisSite = GameMode->TransportManager->IsSiteSpire(tailSpire);
	//Then we unwind until it is done
	if (spInfo && !bTailisSite)
	{
		AActor* headSpire = spInfo->OutgoingSpire;

		TArray<FPlanAction> plan = RecurseUnwind(headSpire);
		plan.Append(BehaviorManager->PlanData->GenPlan_UnwindSpline(tailSpire, headSpire, 1000).PlanStack);
		return plan;
	}
	else
	{
		TArray<FPlanAction> plan;
		return plan;
	}
}

bool UCuttersManagerAgain::InitiateWrapUp(FPlanAction action, AMech_Cutter* cut)
{
	FPlanStack plan;
	
	if (CutterMap[cut].currentJob == AT_GetSiteWork)
	{
		plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_WrapUpSiteWork));

		AActor* tailSpire = SplineMechMap[cut].SpireRef;

		plan.PlanStack.Append(RecurseUnwind(tailSpire));
		
		//Then, of course, we must disconnect
		plan.PlanStack.Append(BehaviorManager->PlanData->GenPlan_GoDisconnectSpire(SplineMechMap[cut].SpireRef, 1000).PlanStack);
		//First wait for hooks to finish
		//plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_MobileAfterHooks));
	}
	else
	{
		plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_WrapUpPioneerWork));
	}



	CheckLOSCutters.Remove(cut);
	BehaviorManager->SubmitAgentPlan(cut, plan);

	return true;
}

TArray<AActor*> UCuttersManagerAgain::WrapUpPioneerWork(FPlanAction action, AMech_Cutter* cut)
{
	TArray<AActor*> leftoverAgents;
	
	//Set limbs to follow
	MakeAllLimbsFollow(cut, leftoverAgents);

	//Mark finished
	FSite siteInfo;
	int32 x;
	int32 y;
	FVector loc;
	GameMode->CitadelManager->MarkPioneeringFinished(cut, SplineMechMap[cut].SpireRef, x,y,loc);
	
	//Relevant spires
	TArray<AActor*> spires;
	spires.Add(SplineMechMap[cut].SpireRef);
	
	//Change to site cutter
	FPlanStack plan = GenerateSiteJob(cut, spires[0], x, y, loc, AT_GetSiteWork);
	BehaviorManager->SubmitAgentPlan(cut, plan);

	leftoverAgents.Add(cut);
	return leftoverAgents;
}

TArray<AActor*> UCuttersManagerAgain::WrapUpSiteWork(FPlanAction action, AMech_Cutter* cut)
{
	TArray<AActor*> leftoverAgents;
	
	//Set limbs to follow
	MakeAllLimbsFollow(cut, leftoverAgents);
	
	GameMode->CitadelManager->MarkSiteCutterFinished(cut);
	JoblessCutters.Add(cut);

	//leftoverAgents.Add(cut);
	return leftoverAgents;
}

bool UCuttersManagerAgain::LayLinkSpire(FPlanAction action, AMech_Cutter* cut)
{
	//Generate angle
	AActor* oldSpire = Cast<AActor>(action.RelevantObject);
	FVector toOther = cut->GetActorLocation() - oldSpire->GetActorLocation();
	toOther.Z = 0;
	toOther.Normalize();
	toOther = -toOther;
	
	ALinkSpire* spire = GameMode->TransportManager->SpawnLinkSpire(action.TargetTransform.GetLocation(), toOther.Rotation());
	
	if (spire)
	{
		//Now connect to it
		FPlanStack plan = BehaviorManager->PlanData->GenPlan_GoConnectSpire(spire, 1000);
		BehaviorManager->SubmitAgentPlan(cut, plan);
		return true;
	}
	//if not lmao idk
	UE_LOG(LogTemp, Error, TEXT("Unexpected lay link spire fail?"))
	return false;
}

bool UCuttersManagerAgain::LaySiteSpire(FPlanAction action, AMech_Cutter* cut)
{
	//Generate angle
	AActor* oldSpire = Cast<AActor>(action.RelevantObject);
	FVector toOther = cut->GetActorLocation() - oldSpire->GetActorLocation();
	toOther.Z = 0;
	toOther.Normalize();
	toOther = -toOther;
	
	ATransportSpire* spire = GameMode->TransportManager->SpawnSiteSpire(action.TargetTransform.GetLocation(), toOther.Rotation() + FRotator(0,-90,0), cut, false);
	
	if (spire)
	{
		//Now connect to it
		GameMode->TransportManager->GetSpireInfo(spire)->armClaims++;
		FPlanStack plan = BehaviorManager->PlanData->GenPlan_GoConnectSpire(spire, 1000);
		BehaviorManager->SubmitAgentPlan(cut, plan);
		return true;
	}
	//if not lmao idk
	UE_LOG(LogTemp, Error, TEXT("Unexpected lay site spire fail?"))
	return false;
}


/*Related Functions*/

//------------------------------------------------
//------------------------------------------------

void UCuttersManagerAgain::RegisterCurrentArm(AGenericMech* mech, USceneComponent* arm)
{
	if (SplineMechMap[mech].CurrentArmReference)
	{
		SplineMechMap[mech].oldArmRef = SplineMechMap[mech].CurrentArmReference;
	}
	SplineMechMap[mech].CurrentArmReference = arm;
}

bool UCuttersManagerAgain::ConnectSplines(AGenericMech* mech, AActor* spire)
{
		//Broken references: log error
	if (!SplineMechMap[mech].CurrentArmReference)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find spire or arm reference!"));
		return false;
	}

	//If mech connected to spline already, transfer ownership and generate connect plan again
	if (GameMode->TransportManager->HasStaticSpline(mech))
	{
		UE_LOG(LogTemp, Error, TEXT("Uh fuck"));
	}
	if (GameMode->TransportManager->GetConnectedSpline(mech))
	{
		//Connect that spline to the spire
		TransferNewSpireSplines(mech);

		//Start new behavior to connect to this spire again
		FPlanStack plan = BehaviorManager->PlanData->GenPlan_GoConnectSpire(spire, 500);
		BehaviorManager->SubmitAgentPlan(mech, plan);
		//leftoverAgents.Add(mech);
		return true;
	}

 
	//--
	
	//Otherwise, we get the children and load socket positions
	TArray<USceneComponent*> children;
	SplineMechMap[mech].CurrentArmReference->GetChildrenComponents(true, children);
	TArray<FVector> leftSplinePoints;
	TArray<FVector> rightSplinePoints;
	//Loop arm children and get L and R sockets
	for (auto child : children)
	{
		if (child->ComponentTags.Num() > 0 && child->ComponentTags[0] == "L")
		{
			leftSplinePoints.Add(child->GetComponentLocation());
			SplineMechMap[mech].CurrentLAnch = child;
		}
		else if (child->ComponentTags.Num() > 0 && child->ComponentTags[0] == "R")
		{
			rightSplinePoints.Add(child->GetComponentLocation());
			SplineMechMap[mech].CurrentRAnch = child;
		}
	}
	//Add socket references from existing arms, cross to make it work
	leftSplinePoints.Add(SplineMechMap[mech].RightSocketRef->GetComponentLocation());
	rightSplinePoints.Add(SplineMechMap[mech].LeftSocketRef->GetComponentLocation());

	//--
	
	//Spawn outgoing spline
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FTransform spawnT = FTransform(spire->GetActorLocation());
	// FString name = "SplineActor";
	// name.AppendInt(SpActorsMade++);
	// spawnParams.Name = FName(name);
	ATransportSplineActor* spActor = Cast<ATransportSplineActor>(GetWorld()->SpawnActor(SpActorBPClass, &spawnT, spawnParams));
	

	//Spawn worked out, apply spline points and register to the manager
	if (spActor)
	{
		CheckLOSCutters.Add(Cast<AMech_Cutter>(mech));
		
		//Left and right spline points from earlier
		spActor->GetLeftSpline()->SetSplinePoints(leftSplinePoints, ESplineCoordinateSpace::World, true);
		spActor->GetRightSpline()->SetSplinePoints(rightSplinePoints, ESplineCoordinateSpace::World, true);

		//Spire reference
		SplineMechMap[mech].SpireRef = spire;

		//Register
		GameMode->TransportManager->RegisterMovingSpline(mech, SplineMechMap[mech].LeftSocketRef, SplineMechMap[mech].RightSocketRef, SplineMechMap[mech].CurrentLAnch, SplineMechMap[mech].CurrentRAnch, spActor, SplineMechMap[mech].CurrentArmReference);
		GameMode->TransportManager->RegisterNewSpline(spActor);
		return true;
	}
	return false;
}

void UCuttersManagerAgain::TransferNewSpireSplines(AGenericMech* mech)
{
	//FSplineMechInfo* mechInfo = &SplineMechMap[mech];


	TArray<USceneComponent*> children;
	TArray<FVector> leftSplinePoints;
	TArray<FVector> rightSplinePoints;
	//Get the Old points for left and right
	SplineMechMap[mech].oldArmRef->GetChildrenComponents(true, children);
	for (auto child : children)
	{
		if (child->ComponentTags.Num() > 0 && child->ComponentTags[0] == "L")
			leftSplinePoints.Add(child->GetComponentLocation());
		else if (child->ComponentTags.Num() > 0 && child->ComponentTags[0] == "R")
			rightSplinePoints.Add(child->GetComponentLocation());
	}
	//Get the Current points for left and right
	SplineMechMap[mech].CurrentArmReference->GetChildrenComponents(true, children);
	for (auto child : children)
	{
		if (child->ComponentTags.Num() > 0 && child->ComponentTags[0] == "L")
			rightSplinePoints.Add(child->GetComponentLocation());
		else if (child->ComponentTags.Num() > 0 && child->ComponentTags[0] == "R")
			leftSplinePoints.Add(child->GetComponentLocation());
	}

	//Remove current spline actor
	ATransportSplineActor* spActor = GameMode->TransportManager->RemoveMobileSpline(mech);
	
	//Assign points
	spActor->GetLeftSpline()->SetSplinePoints(leftSplinePoints, ESplineCoordinateSpace::World, true);
	spActor->GetRightSpline()->SetSplinePoints(rightSplinePoints, ESplineCoordinateSpace::World, true);
	
	//Add as static
	GameMode->TransportManager->RegisterStaticSpline(spActor, nullptr);
	
	//Register incoming on the owner of the current arm (the notifying spire arm
	GameMode->TransportManager->RegisterIncomingSpline(SplineMechMap[mech].oldArmRef->GetAttachmentRootActor(), spActor);
	//Register outgoing on old spire
	GameMode->TransportManager->RegisterOutgoingSpline(SplineMechMap[mech].oldArmRef->GetAttachmentRootActor(), SplineMechMap[mech].CurrentArmReference->GetAttachmentRootActor(), SplineMechMap[mech].oldArmRef, spActor);
	SplineMechMap[mech].CurrentArmReference = nullptr;
	SplineMechMap[mech].CurrentLAnch = nullptr;
	SplineMechMap[mech].CurrentRAnch = nullptr;
	SplineMechMap[mech].oldArmRef = nullptr;
}

void UCuttersManagerAgain::LoadPioneerWork(AMech_Cutter* cut, FVector& attendLoc)
{
	//Prepare empty cutter queues
	TArray<FCutterQueue> cutterQueues;
	for (int32 i = 0; i < CutterMap[cut].Limbs.Num(); i++)
	{
		cutterQueues.Add(FCutterQueue());
	}

	//Populate cutter queues with work and set attend loc
	GameMode->CitadelManager->GetCutterPioneerWork(cut, cutterQueues, attendLoc);

	ULimbsManager* LimbsManager = BehaviorManager->GetLimbsManager();
	//Set all limb queues in memory
	for (int32 i = 0; i < CutterMap[cut].Limbs.Num(); i++)
	{
		LimbsManager->LimbQueues.Add(CutterMap[cut].Limbs[i], cutterQueues[i]);
	}
}

bool UCuttersManagerAgain::LoadSiteWork(AMech_Cutter* cut, FVector& attendLoc)
{
	//Prepare empty cutter queues
	TArray<FCutterQueue> cutterQueues;
	for (int32 i = 0; i < CutterMap[cut].Limbs.Num(); i++)
	{
		cutterQueues.Add(FCutterQueue());
	}

	//Populate cutter queues with work and set attend loc
	ETreePatchDirection dir;
	//if no next patch, jobless
	if (!GameMode->CitadelManager->CutterSiteHasNext(cut, attendLoc, CutterMap[cut].PrevX, CutterMap[cut].PrevY, dir))
	{
		//JoblessCutters.Add(cut);
		//UE_LOG(LogTemp, Warning, TEXT("Site is empty! No more jobs for now :)"))
		return false;
	}

	int32 totalTrees = 0;
	int32 numLimbs = CutterMap[cut].Limbs.Num();
	for (int32 i = 0; i < numLimbs; i++)
	{
		//Get trees for each limb
		GameMode->CitadelManager->GetCutterSiteWork(i, numLimbs, CutterMap[cut].PrevX, CutterMap[cut].PrevY, dir,  cutterQueues[i].trees);

		//Add up total number of trees
		totalTrees+= cutterQueues[i].trees.Num();
	}

	ULimbsManager* LimbsManager = BehaviorManager->GetLimbsManager();
	//Set all limb queues in memory
	for (int32 i = 0; i < CutterMap[cut].Limbs.Num(); i++)
	{
		LimbsManager->LimbQueues.Add(CutterMap[cut].Limbs[i], cutterQueues[i]);
	}
	
	//else If no trees
	if (totalTrees <= 0)
	{
		//UE_LOG(LogTemp, Warning, TEXT("No trees in patch"))
		return LoadSiteWork(cut, attendLoc);
	}

	return true;
}

void UCuttersManagerAgain::MakeAllLimbsFollow(AMech_Cutter* cut, TArray<AActor*> &leftoverAgents)
{
	for (int32 i = 0; i < CutterMap[cut].Limbs.Num(); i++)
	{
		auto limb = CutterMap[cut].Limbs[i];
		FPlanStack plan = FPlanStack(BehaviorManager->PlanData->GenAction(AT_FollowCutter, CutterMap[cut].LimbAnchors[i]));
		BehaviorManager->SubmitAgentPlan(limb, plan);
		leftoverAgents.Add(limb);
	}
}

/*Update Functions*/

//------------------------------------------------
//------------------------------------------------

void UCuttersManagerAgain::UpdateCheckSpireLOS(TArray<AMech_Cutter*> cutterRemoves)
{
	for (AMech_Cutter* cut : CheckLOSCutters)
	{
		//Get distance
		auto spire = SplineMechMap[cut].SpireRef;

		if (!spire)
			continue;

		FVector spireLoc = spire->GetActorLocation();
		FVector thisLoc = cut->GetActorLocation();
		
		float distance = FVector::Distance(spireLoc, thisLoc);
		
		//Get LOS
		FCollisionQueryParams params;
		params.MobilityType = EQueryMobilityType::Static;
		//params.AddIgnoredActor(this);
		params.AddIgnoredActor(spire);
		FHitResult result;
		bool obstructionFound = //GetWorld()->LineTraceSingleByChannel(result,thisLoc + CutObstructionOffset, spireLoc + CutObstructionOffset, ECollisionChannel::ECC_WorldStatic, params);// LineTraceTestByChannel(thisLoc + CutObstructionOffset, spireLoc + CutObstructionOffset, ECollisionChannel::ECC_Visibility, params);
		GetWorld()->LineTraceSingleByProfile(result,thisLoc + CutObstructionOffset, spireLoc + CutObstructionOffset, "BlockAll", params);
		//FCollisionObjectQueryParams paramaram;
		//paramaram.ObjectTypesToQuery = ;
		//GetWorld()->LineTraceSingleByObjectType(result,thisLoc + CutObstructionOffset, spireLoc + CutObstructionOffset, paramaram, params);
		FColor debugColor;
		if (obstructionFound)
		{
			debugColor = FColor::Red;
		}
		else debugColor = FColor::Green;
		DrawDebugLine(GetWorld(), thisLoc + CutObstructionOffset, spireLoc + CutObstructionOffset, debugColor, false);

		if (obstructionFound)
		{
			UE_LOG(LogTemp, Warning, TEXT("Obstruction with %s"), *result.GetActor()->GetName());
			//DrawDebugSphere(GetWorld(), result.Location, 200, 8, FColor::Cyan, true);
		}

		//if over max distance or LOS violated, get last good location
		if (distance >= MinDistFromSpire && (distance >= MaxDistFromSpire || obstructionFound))
		{
			FVector* goodLoc = CutterGoodLocationMap.Find(cut);

			if (!goodLoc)
			{
				UE_LOG(LogTemp, Error, TEXT("No last good location!?"))
				continue;
			}

			//then attend loc where was before
			FPlanStack plan;
			plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_GoToLoc, FTransform(GameMode->AgentNavManager->GetActorTrueDestination(cut))));
			//first go to location and lay spire
			plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_LayLinkSpire, FTransform(*goodLoc), spire));
			plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_GoToLoc, FTransform(*goodLoc)));
			BehaviorManager->SubmitAgentPlan(cut, plan);

			BehaviorManager->MakeAgentAct(cut);

			cutterRemoves.Add(cut);
			//LayingCutters.Add(cut);
		}
			

		//otherwise log current location as good
		CutterGoodLocationMap.Add(cut, thisLoc);
	}
	for (AMech_Cutter* cut : cutterRemoves)
	{
		CheckLOSCutters.Remove(cut);
	}
	cutterRemoves.Empty();
}

void UCuttersManagerAgain::UpdateHookWaiters()
{
	for (auto It = HookWaiters.CreateConstIterator(); It; ++It)
	{
		auto cut = *It;
		
		auto spActor = GameMode->TransportManager->GetConnectedSpline(cut);
		if (!spActor)
		{
			UE_LOG(LogTemp, Error, TEXT("UNIT %s WAITING ON SPLINE WITH BAD REFERENCE"), *cut->GetName());
			continue;
		}
		bool bHasHooks = GameMode->TransportManager->CheckHasHooks(spActor);
	
		//If still has hooks, add plan back on to stack
		if (!bHasHooks)
		{
			//If not, remove the spline
			GameMode->TransportManager->RemoveStaticSpline(spActor, cut);
			GameMode->TransportManager->RegisterMovingSpline(cut, SplineMechMap[cut].LeftSocketRef, SplineMechMap[cut].RightSocketRef, SplineMechMap[cut].CurrentLAnch, SplineMechMap[cut].CurrentRAnch, spActor, SplineMechMap[cut].CurrentArmReference);
			GameMode->TransportManager->RemoveIncomingSpline(SplineMechMap[cut].CurrentArmReference->GetAttachmentRootActor(), spActor);
			CheckLOSCutters.Add(cut);
			BehaviorManager->MakeAgentAct(cut);
			HookWaiters.Remove(cut);
		}
	}
	
}

//Rando public functions

FPlanStack UCuttersManagerAgain::GenerateSiteJob(AMech_Cutter* cut, AActor* spireRef, int32 CenterX, int32 CenterY,
	FVector siteLoc, EActionTypes siteType)
{
	FPlanStack plan;

	auto spor = GameMode->TransportManager->GetSpireInfo(spireRef);
	int32 clems = 4;
	if (spor->bRefinerySpire) clems = 3;
	if (spor->armClaims == clems)
	{
			UE_LOG(LogTemp, Error, TEXT("Claims aren't working out!!"))
	}
	GameMode->TransportManager->GetSpireInfo(spireRef)->armClaims++;
	
	//If current reference exists and it's not the target one, gonna have to disconnect
	bool bWrongConnection = (SplineMechMap[cut].SpireRef != nullptr && spireRef != SplineMechMap[cut].SpireRef);

	//___Must process in reverse order to be a proper behavior stack___
	CutterMap[cut].currentJob = siteType;

	//Finally we Wrap up (pioneer or cutter)
	plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_InitiateWrapUp));

	//if wrapping up pioneer, we first lay a spire before wrapping up
	if (siteType == AT_GetPioneerWork)
	{
		plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(AT_LaySiteSpire, FTransform(siteLoc), spireRef));
	}
	
	//Third, we Get work (pioneer or cutter). This will load work and initiate everything else.
	plan.PlanStack.Push(BehaviorManager->PlanData->GenAction(siteType));
	
	//Second, if wrong connection or no connection at all, queue go to and connect
	if (bWrongConnection || SplineMechMap[cut].SpireRef == nullptr)
	{
		//Cutter "coming from" inside the house
		CutterMap[cut].PrevX = CenterX;
		CutterMap[cut].PrevY = CenterY;
		//Relevant spire is the last of the site's spires
		
		plan.PlanStack.Append(BehaviorManager->PlanData->GenPlan_GoConnectSpire(spireRef, 1000).PlanStack);
	}
	
	//First check for disconnect if wrong connection
	if (bWrongConnection)
	{
		AActor* disconnectSpire = SplineMechMap[cut].SpireRef;
		plan.PlanStack.Append(BehaviorManager->PlanData->GenPlan_GoDisconnectSpire(disconnectSpire, 1000).PlanStack);
	}

	return plan;
}

bool UCuttersManagerAgain::TryStartPioneerJob(AMech_Cutter* cut, TArray<AActor*>& RelevantSpires, int32 CenterX,
	int32 CenterY, FVector targetLoc)
{
	//ATransportSpire* spireRef = RelevantSpires[RelevantSpires.Num()-1];
	//

	//Sort relevant spires by distance if has a free arm
	TArray<AActor*> closestSpires;
	TArray<float> distance;
	for (auto spire : RelevantSpires)
	{
		//If no free arms, skip this one
		auto spInfo = GameMode->TransportManager->GetSpireInfo(spire);
		int32 maxClaims = 4;
		if (spInfo->bRefinerySpire) maxClaims = 3;
		if (spInfo->FreeArms.Num() == 0 || spInfo->armClaims == maxClaims)
		{
			continue;
		}
		
		float dist = FVector::Dist(targetLoc, spire->GetActorLocation());

		//Add if empty
		if (closestSpires.Num() == 0)
		{
			closestSpires.Add(spire);
			distance.Add(dist);
			continue;
		}

		int32 insertIx = 0;
		//Otherwise loop trhough
		for (int32 i = 0; i < closestSpires.Num(); i++)
		{
			insertIx = i;
			//If distance is less, insert this at the index
			if (dist < distance[i])
			{
				break;
			}
			//Increment last one if it failed to make it add at the end
			if (i == closestSpires.Num() -1)
				insertIx++;
		}
		//Insert result in appropriate place
		closestSpires.Insert(spire, insertIx);
		distance.Insert(dist, insertIx);
	}

	
	//If none have free arms, returns fail
	if (closestSpires.Num() == 0)
	{
		return false;
	}

	//Otherwise start pioneer job at closest
	FPlanStack plan = GenerateSiteJob(cut, closestSpires[0], CenterX, CenterY, targetLoc, AT_GetPioneerWork);
	BehaviorManager->SubmitAgentPlan(cut, plan);
	BehaviorManager->MakeAgentAct(cut);
	return true;
}

//Returns true if all done
bool UCuttersManagerAgain::StopAndCheckDone(AMech_Cutter* parent, AMech_Cutter_Limb* limb, bool bStretched)
{
	CutterMap[parent].WorkingLimbs.RemoveSwap(limb);

	//If current is stretched
	if (bStretched)
	{
		//Mark as such
		CutterMap[parent].StretchedLimbs.Add(limb);
	}

	if (CutterMap[parent].WorkingLimbs.Num() == 0)
	{
		//If any limbs just stretched...
		if (bStretched || CutterMap[parent].StretchedLimbs.Num() > 0)
		{
			//Empty that out
			//CutterMap[parent].StretchedLimbs.Empty();
			//Add a reorient behavior to plan
			BehaviorManager->SubmitAgentPlan(parent, BehaviorManager->PlanData->GenPlan_WaitAndReorient());
			//BehaviorManager->MakeAgentAct(parent);
		}
		//If parent has none working, now ready to move on
		else if (CutterMap[parent].WorkingLimbs.Num() == 0)
		{
			//Add wrap up instruction needs attending to
			FPlanStack plan = BehaviorManager->PlanData->GenAction(AT_MobileAfterHooks);
			BehaviorManager->SubmitAgentPlan(parent, FPlanStack(plan));
			//BehaviorManager->MakeAgentAct(parent);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("What"));
			return false;
		}

		return true;
	}
	return false;
}

void UCuttersManagerAgain::SendChips(AMech_Cutter* cut, float amtReceived, int32 &lastSentSocket)
{
	float curAmt = CutterMap[cut].ChipHoldings;

	//Check if spawn a bag
	if (curAmt + amtReceived > ChipCapacity_KG)
	{
		auto spActor = GameMode->TransportManager->GetConnectedSpline(cut);
		if (!spActor)
		{
			UE_LOG(LogTemp, Error, TEXT("No current spline!"));
			return;
		}
		//Spawn
		if (lastSentSocket == 0)
		{
			GameMode->TransportManager->SpawnHook(spActor, true);
			lastSentSocket = 1;
		}
		else
		{
			GameMode->TransportManager->SpawnHook(spActor, false);
			lastSentSocket = 0;
		}
		
		//Set to difference of total minus capacity
		curAmt = curAmt + amtReceived - ChipCapacity_KG;
	}
	else
	{
		//Set to amount with more KG
		curAmt = curAmt + amtReceived;
	}
	//Add to cutter holdings
	CutterMap[cut].ChipHoldings = curAmt;
}
