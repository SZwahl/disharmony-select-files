// Fill out your copyright notice in the Description page of Project Settings.


#include "MoriManager.h"

#include <cctype>


#include "DrawDebugHelpers.h"
#include "Engine/Texture2D.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Misc/FileHelper.h"

// Sets default values
AMoriManager::AMoriManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

float AMoriManager::GetMyceliumWeight(int32 x, int32 y)
{
	int32 curIndex = -1;

	WorldToMapCoords(x, y, curIndex);
	
	if (curIndex >= 0 && curIndex < TM_MyceliumWeight.Num())
	{
		//UE_LOG(LogTemp, Warning, TEXT("X: %d, Y: %d, Value: %f"), translatedX, translatedY, TM_MyceliumWeight[index])
		return TM_MyceliumWeight[curIndex];
	}
	else return 0;
}

//Removes tree HISM and affects mycelium
bool AMoriManager::KillTree(int32 index, int32 &type, float &age)
{
	if (index == -1)
	{
		UE_LOG(LogTemp, Error, TEXT("Tree index messed up big time"))
		return false;
	}
	
	int32 x = index % Resolution;
	int32 y = (index - x) / Resolution;
	int8 treeType = TM_Type[index];
	type = treeType;
	age = TM_Age[index];

	//Get HISM index
	int32 hism_x = x / HISM_Width;
	int32 hism_y = y / HISM_Width;
	int32 hism_ix = hism_y * HISM_Num_Side + hism_x;

	if (type == -1)
	{
		UE_LOG(LogTemp, Error, TEXT("Killing tree that doesn't exist?"))
		return false;
	}
	
	UHierarchicalInstancedStaticMeshComponent* ptrHISM = nullptr;

	//Get the right HISM pointer
	switch (treeType)
	{
	case 1:
		ptrHISM = HISM_SSpruce[hism_ix];
		break;
	case 2:
		ptrHISM = HISM_DFir[hism_ix];
		break;
	case 3:
		ptrHISM = HISM_WHemlock[hism_ix];
		break;
	case 5:
		ptrHISM = HISM_SSpruce_Dead_S[hism_ix];
		break;
	case 6:
		ptrHISM = HISM_DFir_Dead_S[hism_ix];
		break;
	case 7:
		ptrHISM = HISM_WHemlock_Dead_S[hism_ix];
		break;
	case 9:
		ptrHISM = HISM_SSpruce_Dead_S[hism_ix];
		break;
	case 10:
		ptrHISM = HISM_DFir_Dead_S[hism_ix];
		break;
	case 11:
		ptrHISM = HISM_WHemlock_Dead_S[hism_ix];
		break;
	default:
		break;
	}

	//Something is wrong if no pointer
	if (!ptrHISM)
	{
		UE_LOG(LogTemp, Error, TEXT("Killing a tree without proper HISM!"))
		return false;
	}


	//Transform is same as old scaled to 0
	FTransform newT;
	ptrHISM->GetInstanceTransform(TM_InstanceIx[index], newT, true);
	newT.SetScale3D(FVector(0,0,0));

	//Update transform and mark frame dirty
	ptrHISM->UpdateInstanceTransform(TM_InstanceIx[index], newT, true, false, true);
	//bMarkFrameDirty = true;
    //	if (!dirtyHISMs.Contains(ptrHISM))
    //			dirtyHISMs.Add(ptrHISM);
	TM_InstanceIx[index] = -1;
	
	//Set radius to check for neighbors by species
	int32 nRadius = 0;
	if (treeType == 1 || treeType == 5 || treeType == 9)
		nRadius = SSpruceCullRadius;
	else if (treeType == 2 || treeType == 6 || treeType == 10)
		nRadius = DFirCullRadius;
	else if (treeType == 3 || treeType == 7 || treeType == 11)
		nRadius = WHemlockCullRadius;

	nRadius *= TM_Age[index];
	//Check every neighbor and damage mycelium integrity
	for (int i = x-nRadius; i < x+nRadius; i++)
	{
		for (int j = y-nRadius; j < y+nRadius; j++)
		{
			int32 nIndex = j * Resolution + i;
			
			//Skip if OOB
			if (nIndex < 0 || nIndex > (Resolution * Resolution))
				continue;

			//Damage the mycelium
			M_DepletionIndices.Add(nIndex, TM_MyceliumWeight[nIndex] * .3);
		}
	}

	//Set to stump
	TM_Type[index] = -1;
	TM_GrowthStageCullMap[index] = -1;

	return true;
}

