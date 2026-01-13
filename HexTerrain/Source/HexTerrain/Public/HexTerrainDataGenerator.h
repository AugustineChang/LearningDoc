#pragma once

#include "CoreMinimal.h"

struct FHexCellConfigData;
enum class EHexTerrainType : uint8;

struct FHexTerrainDataGenerator 
{
	FHexTerrainDataGenerator(int32 MaxTerranceElevation, FHexCellConfigData& OutData);

	void GenerateData();
	FIntPoint GetMapSize() const { return MapSize; }

private:
	
	void LoadConfigFromFile();
	void CreateRegions();

	int32 RaiseSinkTerrain(int32 NumOfGrids, int32 RegionId, bool bSink = false);
	bool SelectGridFromNeighborsRandomly(FIntPoint& OutGrid);
	EHexTerrainType GetTerrainTypeByElevation(int32 InElevation);
	FIntPoint GetRandomGridInMap(int32 RegionId);

	void ErodeTerrain();
	bool CheckErodible(const FIntPoint& CurGrid, const TArray<FIntVector>& Neighbors);
	bool CheckErodible(const FIntPoint& CurGrid, const FIntPoint& Neighbor);
	
	void PaintTerrainType(EHexTerrainType FirstLayer, EHexTerrainType SecondLayer);
	void GetGridNeighbors(const FIntPoint& CurGrid, int32 MaxDist, TArray<FIntVector>& OutNeighbors);

private:

	// inputs
	int32 MaxTerranceElevation;
	FHexCellConfigData& OutConfigData;

	// common
	FIntPoint MapSize;
	FIntPoint ChunkSize;
	int32 RandomSeed;
	FIntPoint MapBorder;
	FIntPoint ElevationRange;
	FIntPoint StartElevation;
	int32 StartWaterLevel;
	TMap<EHexTerrainType, int32> ElevationToTerrainType;
	TArray<FInt32Rect> MapRegions;
	FIntPoint RegionCount;
	int32 RegionBorder;

	// land
	float LandRandomness;
	int32 LandPercentage;
	FIntPoint NumOfGridsPerLand;
	float HighRiseProbability;
	float SinkProbability;

	// erosion
	int32 ErosionPercentage;

	// inner
	TSet<FIntPoint> SelectedGrids;
	TMap<FIntPoint, int32> NeighborGrids;
};