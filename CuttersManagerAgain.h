// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MechManager.h"
#include "Object.h"
#include "Mechs/Mech_Cutter.h"
#include "UObject/NoExportTypes.h"
#include "CuttersManagerAgain.generated.h"

class AAgentsBehaviorManager;
class ATransportSplineActor;
class AMech_Cutter_Limb;

USTRUCT()
struct FSplineMechInfo
{
	GENERATED_BODY()

	UPROPERTY()
	USceneComponent* oldArmRef = nullptr;
	UPROPERTY()
	USceneComponent* CurrentArmReference = nullptr;
	UPROPERTY()
	USceneComponent* CurrentLAnch = nullptr;
	UPROPERTY()
	USceneComponent* CurrentRAnch = nullptr;
	UPROPERTY()
	USceneComponent* LeftSocketRef = nullptr;
	UPROPERTY()
	USceneComponent* RightSocketRef = nullptr;
	
	
	UPROPERTY()
	AActor* SpireRef = nullptr;
};


USTRUCT()
struct FCutterInfos
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<AMech_Cutter_Limb*> Limbs;
	UPROPERTY()
	TArray<USceneComponent*> LimbAnchors;
	UPROPERTY()
	TArray<AMech_Cutter_Limb*> WorkingLimbs;
	UPROPERTY()
	TArray<AMech_Cutter_Limb*> StretchedLimbs;
	UPROPERTY()
	int32 PrevX = 0;
	UPROPERTY()
	int32 PrevY = 0;
	UPROPERTY()
	float ChipHoldings;
	UPROPERTY()
	TEnumAsByte<EActionTypes> currentJob;
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class UNSATISFACTORY_API UCuttersManagerAgain : public UObject, public IMechManager
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	ADisharmonyGameMode* GameMode = nullptr;
	UPROPERTY()
	AAgentsBehaviorManager* BehaviorManager = nullptr;
	
	UPROPERTY()
	TArray<AMech_Cutter*> JoblessCutters;
	
private:	
	UPROPERTY()
	TMap<AMech_Cutter*, FPlanAction> GetArm;
	UPROPERTY()
	TMap<AMech_Cutter*, FPlanAction> ConnectSpline;
	UPROPERTY()
	TMap<AMech_Cutter*, FPlanAction> DisconnectSpline;
	UPROPERTY()
	TMap<AMech_Cutter*, FPlanAction> GrabLinkIn;
	UPROPERTY()
	TMap<AMech_Cutter*, FPlanAction> MobileAfterHooks;
	UPROPERTY()
	TMap<AMech_Cutter*, FPlanAction> PioneerWork;
	UPROPERTY()
	TMap<AMech_Cutter*, FPlanAction> SiteWork;
	UPROPERTY()
	TMap<AMech_Cutter*, FPlanAction> LimbSenders;
	UPROPERTY()
	TMap<AMech_Cutter*, FPlanAction> Reorient;
	UPROPERTY()
	TMap<AMech_Cutter*, FPlanAction> InitWrap;
	UPROPERTY()
	TMap<AMech_Cutter*, FPlanAction> WrapPioneer;
	UPROPERTY()
	TMap<AMech_Cutter*, FPlanAction> WrapSite;
	UPROPERTY()
	TMap<AMech_Cutter*, FPlanAction> LayLink;
	UPROPERTY()
	TMap<AMech_Cutter*, FPlanAction> LaySite;

	//Cutters
	UPROPERTY()
	TMap<AMech_Cutter*, FCutterInfos> CutterMap;
	UPROPERTY()
	TMap<AGenericMech*, FSplineMechInfo> SplineMechMap;
	
	//Work tracking
	UPROPERTY()
	TArray<AMech_Cutter*> CheckLOSCutters;
	UPROPERTY()
	TArray<AMech_Cutter*> HookWaiters;
	
	//LOS variables
	UPROPERTY()
	TMap<AMech_Cutter*, FVector> CutterGoodLocationMap;
	UPROPERTY(EditAnywhere, Category="Cutter")
	FVector CutObstructionOffset = FVector(0,0,2000);
	UPROPERTY(EditAnywhere, Category="Cutter")
	float MaxDistFromSpire = 40000;
	UPROPERTY(EditAnywhere, Category="Cutter")
	float MinDistFromSpire = 10000;

	//Chip variables
	UPROPERTY(EditAnywhere, Category="Cutter")
	float ChipCapacity_KG = 40000;

	//BPs
	UPROPERTY(EditAnywhere, Category="Blueprint References")
	UClass* SpActorBPClass = nullptr;
	UPROPERTY(EditAnywhere, Category="Blueprint References")
	UClass* CutterLimbBP = nullptr;
	