FVector AMoriManager::GetWorldLocByIndex(int32 index)
{
	int32 i = index % Resolution;
	int32 j = (index - i) / Resolution;
	
	return FVector((i * WidthScale) + VertexOffset,(j * WidthScale) + VertexOffset, VertexHeightMap[j * Resolution + i]);
}

TArray<int32> AMoriManager::CreateTreePatchRes(int32 X, int32 Y, ETreePatchDirection direction, int32 height, int32 width, int32 offset)
{
	//Swap to bottom left corner of map res space
	X *= height;
	Y *= height;

	TArray<int32> treesInOrder;

	int32 curIndex = 0;

	//Add present trees to array in winding order
	switch (direction)
	{
		case ETreePatchDirection::TPD_TopBottom:
			{
				 X += offset;
				//Wind top to bottom
				for (int j = Y+height-1; j >= Y; j--)
				{
					for (int i = X; i < X + width; i++)
					{
						int32 newi = i;
						
						//If odd, descend down the X
						if (j % 2 == 1)
						{
							newi = X + width - ((i % width) +1);
						}
						
						curIndex = j * Resolution + newi;
						//Tree is present, add index
						if (TM_Type[curIndex] > 0)
							treesInOrder.Add(curIndex);
					}
				}	
				break;
			}
		case ETreePatchDirection::TPD_BottomTop:
			{
				X += offset;
				//Wind bottom to top
				for (int j = Y; j < Y+height; j++)
				{
					for (int i = X; i < X + width; i++)
					{
						int32 newi = i;
						
						//If odd, descend down the X
						if (j % 2 == 1)
						{
							newi = X + width - ((i % width) +1);
						}
						
						curIndex = j * Resolution + newi;
						//Tree is present, add index
						if (TM_Type[curIndex] > 0)
							treesInOrder.Add(curIndex);
					}
				}
				break;
			}
		case ETreePatchDirection::TPD_LeftRight:
			{
				Y += offset;
				//Wind left to right
				for (int i = X; i < X + height; i++)
				{
					for (int j = Y; j < Y+width; j++)
					{
						int32 newj = j;
						
						//If odd, descend down the X
						if (i % 2 == 1)
						{
							newj = Y + width - ((j % width) +1);
						}
						
						curIndex = newj * Resolution + i;
						//Tree is present, add index
						if (TM_Type[curIndex] > 0)
							treesInOrder.Add(curIndex);
					}
				}
				break;
			}
		case ETreePatchDirection::TPD_RightLeft:
			{
				Y += offset;
				//Wind right to left
				for (int i = X + height -1; i >= X; i--)
				{
					for (int j = Y; j < Y+width; j++)
					{
						int32 newj = j;
						
						//if odd, descend down the X
						 if (i % 2 == 1)
						 {
						 	newj = Y + width - ((j % width) +1);
						 }
						
						curIndex = newj * Resolution + i;
						//Tree is present, add index
						if (TM_Type[curIndex] > 0)
							treesInOrder.Add(curIndex);
					}
				}
				break;
			}
		default:
			break;
	}


	return treesInOrder;
}

TArray<int32> AMoriManager::CreatePioneerTreeLanes(TArray<int32> indicesX, TArray<int32> indicesY, int32 dX, int32 dY, int32 length)
{
	TArray<int32> treesFound;

	int32 curX;
	int32 curY;
	int32 curIx;
	//for length of path
	for (int32 i = 0; i < length; i++)
	{
		//check each index with directional offset for a tree, add if there
		for (int32 j = 0; j < indicesX.Num(); j++)
		{
			curX = indicesX[j] + dX*1 + dX*i;
			curY = indicesY[j] + dY*1 + dY*i;


			curIx = curY * Resolution + curX;
			//Tree is present, add index
			if (TM_Type[curIx] > 0)
				treesFound.Add(curIx);
		}
	}

	return treesFound;
}

