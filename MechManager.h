// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interface.h"
#include "Data/PlanData.h"
#include "MechManager.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UMechManager : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class UNSATISFACTORY_API IMechManager
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void PerformTick(float DeltaTime) =0;
	virtual void CreateActionObject(FPlanAction action, AActor* agent) =0;
	virtual TSet<AActor*> ExecuteAllActions() =0;
};
