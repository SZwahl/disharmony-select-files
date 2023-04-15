// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Data/PlanData.h"
#include "GameFramework/Actor.h"
#include "Data/ProgressGridData.h"
#include "CitadelBrainManager.generated.h"


class UPlanData;
class AGenericMech;
struct FCutterQueue;
class ATransportSpire;
class ADisharmonyGameMode;
class AMech_Weaver;
class AMech_Cutter;


USTRUCT()
struct FFrontierInfo
{
	GENERATED_BODY()
	UPROPERTY()
	TArray<int32> SiteIx;
	UPROPERTY()
	TArray<int32> EstimatedMass;
	UPROPERTY()
	TArray<int32> PrevSiteID;
};

USTRUCT()
struct FNeighborList
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<int32> Neighbors;
};

USTRUCT()
struct FPioneerRetryData
{
	GENERATED_BODY()

	UPROPERTY()
	float time;
	UPROPERTY()
	int32 startIx;
	UPROPERTY()
	int32 targetIx;
	UPROPERTY()
	int32 pioneerX;
	UPROPERTY()
	int32 pioneerY;
};

USTRUCT()
struct FSiteDemandInfo
{
	GENERATED_BODY()

	UPROPERTY()
	int32 StartSite;
	UPROPERTY()
	int32 EndSite;
	UPROPERTY()
	FVector StartLoc;
	UPROPERTY()
	FVector EndLoc;
};

USTRUCT()
struct FProductionSitePlan
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Progress = 0;

	UPROPERTY()
	FSiteDemandInfo DemandInfo;
};

UCLASS()
class UNSATISFACTORY_API ACitadelBrainManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACitadelBrainManager();
	//Initialzied by game mode
	void InitializeSystem();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void RegisterGameMode(ADisharmonyGameMode* gm)
	{
		GameMode = gm;
	}

protected:
	
	UPROPERTY()
	ADisharmonyGameMode* GameMode;

	UPROPERTY(EditAnywhere)
	bool bWork = true;
	UPROPERTY(EditAnywhere)
	bool bAllSitesCanBeFactories = true;
	
	UPROPERTY()
	float RunningTime = 0;
	

	//Sorted least to most desirable
	UPROPERTY()
	FFrontierInfo Frontier;
	
	//-----------Sites
	//Site Data
	UPROPERTY(EditAnywhere, Category = "Sites")
	int32 CuttersPerSite;
	UPROPERTY(EditAnywhere, Category = "Sites")
	int32 FactoryCandidateBigNeighborThreshold = 3;
	UPROPERTY(EditAnywhere, Category = "Sites")
	int32 BigNeighborTreeCount = 50;

	
	//Sites about to be ready for work
	UPROPERTY()
	TArray<int32> PendingSites;
	UPROPERTY()
	TArray<int32> ActiveSites;
	UPROPERTY()
	TArray<int32> FinishedSites;

	//UPROPERTY()
	//TMap<int32, FNeighborList> FinishedNeighbors;

	//Cutter Demands
	UPROPERTY()
	TArray<int32> Demand_C_Site;
	UPROPERTY()
	TArray<int32> Demand_C_Pioneer;
	UPROPERTY()
	TMap<int32, int32> Demand_C_PioneerPrev;

	//Unit Build Demands
	UPROPERTY()
	int32 Demand_U_Cutter;
	UPROPERTY()
	int32 FormingCutters;
	//UPROPERTY()
	//int32 Demand_U_Weaver;
	UPROPERTY()
	int32 FormingWeavers;

	//Building Build Demands
	UPROPERTY()
	TArray<FSiteDemandInfo> Demand_W_ProductionSite;
	
	//Mech tracking
	UPROPERTY()
	TMap<AMech_Cutter*, int32> SiteCutters;
	UPROPERTY()
	TMap<AMech_Cutter*, int32> PioneerCutterEnds;
	UPROPERTY()
	TMap<AMech_Cutter*, int32> PioneerCutterStarts;

	//Weavers building production sites
	UPROPERTY()
	TMap<int32, FProductionSitePlan> ProductionSitePlan;
	UPROPERTY()
	int32 ProdPlansCreated = 0;
	
	UPROPERTY(EditAnywhere)
	float RetryPioneerTimer = 60;
	UPROPERTY()
	TMap<AMech_Cutter*, FPioneerRetryData> RetryForlornPioneers;
	
	
	UPROPERTY()
	TMap<AMech_Cutter*, bool> CutterSplineReady; 

	//Data
	UPROPERTY(EditAnywhere, Category = "Data References")
	UProgressGridData* ProgressGridData;
	UPROPERTY(EditAnywhere, Category="Data References")
	UPlanData* PlanData;


protected:
	//Update functions
	void UpdateJoblessCutters(TArray<AMech_Cutter*>& Removed);
	void UpdateSiteDemand();
	void UpdateUnitDemand();
	void UpdateWeaverCreationInstructions();
	void UpdateForlornPioneers(float DeltaTime);

	TArray<int32> GetClosestDemandSites(int32 x, int32 y, bool bIsPioneer, int32 considerNum);
	
	int32 AddNeighborSitesToFrontier(int32 sIx);
	void AddSiteToFrontier(int32 sIx, int32 prevIx, int32& estMass);

	void MarkSiteHalfFinished(int32 ix);
	void MarkSiteFullyFinished(int32 ix);
	
public:
	//Waypoint stuff
	//TArray<FVector> GetWaypointPath(FVector start, FVector end, float simplePathRange);
	// int32 GetClosestFinishedSiteCell(int32 x, int32 y);
	// TArray<FVector> AStarFinishedSites(int32 startIx, int32 endIx);
	// int32 WaypointHeuristic(int32 curIx, int32 neighIx);
	
	//Update forming things
	int32 GetFormingCutters(){return FormingCutters;}
	void SetFormingCutters(int32 f){FormingCutters = f;}

	//Mark work finished
	void MarkPioneeringFinished(AMech_Cutter* cut, AActor* attachedSpire, int32 &centerX, int32 &centerY, FVector &worldLoc);
	void MarkSiteCutterFinished(AMech_Cutter* cut);

	// void SubmitSiteWorkWeaver(FProductionSitePlan plan, AMech_Weaver* weav);
	// void MarkSiteWorkWeaverReady(AMech_Weaver* weav);

	void TryAddSpireToSite(AActor* spire, AMech_Cutter* mech);


	//Get work
	bool CutterSiteHasNext(AMech_Cutter* cut, FVector& attendLoc, int32& prevX, int32& prevY, ETreePatchDirection& dir);
	void GetCutterSiteWork(int32 limbIx, int32 limbTotal,  int32 x, int32 y, ETreePatchDirection dir, TArray<int32>& trees);

	//Tree stuff
	void PopulateCornerIndices(int32 dX, int32 dY, int32 cornerX, int32 cornerY, TArray<int32>& indicesX, TArray<int32>& indicesY, int32 span);
	void PopulateOrthogonalIndices(int32 dX, int32 dY, int32 cornerX, int32 cornerY, TArray<int32>& indicesX, TArray<int32>& indicesY, int32 span);
	void GatherScrapTrees(bool diagonal, bool firstHalf, TArray<FCutterQueue>& cutterQueues, int32 dX, int32 dY, int32 span, TArray<int32> halfIxX, TArray<int32> halfIxY);
	void GetCutterPioneerWork(AMech_Cutter* cut, TArray<FCutterQueue>& cutterQueues, FVector& attendLoc);

	

	//FPlanStack GetNextProductionPlan(int32 planID, AMech_Weaver* weaver);
	
};
