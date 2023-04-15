// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DisharmonyGameMode.h"
#include "GameFramework/Actor.h"
#include "Data/PlanData.h"
#include "AgentsBehaviorManager.generated.h"


class AMech_Weaver;
class AMech_Cutter;
class ADisharmonyGameMode;
class UWeaverManager;
class ULimbsManager;
class UCuttersManagerAgain;
UCLASS()
class UNSATISFACTORY_API AAgentsBehaviorManager : public AActor
{
	GENERATED_BODY()
	UPROPERTY()
	ADisharmonyGameMode* GameMode;

	UPROPERTY()
	UCuttersManagerAgain* CutterManager;
	UPROPERTY()
	ULimbsManager* LimbsManager;
	UPROPERTY()
	UWeaverManager* WeaverManager;
	
	UPROPERTY(EditAnywhere)
	bool WOrkIt = true;
	
	//Mech Behaviors
	UPROPERTY(VisibleAnywhere)
	TMap<AActor*, FPlanStack> AgentPlans;	
	UPROPERTY()
	TSet<AActor*> NeedPlanInitiate;
	
	UPROPERTY(EditAnywhere)
	bool DebugHelpers = false;

	UPROPERTY()
	TMap<AActor*, FActionLog> ActionLogs;

	//BPs
	UPROPERTY(EditAnywhere, Category="Blueprint References")
	UClass* CutterManagerBP = nullptr;
	UPROPERTY(EditAnywhere, Category="Blueprint References")
	UClass* LimbsManagerBP = nullptr;
	UPROPERTY(EditAnywhere, Category="Blueprint References")
	UClass* WeaverManagerBP = nullptr;
	
public:	
	// Sets default values for this actor's properties
	AAgentsBehaviorManager();

	UPROPERTY()
	TMap<AActor*, FPlanAction> GoingToLoc;
	UPROPERTY()
	TMap<AActor*, FPlanAction> GoingToObj;

	UPROPERTY()
	TArray<AActor*> AgentsPendingMovement;

	TArray<AMech_Cutter*>* GetJoblessCutters();
	//Setters
	void SubmitAgentPlan(AActor* agent, FPlanStack ps)
	{
		AgentPlans[agent].PlanStack.Append(ps.PlanStack);
	}

	UPROPERTY(EditAnywhere)
	bool WorkSuperFast = false;
	
	UPROPERTY(EditAnywhere, Category="Blueprint References")
	UClass* CutterBP = nullptr;
	UPROPERTY(EditAnywhere, Category="Temp")
	TArray<AActor*> StartingCutters;
	
	UPROPERTY(EditAnywhere, Category="Data References")
	UPlanData* PlanData;
	
	UPROPERTY(EditAnywhere, Category="Starting Units")
	TArray<AMech_Weaver*> StartingWeavers;

	UCuttersManagerAgain* GetCutterManager() {return CutterManager;}
	ULimbsManager* GetLimbsManager() {return LimbsManager;}
	UWeaverManager* GetWeaverManager() {return WeaverManager;}

private:
	void UpdateNeedsPlanInitiation();

	TSet<AActor*> ExecuteGenericActions();
	
	bool GoToLoc(FPlanAction action, AActor* agent);
	bool GoToObj(FPlanAction action, AActor* agent);

public:
	//Initialized via game mode
	void InitializeSystem();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void RegisterGameMode(ADisharmonyGameMode* gm)
	{
		GameMode = gm;
	}

	void RegisterAgent(AActor* agent)
	{
		AgentPlans.Add(agent, FPlanStack());
	}
	void MakeAgentAct(AActor* agent);
	void MarkAgentDoneMoving(AActor* agent);
	void AppendMechInstructions(TArray<FPlanAction> plan, AActor* agent);
	
	//Utility?
	bool ExtendAndRotateVector(FVector& result, FVector start, FVector end, float rExtent, float rAngle);
};
