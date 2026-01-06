#pragma once

#include "CoreMinimal.h"

struct FHexCellConfigData;
enum class EHexTerrainType : uint8;

struct FHexTerrainDataGenerator 
{
	FHexTerrainDataGenerator(const FIntPoint& GridSize, const FIntPoint& ChunkSize, FHexCellConfigData& OutData);

	void GenerateData();

private:
	
	void LoadConfigFromFile();

	int32 RaiseSinkTerrain(int32 NumOfGrids, bool bSink = false);
	bool SelectGridFromNeighborsRandomly(FIntPoint& OutGrid);
	void GetGridNeighbors(const FIntPoint& CurGrid, int32 MaxDist, TArray<FIntVector>& OutNeighbors);
	EHexTerrainType GetTerrainTypeByElevation(int32 InElevation);

	void GenerateCoastline(EHexTerrainType FirstLayer, EHexTerrainType SecondLayer);

private:

	// inputs
	FIntPoint GridSize;
	FIntPoint ChunkSize;
	FHexCellConfigData& OutConfigData;

	// common
	FIntPoint ElevationRange;
	int32 StartElevation;
	int32 StartWaterLevel;
	TMap<EHexTerrainType, int32> ElevationToTerrainType;

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