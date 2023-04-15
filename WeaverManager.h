// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MechManager.h"
#include "Object.h"
#include "Data/FactoryStructs.h"
#include "Data/PipeStructs.h"
#include "WeaverManager.generated.h"

class AMech_Weaver;
class AAgentsBehaviorManager;
class ADisharmonyGameMode;


USTRUCT()
struct FPumpLayStartingInfo
{
	GENERATED_BODY()

	FPumpLayStartingInfo(){};
	FPumpLayStartingInfo(EPumpType type, FVector endLoc, int32 startSite, bool bOil, bool bWater)
	{
		PumpType = type;
		EndLoc = endLoc;
		StartSiteIx = startSite;
		bIsOil = bOil;
		bIsWater = bWater;
	}

	UPROPERTY()
	TEnumAsByte<EPumpType> PumpType = PT_Garden;
	UPROPERTY()
	FVector EndLoc = FVector::ZeroVector;
	UPROPERTY()
	int32 StartSiteIx = -1;
	UPROPERTY()
	bool bIsOil = false;
	UPROPERTY()
	bool bIsWater = false;
};

USTRUCT()
struct FWeaverPumpJobs
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPumpLayStartingInfo> Jobs;

	UPROPERTY()
	int32 lastPumpLain = -1;
};

USTRUCT()
struct FUmbLayInfo
{
	GENERATED_BODY()

	FUmbLayInfo(){};
	FUmbLayInfo(FTransform t, int32 id)
	{
		T = t;
		SiteID = id;
	}
	
	UPROPERTY()
	FTransform T = FTransform();
	UPROPERTY()
	int32 SiteID = -1;
};

USTRUCT()
struct FWeaverUmbilicalJobs
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FUmbLayInfo> Jobs;
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class UNSATISFACTORY_API UWeaverManager : public UObject, public IMechManager
{
	GENERATED_BODY()

public:
	UPROPERTY()
	ADisharmonyGameMode* GameMode = nullptr;
	UPROPERTY()
	AAgentsBehaviorManager* BehaviorManager = nullptr;

private:
	UPROPERTY()
	TMap<AMech_Weaver*, FPlanAction> LayingRails;
	UPROPERTY()
	TMap<AMech_Weaver*, FPlanAction> CheckingUmbJob;
	UPROPERTY()
	TMap<AMech_Weaver*, FPlanAction> SpawningUmbilicals;
	UPROPERTY()
	TMap<AMech_Weaver*, FPlanAction> FinishingUmiblicals;
	UPROPERTY()
	TMap<AMech_Weaver*, FPlanAction> SeedingWombs;
	UPROPERTY()
	TMap<AMech_Weaver*, FPlanAction> BecomingIdle;
	UPROPERTY()
	TMap<AMech_Weaver*, FPlanAction> LayingPipe;
	UPROPERTY()
	TMap<AMech_Weaver*, FPlanAction> CheckingNextPipe;
	UPROPERTY()
	TMap<AMech_Weaver*, FPlanAction> BuildingPump;
	UPROPERTY()
	TMap<AMech_Weaver*, FPlanAction> CheckPumpJob;

	//Weavers
	//UPROPERTY()
	//TMap<AMech_Weaver*, FRailLayInfo> RailWeaverMap;
	UPROPERTY()
	TMap<AMech_Weaver*, FPipelineLayPackage> PipeWeaverMap;
	UPROPERTY()
	TMap<AMech_Weaver*, FSeedInstruction> SeedWeaverMap;

	UPROPERTY()
	TMap<AMech_Weaver*, FWeaverPumpJobs> WeaverPumpJobs;
	UPROPERTY()
	TMap<AMech_Weaver*, int32> LastPumpBuilt; 
	UPROPERTY()
	TMap<AMech_Weaver*, FWeaverUmbilicalJobs> WeaverUmbilicalJobs;
	UPROPERTY()
	TMap<AMech_Weaver*, FUmbLayInfo> WeaverUmbilicalInfo;
	
	//BPs
	UPROPERTY(EditAnywhere, Category="Blueprint References")
	UClass* RailBP = nullptr;

public:
	//Override Functions
	virtual void PerformTick(float DeltaTime) override;
	virtual void CreateActionObject(FPlanAction action, AActor* agent) override;
	virtual TSet<AActor*> ExecuteAllActions() override;

	void InitializeWeaver(AMech_Weaver* weav);

private:
	//Action Functions
	bool LayRail(FPlanAction action, AMech_Weaver* weav);
	bool CheckUmbJob(FPlanAction action, AMech_Weaver* weav);
	bool SpawnUmbilicals(FPlanAction action, AMech_Weaver* weav);
	bool FinishUmbilicals(FPlanAction action, AMech_Weaver* weav);
	bool SeedWomb(FPlanAction action, AMech_Weaver* weav);
	bool BecomeIdleAttendant(FPlanAction action, AMech_Weaver* weav);
	bool LayPipe(FPlanAction action, AMech_Weaver* weav);
	bool CheckNextPipe(FPlanAction action, AMech_Weaver* weav);
	bool BuildPump(FPlanAction action, AMech_Weaver* weav);
	bool CheckPumpJobs(FPlanAction action, AMech_Weaver* weav);

	//Related Functions
	//bool RegisterFinishedPump(AMech_Weaver* weav);
	//void LayRefineryRail(AMech_Weaver* weaver);
	
	//Update Functions

public:
	void EnactBuildInstructions(TArray<AMech_Weaver*> weavers, TArray<FVector> destinations, TArray<FString> names, TArray<FPlanStack> startInsts, TArray<int32> siteIx);
	void AddSeedInstruction(AMech_Weaver* weav, FVector dest, FString name);
	FPlanStack GenerateRefineryPlan(AMech_Weaver* weav, FVector sitePumpTarget, int32 startSite, int32 endSite);
private:
	FPlanStack GeneratePumpPlan(AMech_Weaver* weav, EPumpType type, FVector endPoint, int32 startSite, int32 endSite, bool bOil, bool
	                            bWater);

};