FVector AMoriManager::GetWorldLocByCoords(int32 x, int32 y)
{
	return FVector((x * WidthScale) + VertexOffset,(y * WidthScale) + VertexOffset, VertexHeightMap[y * Resolution + x]);
}

void AMoriManager::LoadData()
{
	UE_LOG(LogTemp, Warning, TEXT("Loading data..."));
	
	//Ready buffers
	TArray<uint8> TerrainBuffer;
	TArray<uint8> SSpruceBuffer;
	TArray<uint8> DFirBuffer;
	TArray<uint8> WHemlockBuffer;
	TArray<uint8> WaterLevelsBuffer;

	//Allocate for viability maps
	VertexHeightMap.Reserve(ReadRes * ReadRes);
	SSpruceViabilityMap.Reserve(ReadRes * ReadRes);
	DFirViabilityMap.Reserve(ReadRes * ReadRes);
	WHemlockViabilityMap.Reserve(ReadRes * ReadRes);
	WaterLevelsMap.Reserve(ReadRes*ReadRes);
	
	//For each float in the .r32, put default
	for (int i = 0; i < ReadRes; i++)
	{
		for (int j = 0; j < ReadRes; j++)
		{
			VertexHeightMap.Add(0);
			SSpruceViabilityMap.Add(0);
			DFirViabilityMap.Add(0);
			WHemlockViabilityMap.Add(0);
			WaterLevelsMap.Add(-1);

			TM_Type.Add(0);
			TM_InstanceIx.Add(-1);
			TM_Viability.Add(0);
			TM_Age.Add(-1);
			TM_GrowthStageCullMap.Add(0);
			TM_MyceliumWeight.Add(0);
		}
	}

	//Load terrain
	FString projectDir = FPaths::ProjectContentDir();
	DirectoryPath = projectDir + DirectoryPath;
	FString TerrainPath = DirectoryPath + "Terrain32.r32";
	IPlatformFile& terrainfile = FPlatformFileManager::Get().GetPlatformFile();
	if (terrainfile.FileExists(*TerrainPath)) {
		FFileHelper::LoadFileToArray(TerrainBuffer, *TerrainPath);
	}
	//Load spruce viability
	FString SSprucePath = DirectoryPath + "SSpruce.r32";
	IPlatformFile& spruceFile = FPlatformFileManager::Get().GetPlatformFile();
	if (spruceFile.FileExists(*SSprucePath)) {
		FFileHelper::LoadFileToArray(SSpruceBuffer, *SSprucePath);
	}
	//Load douglas fir viability
	FString DFirPath = DirectoryPath + "DFir.r32";
	IPlatformFile& firFile = FPlatformFileManager::Get().GetPlatformFile();
	if (firFile.FileExists(*DFirPath)) {
		FFileHelper::LoadFileToArray(DFirBuffer, *DFirPath);
	}
	//Load western hemlock viability
	FString WHemlockPath = DirectoryPath + "WHemlock.r32";
	IPlatformFile& hemlockFile = FPlatformFileManager::Get().GetPlatformFile();
	if (hemlockFile.FileExists(*WHemlockPath)) {
		FFileHelper::LoadFileToArray(WHemlockBuffer, *WHemlockPath);
	}

	//Load western hemlock viability
	FString WaterLevelsPath = DirectoryPath + "WaterPresence.r32";
	IPlatformFile& waterFile = FPlatformFileManager::Get().GetPlatformFile();
	if (hemlockFile.FileExists(*WaterLevelsPath)) {
		FFileHelper::LoadFileToArray(WaterLevelsBuffer, *WaterLevelsPath);
	}
	
	//For each float in the .r32, load terrain and viability into respective maps
	for (int i = 0; i < ReadRes; i++)
	{
		for (int j = 0; j < ReadRes; j++)
		{
			//Modify height by parameters and assign
			float rawHeight = 0;
			FMemory::Memcpy(&rawHeight, &TerrainBuffer[j * ReadRes * 4 + (i * 4)] , 4);
			VertexHeightMap[j * ReadRes + i] = rawHeight * HeightScale + HeightOffsetMultiplier;
			//Assign viabilities
			FMemory::Memcpy(&SSpruceViabilityMap[j * ReadRes + i], &SSpruceBuffer[j * ReadRes * 4 + (i * 4)], 4);
			FMemory::Memcpy(&DFirViabilityMap[j * ReadRes + i], &DFirBuffer[j * ReadRes * 4 + (i * 4)], 4);
			FMemory::Memcpy(&WHemlockViabilityMap[j * ReadRes + i], &WHemlockBuffer[j * ReadRes * 4 + (i * 4)], 4);
			FMemory::Memcpy(&rawHeight, &WaterLevelsBuffer[j * ReadRes * 4 + (i * 4)], 4);
			WaterLevelsMap[j * ReadRes + i] = rawHeight * .82 * HeightScale + HeightOffsetMultiplier;
		}
	}
}

