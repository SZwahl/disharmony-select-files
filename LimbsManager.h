// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MechManager.h"
#include "Object.h"
#include "LimbsManager.generated.h"

class UTreePropertyDatabase;
class ACuttingTree;
class AMech_Cutter;
class AMech_Cutter_Limb;
class AAgentsBehaviorManager;
class ADisharmonyGameMode;

USTRUCT()
struct FCutterQueue
{
	GENERATED_BODY()

	UPROPERTY()
	int32 CurrentTree = -1;
	UPROPERTY()
	FVector TreeLoc;
	UPROPERTY()
	TArray<int32> trees;
};

USTRUCT()
struct FGrindingTrees
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<AMech_Cutter_Limb*> Cutters;
	UPROPERTY()
	TArray<int32> LastSent;
	UPROPERTY()
	TArray<ACuttingTree*> Mesh;
	UPROPERTY()
	TArray<FVector> TreeLoc;
	UPROPERTY()
	TArray<float> Height;
	UPROPERTY()
	TArray<float> KG_Total;
	UPROPERTY()
	TArray<float> KG_Remaining;
	UPROPERTY()
	TArray<int32> SectionAmount;
	UPROPERTY()
	TArray<int32> HiddenSections;
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class UNSATISFACTORY_API ULimbsManager : public UObject, public IMechManager
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	ADisharmonyGameMode* GameMode = nullptr;
	UPROPERTY()
	AAgentsBehaviorManager* BehaviorManager = nullptr;

	UPROPERTY()
	TMap<AMech_Cutter_Limb*, USceneComponent*> FollowingLimbs;
	UPROPERTY()
	TMap<AMech_Cutter_Limb*, AMech_Cutter*> LimbParents;
	UPROPERTY()
	TMap<AMech_Cutter_Limb*, FCutterQueue> LimbQueues;

private:	
	UPROPERTY()
	TMap<AMech_Cutter_Limb*, FPlanAction> Follow;
	UPROPERTY()
	TMap<AMech_Cutter_Limb*, FPlanAction> NextTree;
	UPROPERTY()
	TMap<AMech_Cutter_Limb*, FPlanAction> Grind;

	UPROPERTY(EditAnywhere)
	bool bUltraFastCutting = false;
	
	//Grind variables
	UPROPERTY(EditAnywhere, Category="Limbs")
	float LimbMaxDistance = 12000.0f;
	UPROPERTY(EditAnywhere)
	float GrindRate_KGperSecond = 8000;

	//Stretch variables
	UPROPERTY(EditAnywhere, Category="Limbs")
	float LimbAnchorUpdateTime = 1.5f;
	UPROPERTY()
	float LimbAnchorTimer = 0;
	
	//BPs
	UPROPERTY(EditAnywhere, Category="Blueprint References")
	UClass* TreeBP = nullptr;
	
	//Data
	UPROPERTY(EditAnywhere, Category="Data References")
	UTreePropertyDatabase* TreePropData;
	
	//Work tracking
	UPROPERTY()
	FGrindingTrees GrindingTrees;
	
public:
	//Override Functions
	virtual void PerformTick(float DeltaTime) override;
	virtual void CreateActionObject(FPlanAction action, AActor* agent) override;
	virtual TSet<AActor*> ExecuteAllActions() override;

private:
	//Action Functions
	bool FollowCutter(FPlanAction action, AMech_Cutter_Limb* limb);
	TArray<AActor*> TryGetNextTree(FPlanAction action, AMech_Cutter_Limb* limb);
	bool GrindTree(FPlanAction action, AMech_Cutter_Limb* limb);
	
	//Related Functions
	bool TryStartGrinding(AMech_Cutter_Limb* cut);
	void RemoveGrindingTreeAtIndex(int32 ix);
	
	//Update Functions
	void UpdateFollowingLimbs(float DeltaTime);
	void UpdateGrindingLimbs(float DeltaTime);
};
