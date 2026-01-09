#pragma once

#include "CoreMinimal.h"

struct FHexCellConfigData;
enum class EHexTerrainType : uint8;

struct FHexTerrainDataGenerator 
{
	FHexTerrainDataGenerator(FHexCellConfigData& OutData);

	void GenerateData();
	FIntPoint GetMapSize() const { return MapSize; }

private:
	
	void LoadConfigFromFile();

	void CreateRegions();
	int32 RaiseSinkTerrain(int32 NumOfGrids, int32 RegionId, bool bSink = false);
	bool SelectGridFromNeighborsRandomly(FIntPoint& OutGrid);
	void GetGridNeighbors(const FIntPoint& CurGrid, int32 MaxDist, TArray<FIntVector>& OutNeighbors);
	EHexTerrainType GetTerrainTypeByElevation(int32 InElevation);
	FIntPoint GetRandomGridInMap(int32 RegionId);

	void GenerateCoastline(EHexTerrainType FirstLayer, EHexTerrainType SecondLayer);

private:

	// inputs
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
	int32 RegionBorder;

	// land
	float LandRandomness;
	int32 LandPercentage;
	FIntPoint NumOfGridsPerLand;
	float HighRiseProbability;
	float SinkProbability;

	// inner
	TSet<FIntPoint> SelectedGrids;
	TMap<FIntPoint, int32> NeighborGrids;
};