void AMoriManager::GenerateTrees()
{
	UE_LOG(LogTemp, Warning, TEXT("Generating Trees..."));
	//Generate old growth and cull neighbors
	for (int i = 0; i < Resolution; i++)
	{
		for (int j = 0; j < Resolution; j++)
		{
			int32 curIndex = j * ReadRes + i;

			//Change this if adding more trees, but max total viability is the mycelium presence
			TM_MyceliumWeight[curIndex] = FMath::Max3(SSpruceViabilityMap[curIndex], DFirViabilityMap[curIndex], WHemlockViabilityMap[curIndex]);

			//If the tree was marked as culled or new growth earlier (1 or 2), then skip
			if (TM_GrowthStageCullMap[curIndex] > 0)
			{
				continue;
			}
			
			float winnerV = -1;
			int8 winner = -1;

			//Identify species by randomly existing via viability and highest viability 
			if (UKismetMathLibrary::RandomBoolWithWeight(SSpruceViabilityMap[curIndex] * SSpruceDensityModifier))
			{
				winnerV = SSpruceViabilityMap[curIndex];
				winner = 1;
			}
			if (UKismetMathLibrary::RandomBoolWithWeight(DFirViabilityMap[curIndex] * DFirDensityModifier) && DFirViabilityMap[curIndex] >= winnerV)
			{
				winnerV = DFirViabilityMap[curIndex];
				winner = 2; 
			}
			if (UKismetMathLibrary::RandomBoolWithWeight(WHemlockViabilityMap[curIndex] * WHemlockDensityModifier) && WHemlockViabilityMap[curIndex] >= winnerV)
			{
				winnerV = WHemlockViabilityMap[curIndex];
				winner = 3;
			}

			//If nothing continue the loop
			if (winner == -1)
			{
				continue;
			}

			//If viability is over the cap, cap it
			if (winnerV >= MaxTreeSize)
				winnerV = MaxTreeSize;
			
			//Set viability of the tree and the age as a function of viability
			TM_Viability[curIndex] = winnerV;
			TM_Age[curIndex] = FMath::Square(winnerV);


			//Next figure out if the tree is alive or dead, standing or fallen
			float deathChance = 0;
			float deathStanding = true;
			
			//If large tree
			if (winnerV >= LargeTreesDefinition)
			{
				//Check chance of death and if fallen
				deathChance = winnerV * DeadChanceLarge;
				deathStanding = UKismetMathLibrary::RandomBoolWithWeight(DeadStandingChanceLarge);
			}
			else
			{
				//Otherwise use different parameters for death and fallen chance
				deathChance = DeadChanceSmall;
				deathStanding = UKismetMathLibrary::RandomBoolWithWeight(DeadStandingChanceSmall);
			}

			//Assign Type differently according to type of dead tree it is
			bool dead = UKismetMathLibrary::RandomBoolWithWeight(deathChance);
			if(dead)
			{
				TM_Type[curIndex] = winner + 4;
				if (!deathStanding)
					TM_Type[curIndex] += 4;
			}
			else
			{
				TM_Type[curIndex] = winner;
			}
			
			//Set radius to check for neighbors by species
			int32 nRadius = 0;
			if (winner == 1)
				nRadius = SSpruceCullRadius;
			else if (winner == 2)
				nRadius = DFirCullRadius;
			else if (winner == 3)
				nRadius = WHemlockCullRadius;

			nRadius *= TM_Age[curIndex];
			//Check every neighbor (in circle radius) for competition
			for (int x = i-nRadius; x < i+nRadius; x++)
			{
				for (int y = j-nRadius; y < j+nRadius; y++)
				{
					int32 nIndex = y * Resolution + x;
					//Skip if OOB
					if (nIndex < 0 || nIndex > (Resolution * Resolution))
						continue;
	
					//Skip if center
					if (nIndex == curIndex)
						continue;

					float sqrRadius = ((x-i)*(x-i)) + ((y-j)*(y-j));
					
					//Skip if out of real radius
					if (sqrRadius > nRadius * nRadius)
						continue;
					
					//If this is alive, or if dead and too far away, cull
					if (!dead)// || (dead && sqrRadius > nRadius/2 * nRadius/2))
						TM_GrowthStageCullMap[nIndex] = 1;
					//Otherwise could be successive
					else
						TM_GrowthStageCullMap[nIndex] = 2;

					//If it's alive and more than two meters away, mark for new growth
					if (!dead && sqrRadius > 2 * 2)
						TM_GrowthStageCullMap[nIndex] = 3;
				}
			}//end check neighbors
		}
	}//end old growth loop

	//Loop again to find successive growth
	for (int i = 0; i < Resolution; i++)
	{
		for (int j = 0; j < Resolution; j++)
		{
			int32 curIndex = j * ReadRes + i;

			//Skip if not succession
			if (TM_GrowthStageCullMap[curIndex] != 2 && TM_GrowthStageCullMap[curIndex] != 3)
				continue;

			int32 winner = -1;
			float winnerV = -1;
			
			//Try a Spruce successor
			if (UKismetMathLibrary::RandomBoolWithWeight(SSpruceViabilityMap[curIndex] * SSpruceDensityModifier))
			{
				winnerV = SSpruceViabilityMap[curIndex];
				winner = 1;
			}
			//Try a Hemlock
			if (UKismetMathLibrary::RandomBoolWithWeight(WHemlockViabilityMap[curIndex] * WHemlockDensityModifier) && WHemlockViabilityMap[curIndex] >= winnerV)
			{
				winnerV = WHemlockViabilityMap[curIndex];
				winner = 3;
			}

			//If nothing continue the loop
			if (winner == -1)
			{
				continue;
			}
			
			//Set viability of the tree and the age as a function of viability
			TM_Viability[curIndex] = winnerV;
			TM_Age[curIndex] = FMath::Square(winnerV);
			TM_Type[curIndex] = winner;

			if (TM_GrowthStageCullMap[curIndex] == 2)
			{
				//Cap successors near snags at .8 as old
				TM_Age[curIndex] *= .8;
			}
			else if (TM_GrowthStageCullMap[curIndex] == 3)
			{
				//Cap successors under tall trees at .5 as old
				TM_Age[curIndex] *= .5;
			}
			
			//Random modification to age
			TM_Age[curIndex] *= FMath::RandRange(0.0f, 1.0f);
			
			
			//Set radius to check for neighbors by species
			int32 nRadius = 0;
			if (TM_Type[curIndex] == 1)
				nRadius = SSpruceCullRadius;
			else if (winner == 3)
				nRadius = WHemlockCullRadius;

			nRadius *= TM_Age[curIndex];
			
			//Check every neighbor (in circle radius) for competition
			for (int x = i-nRadius; x < i+nRadius; x++)
			{
				for (int y = j-nRadius; y < j+nRadius; y++)
				{
					int32 nIndex = y * Resolution + x;
					//Skip if OOB
					if (nIndex < 0 || nIndex > (Resolution * Resolution))
						continue;
	
					//Skip if center
					if (nIndex == curIndex)
						continue;

					//Skip if it's an existing tree
					if (TM_GrowthStageCullMap[nIndex] != 1)
						continue;

					float sqrRadius = ((x-i)*(x-i)) + ((y-j)*(y-j));
					
					//Skip if out of real radius
					if (sqrRadius > nRadius * nRadius)
						continue;

					TM_GrowthStageCullMap[nIndex] = 1;
				}
			}//end check neighbors
		}
	}//end old growth loop

	SSpruceViabilityMap.Empty();
	DFirViabilityMap.Empty();
	WHemlockViabilityMap.Empty();
}