public:
	//Override Functions
	virtual void PerformTick(float DeltaTime) override;
	virtual void CreateActionObject(FPlanAction action, AActor* agent) override;
	virtual TSet<AActor*> ExecuteAllActions() override;

	void InitializeCutter(AMech_Cutter* cut);
	void SpawnRegisterCutterLimbs(AMech_Cutter* cut, TArray<USceneComponent*> scenes);

	
private:
	//Action Functions
	bool GetFreeArm(FPlanAction action, AMech_Cutter* cut);
	bool ConnectSplines(FPlanAction action, AMech_Cutter* cut);
	bool DisconnectSplines(FPlanAction action, AMech_Cutter* cut);
	bool GrabLinkSpireOutgoing(FPlanAction action, AMech_Cutter* cut);
	bool IsMobileAfterHooks(FPlanAction action, AMech_Cutter* cut);
	bool GetPioneerWork(FPlanAction action, AMech_Cutter* cut);
	bool GetSiteWork(FPlanAction action, AMech_Cutter* cut);
	TArray<AActor*> SendOutLimbs(FPlanAction action, AMech_Cutter* cut);
	bool ReorientForLimbs(FPlanAction action, AMech_Cutter* cut);
	TArray<FPlanAction> RecurseUnwind(AActor* tailSpire);
	bool InitiateWrapUp(FPlanAction action, AMech_Cutter* cut);
	TArray<AActor*> WrapUpPioneerWork(FPlanAction action, AMech_Cutter* cut);
	TArray<AActor*> WrapUpSiteWork(FPlanAction action, AMech_Cutter* cut);
	bool LayLinkSpire(FPlanAction action, AMech_Cutter* cut);
	bool LaySiteSpire(FPlanAction action, AMech_Cutter* cut);
	
	//Related Functions
	void RegisterCurrentArm(AGenericMech* mech, USceneComponent* arm);
	bool ConnectSplines(AGenericMech* mech, AActor* spire);
	void TransferNewSpireSplines(AGenericMech* mech);
	void LoadPioneerWork(AMech_Cutter* cut, FVector& attendLoc);
	bool LoadSiteWork(AMech_Cutter* cut, FVector& attendLoc);
	void MakeAllLimbsFollow(AMech_Cutter* cut, TArray<AActor*> &leftoverAgents);

	//Update Functions
	void UpdateCheckSpireLOS(TArray<AMech_Cutter*> cutterRemoves);
	void UpdateHookWaiters();

public:
	FPlanStack GenerateSiteJob(AMech_Cutter* cut, AActor* spireRef, int32 CenterX, int32 CenterY, FVector siteLoc, EActionTypes
					siteType);
	bool TryStartPioneerJob(AMech_Cutter* cut, TArray<AActor*> &RelevantSpires, int32 CenterX, int32 CenterY, FVector targetLoc);
	bool StopAndCheckDone(AMech_Cutter* parent, AMech_Cutter_Limb* limb, bool bStretched);
	void SendChips(AMech_Cutter* cut, float amtReceived, int32 &lastSentSocket);
};
