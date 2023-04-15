// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Data/ProgressGridData.h"
#include "GameFramework/Actor.h"
#include "MoriManager.generated.h"

class ADisharmonyGameMode;


class ALandscape;
class UTexture2D;
class UHierarchicalInstancedStaticMeshComponent;

UCLASS()
class UNSATISFACTORY_API AMoriManager : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY()
	ADisharmonyGameMode* GameMode;
public:	
	// Sets default values for this actor's properties
	AMoriManager();

	float GetMyceliumWeight(int32 x, int32 y);

	UFUNCTION(BlueprintCallable)
	bool KillTree(int32 index, int32 &type, float &age);
	FVector GetWorldLocByIndex(int32 index);
	FVector GetWorldLocByCoords(int32 x, int32 y);
	
	///Takes cell coordinates, desired direction, and tree patch resolution, returns a set of trees found by winding through this area
	TArray<int32> CreateTreePatchRes(int32 X, int32 Y, ETreePatchDirection direction, int32 height, int32 width, int32 offset);
	TArray<int32> CreatePioneerTreeLanes(TArray<int32> indicesX, TArray<int32> indicesY, int32 dX, int32 dY, int32 length);

	

protected:	
	void LoadData();
	void GenerateTrees();
	void InstantiateTrees();
	void GenerateMycelium();
	void GenerateWaterLevels();
	
	void TickMycelium(float DeltaTime);

	//Data
	UPROPERTY(EditAnywhere, Category = "Data References")
	UProgressGridData* ProgressGridData;

public:
	//Initialized via game mode
	void InitializeSystem();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void RegisterGameMode(ADisharmonyGameMode* gm)
	{
		GameMode = gm;
	}

	void WorldToMapCoords(int32 &x, int32 &y, int32 &curIndex);
	
	UPROPERTY(EditAnywhere, Category = "Temp")
	AActor* WaterPlaceholder = nullptr;
	UPROPERTY(EditAnywhere, Category = "Temp")
	bool bAdjustWater = true;

	UPROPERTY(EditAnywhere)
	bool bGenerate = true;
	
	UPROPERTY(EditAnywhere, Category = "Trees")
	float MaxTreeSize = 0.95;
	UPROPERTY(EditAnywhere, Category = "Trees")
	float LargeTreesDefinition = 0.65;
	UPROPERTY(EditAnywhere, Category = "Trees")
	float DeadChanceLarge = 0.3;
	UPROPERTY(EditAnywhere, Category = "Trees")
	float DeadStandingChanceLarge = 0.9;
	UPROPERTY(EditAnywhere, Category = "Trees")
	float DeadChanceSmall = 0.42;
	UPROPERTY(EditAnywhere, Category = "Trees")
	float DeadStandingChanceSmall = 0.4;
	UPROPERTY(EditAnywhere, Category = "Trees")
	float JitterScale_m = 4;

	UPROPERTY(EditAnywhere, Category = "Grid")
	int32 ReadRes = 4033;
	UPROPERTY(EditAnywhere, Category = "Grid")
	int32 Resolution = 4033;

	UPROPERTY(EditAnywhere, Category = "Grid")
	int32 HISM_Num_Side = 2;
	UPROPERTY()
	int32 HISM_Width = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Loading")
	FString DirectoryPath = "MapRaws/";
	UPROPERTY(EditAnywhere, Category = "Trees")
	float SSpruceDensityModifier = 1;
	UPROPERTY(EditAnywhere, Category = "Trees")
	float SSpruceCullRadius = 10;
	UPROPERTY(EditAnywhere, Category = "Trees")
	float DFirDensityModifier = 1;
	UPROPERTY(EditAnywhere, Category = "Trees")
	float DFirCullRadius = 8;
	UPROPERTY(EditAnywhere, Category = "Trees")
	float WHemlockDensityModifier = 1;
	UPROPERTY(EditAnywhere, Category = "Trees")
	float WHemlockCullRadius = 6;
	
	UPROPERTY(EditAnywhere, Category = "Map Loading")
	float VertexOffset = -201600.0;

	UPROPERTY(EditAnywhere, Category = "Map Loading")
	float HeightScale = 100;
	UPROPERTY(EditAnywhere, Category = "Map Loading")
	float WidthScale = 200;
	UPROPERTY(EditAnywhere, Category = "Map Loading")
	float HeightOffsetMultiplier = 0;
	
	UPROPERTY(EditAnywhere, Category = "References")
	AActor* LandscapeActor;

	UPROPERTY(EditAnywhere, Category = "Map Loading")
	UTexture2D* Data_Heightmap;

	UPROPERTY()
	TArray<UHierarchicalInstancedStaticMeshComponent*> HISM_SSpruce;
	UPROPERTY()
	TArray<UHierarchicalInstancedStaticMeshComponent*> HISM_DFir;
	UPROPERTY()
	TArray<UHierarchicalInstancedStaticMeshComponent*> HISM_WHemlock;
	UPROPERTY()
	TArray<UHierarchicalInstancedStaticMeshComponent*> HISM_SSpruce_Dead_S;
	UPROPERTY()
	TArray<UHierarchicalInstancedStaticMeshComponent*> HISM_DFir_Dead_S;
	UPROPERTY()
	TArray<UHierarchicalInstancedStaticMeshComponent*> HISM_WHemlock_Dead_S;


	UPROPERTY(EditAnywhere, Category = "Mycelium")
	float MyceliumDepletionTime = 360;
	UPROPERTY(EditAnywhere, Category = "Mycelium")
	float MyceliumGrowthTime = 720;
	UPROPERTY(EditAnywhere, Category = "Mycelium")
	float MyceliumDepletionMin = .1;	
	UPROPERTY(EditAnywhere, Category = "Mycelium")
	bool DebugMycelium = false;
	UPROPERTY(EditAnywhere, Category = "Mycelium")
	UHierarchicalInstancedStaticMeshComponent* HISM_MyceliumDebugger = nullptr;
	UPROPERTY(EditAnywhere, Category = "Mycelium")
	UMaterialInstance* MyceliumDebuggerMaterial;
	
	
protected:
UPROPERTY()
	bool bMarkFrameDirty = false;
	UPROPERTY()
	TArray<UHierarchicalInstancedStaticMeshComponent*> dirtyHISMs;

	
	//Persists
	UPROPERTY()
	TArray<float> VertexHeightMap;
	UPROPERTY()
	TArray<float>WaterLevelsMap;
	UPROPERTY()
	TArray<int8> TM_Type;
	UPROPERTY()
	TArray<int32> TM_InstanceIx;
	UPROPERTY()
	TArray<float> TM_Viability;
	UPROPERTY()
	TArray<float> TM_Age;
	//Acts as a tree map in the end, 1 is culled, 0/2/3 are trees. More accurate to use Type though?
	UPROPERTY()
	TArray<int8> TM_GrowthStageCullMap;
	UPROPERTY(BlueprintReadOnly)
	TArray<float> TM_MyceliumWeight;
	UPROPERTY()
	TMap<int32, float> M_DepletionIndices;

	UPROPERTY()
	TArray<int32> Keys;
	
	//Used for tree generation and deleted
	UPROPERTY()
	TArray<float> SSpruceViabilityMap;
	UPROPERTY()
	TArray<float> DFirViabilityMap;
	UPROPERTY()
	TArray<float> WHemlockViabilityMap;




};