void AMoriManager::InstantiateTrees()
{
	UE_LOG(LogTemp, Warning, TEXT("Instantiating tree meshes..."));

	HISM_Width = Resolution / HISM_Num_Side;

	int32 treeCount = 0;
	
	//Foreach slot
	for (int i = 0; i < Resolution; i++)
	{
		for (int j = 0; j < Resolution; j++)
		{
			int32 index = j * ReadRes + i;
			int8 tType = TM_Type[index];
			float scale = TM_Age[index];

			int32 hism_x = i / HISM_Width;
			int32 hism_y = j / HISM_Width;
			int32 hism_ix = hism_y * HISM_Num_Side + hism_x;

			
			FTransform newT = FTransform(FQuat(0,0,0,1), FVector((i * WidthScale) + VertexOffset,(j * WidthScale) + VertexOffset, VertexHeightMap[index]), FVector(scale, scale,scale));

			//add Jitter
			float dX = FMath::RandRange(-1,1) * JitterScale_m * scale;
			float dY = FMath::RandRange(-1,1) * JitterScale_m * scale;
			newT.SetLocation(newT.GetLocation()+ FVector(dX, dY, 0));

			//Instantiate a spruce if it exists
			if (tType == 1)
			{
				TM_InstanceIx[index] = HISM_SSpruce[hism_ix]->AddInstance(newT, true);
			}
			//Instantiate a fir if it exists
			else if (tType == 2)
			{
				TM_InstanceIx[index] = HISM_DFir[hism_ix]->AddInstance(newT, true);
			}
			//Instantiate a hemlock if it exists
			else if (tType == 3)
			{
				TM_InstanceIx[index] = HISM_WHemlock[hism_ix]->AddInstance(newT, true);
			}
			//Instantiate dead standing spruce
			else if (tType == 5 || tType == 9)
			{
				TM_InstanceIx[index] = HISM_SSpruce_Dead_S[hism_ix]->AddInstance(newT, true);
			}
			//Instantiate dead standing fir
			else if (tType == 6 || tType == 10)
			{
				TM_InstanceIx[index] = HISM_DFir_Dead_S[hism_ix]->AddInstance(newT, true);
			}
			//Instantiate dead standing hemlock
			else if (tType == 7 || tType == 11)
			{
				TM_InstanceIx[index] = HISM_WHemlock_Dead_S[hism_ix]->AddInstance(newT, true);
			}
			else continue;
			treeCount++;
			// //Instantiate dead fallen spruce
			// else if (tType == 9)
			// {
			// 	
			// }
			// //Instantiate dead fallen fir
			// else if (tType == 10)
			// {
			// 	
			// }
			// //Instantiate dead fallen Hemlock
			// else if (tType == 11)
			// {
			// 	
			// }

			
		}
	}
		//Instantiate a mesh if it exists
		//HISM_Trees->AddInstanceWorldSpace(FTransform(FVector((i * 100) + VertexOffset,(j * 100) + VertexOffset, fHeight)));

	UE_LOG(LogTemp, Warning, TEXT("This many trees instantiated: %d"), treeCount);
}


void AMoriManager::GenerateMycelium()
{
	if (DebugMycelium && HISM_MyceliumDebugger != nullptr)
	{
		for (int i = 0; i < Resolution; i++)
		{
			for (int j = 0; j < Resolution; j++)
			{
				const int32 curIndex = j * ReadRes + i;
				HISM_MyceliumDebugger->AddInstance(FTransform(FQuat(0,0,0,1), FVector((i * WidthScale) + VertexOffset,(j * WidthScale) + VertexOffset, VertexHeightMap[curIndex] + 500), FVector(.5, .5,.5)), true);
			}
		}
	}
}

void AMoriManager::GenerateWaterLevels()
{
	//Loop through every point, give respective tile a height if it's not greater than one recorded
	//tile map is filled with like 99999s, some divided factor of Resolution big.

	//Placeholder put down an ocean plane at 0,0 z
	if (WaterPlaceholder != nullptr && bAdjustWater)
		WaterPlaceholder->SetActorLocation(FVector(0,0,WaterLevelsMap[0]));
}

void AMoriManager::WorldToMapCoords(int32 &x, int32 &y, int32 &curIndex)
{
	//Translate coordinates back to index-usable ones
	x = x - VertexOffset;
	x =  x / WidthScale;

	y = y - VertexOffset;
	y = y / WidthScale;
	
	curIndex = y * Resolution + x;
}

void AMoriManager::TickMycelium(float DeltaTime)
{
	M_DepletionIndices.GetKeys(Keys);
	
	if (M_DepletionIndices.Num() > 0)
	{		
		for (int32 i : Keys)
		{
			const float target = *M_DepletionIndices.Find(i);

			if (target > TM_MyceliumWeight[i])
			{
				//Add an amount such that a 0 goes from 0 to 1 in GrowthTime seconds
				TM_MyceliumWeight[i] += DeltaTime * 1/MyceliumGrowthTime;

				//At minimum, remove from list
				if (TM_MyceliumWeight[i] >= target)
				{
					TM_MyceliumWeight[i] = target;
					M_DepletionIndices.Remove(i);
				}
			}
			else
			{
				//Deplete an amount such that a 1 goes from 1 to the minimum in DepletionTime seconds
				TM_MyceliumWeight[i] -= DeltaTime * (1-MyceliumDepletionMin)/MyceliumDepletionTime;

				//At minimum, remove from list
				if (TM_MyceliumWeight[i] <= target)
				{
					TM_MyceliumWeight[i] = target;
					M_DepletionIndices.Remove(i);
				}
			}
		}
	}
}

void AMoriManager::InitializeSystem()
{
	//LandscapeActor.Import()

	auto spruceC =  Cast<UHierarchicalInstancedStaticMeshComponent>(GetComponentsByTag(UHierarchicalInstancedStaticMeshComponent::StaticClass(), "SSpruce").Last());
	if (!spruceC)
		UE_LOG(LogTemp, Error, TEXT("No SSpruce base HISM!"))
	auto firC = Cast<UHierarchicalInstancedStaticMeshComponent>(GetComponentsByTag(UHierarchicalInstancedStaticMeshComponent::StaticClass(), "DFir").Last());
	if (!firC)
		UE_LOG(LogTemp, Error, TEXT("No DFir base HISM!"))
	auto hemC = Cast<UHierarchicalInstancedStaticMeshComponent>(GetComponentsByTag(UHierarchicalInstancedStaticMeshComponent::StaticClass(), "WHemlock").Last());
	if (!hemC)
		UE_LOG(LogTemp, Error, TEXT("No WHemlock base HISM!"))
	auto dead_s_spruceC = Cast<UHierarchicalInstancedStaticMeshComponent>(GetComponentsByTag(UHierarchicalInstancedStaticMeshComponent::StaticClass(), "SSpruce_Dead_S").Last());
	if (!dead_s_spruceC)
		UE_LOG(LogTemp, Error, TEXT("No SSpruce_Dead_S base HISM!"))
	auto dead_s_firC = Cast<UHierarchicalInstancedStaticMeshComponent>(GetComponentsByTag(UHierarchicalInstancedStaticMeshComponent::StaticClass(), "DFir_Dead_S").Last());
	if (!dead_s_firC)
		UE_LOG(LogTemp, Error, TEXT("No DFir_Dead_S base HISM!"))
	auto dead_s_hemC = Cast<UHierarchicalInstancedStaticMeshComponent>(GetComponentsByTag(UHierarchicalInstancedStaticMeshComponent::StaticClass(), "WHemlock_Dead_S").Last());
	if (!dead_s_hemC)
		UE_LOG(LogTemp, Error, TEXT("No WHemlock_Dead_S base HISM!"))
	
	HISM_SSpruce.Add(spruceC);
	HISM_DFir.Add(firC);
	HISM_WHemlock.Add(hemC);
	HISM_SSpruce_Dead_S.Add(dead_s_spruceC);
	HISM_DFir_Dead_S.Add(dead_s_firC);
	HISM_WHemlock_Dead_S.Add(dead_s_hemC);
	
	for (int32 i = 0; i < HISM_Num_Side; i++)
	{
		for (int32 j = 0; j < HISM_Num_Side; j++)
		{
			if (i == 0 && j == 0)
				continue;

			auto a = Cast<UHierarchicalInstancedStaticMeshComponent>(StaticDuplicateObject(spruceC, this, FName(FString("HISM_SSpruce_" + FString::FromInt(i) + FString::FromInt(j)))));
			auto b = Cast<UHierarchicalInstancedStaticMeshComponent>(StaticDuplicateObject(firC, this, FName(FString("HISM_DFir_" + FString::FromInt(i) + FString::FromInt(j)))));
			auto c = Cast<UHierarchicalInstancedStaticMeshComponent>(StaticDuplicateObject(hemC, this, FName(FString("HISM_WHemlock_" + FString::FromInt(i) + FString::FromInt(j)))));
			auto d = Cast<UHierarchicalInstancedStaticMeshComponent>(StaticDuplicateObject(dead_s_spruceC, this, FName(FString("HISM_SSpruce_Dead_S_" + FString::FromInt(i) + FString::FromInt(j)))));
			auto e = Cast<UHierarchicalInstancedStaticMeshComponent>(StaticDuplicateObject(dead_s_firC, this, FName(FString("HISM_DFir_Dead_S_" + FString::FromInt(i) + FString::FromInt(j)))));
			auto f = Cast<UHierarchicalInstancedStaticMeshComponent>(StaticDuplicateObject(dead_s_hemC, this, FName(FString("HISM_WHemlock_Dead_S_" + FString::FromInt(i) + FString::FromInt(j)))));
			
			a->RegisterComponent();
			b->RegisterComponent();
			c->RegisterComponent();
			d->RegisterComponent();
			e->RegisterComponent();
			f->RegisterComponent();
			
			HISM_SSpruce.Add(a);
			HISM_DFir.Add(b);
			HISM_WHemlock.Add(c);
			HISM_SSpruce_Dead_S.Add(d);
			HISM_DFir_Dead_S.Add(e);
			HISM_WHemlock_Dead_S.Add(f);
		}
	}
	
	
	double secs = FPlatformTime::Seconds();
	if (bGenerate)
	{
		//OptionsSetupHISM();
	
		LoadData();
		GenerateTrees();
		InstantiateTrees();
		GenerateMycelium();
		GenerateWaterLevels();
	}
	double elapsed = (FPlatformTime::Seconds() - secs) * 1000;
	UE_LOG(LogTemp, Warning, TEXT("Tree generation takes %f milliseconds"), elapsed);
	
	// UE_LOG(LogTemp, Warning, TEXT("%d SSpruce"), HISM_SSpruce->GetInstanceCount());
	// UE_LOG(LogTemp, Warning, TEXT("%d HISM_DFir"), HISM_DFir->GetInstanceCount());
	// UE_LOG(LogTemp, Warning, TEXT("%d HISM_WHemlock"), HISM_WHemlock->GetInstanceCount());
	// UE_LOG(LogTemp, Warning, TEXT("%d HISM_SSpruce_Dead_S"), HISM_SSpruce_Dead_S->GetInstanceCount());
	// UE_LOG(LogTemp, Warning, TEXT("%d HISM_DFir_Dead_S"), HISM_DFir_Dead_S->GetInstanceCount());
	// UE_LOG(LogTemp, Warning, TEXT("%d HISM_WHemlock_Dead_S"), HISM_WHemlock_Dead_S->GetInstanceCount());
	// UE_LOG(LogTemp, Warning, TEXT("%d Total Trees:"), HISM_SSpruce->GetInstanceCount() + HISM_DFir->GetInstanceCount() +
	// 	HISM_WHemlock->GetInstanceCount() + HISM_SSpruce_Dead_S->GetInstanceCount() + HISM_DFir_Dead_S->GetInstanceCount() + HISM_WHemlock_Dead_S->GetInstanceCount());
}

// Called every frame
void AMoriManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bMarkFrameDirty)
	{
		bMarkFrameDirty = false;
		for (auto hism : dirtyHISMs)
		{
			hism->MarkRenderStateDirty();
		}
	}
	//TickMycelium(DeltaTime);
}

