#pragma once

#include "CoreMinimal.h"

struct FHexCellConfigData;
enum class EHexTerrainType : uint8;
enum class EHexDirection : uint8;

struct FClimateData
{
	float CloudAmount;
	float Humidity;

	FClimateData();
	void ClearData();
};

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
	FIntPoint GetRandomGridInMap(int32 RegionId);

	void ErodeTerrain();
	bool CheckErodible(const FIntPoint& CurGrid, const TArray<FIntVector>& Neighbors);
	bool CheckErodible(const FIntPoint& CurGrid, const FIntPoint& Neighbor);
	
	void EvolveClimateData();
	void PaintTerrainType(EHexTerrainType FirstLayer);
	EHexTerrainType GetTerrainTypeByElevation(int32 InElevation);
	EHexTerrainType GetTerrainTypeByParameters(float Humidity, int32 InElevation);
	void GetGridNeighbors(const FIntPoint& CurGrid, int32 MaxDist, TArray<FIntVector>& OutNeighbors);
	static EHexDirection GetHexDirection(const FString& InTypeStr);

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
	TMap<EHexTerrainType, float> HumidityToTerrainType;
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

	// climate
	TArray<TArray<FClimateData>> TerrainClimate;
	TArray<TArray<FClimateData>> LastTerrainClimate;
	int32 EvolutionSteps;
	float EvaporationRatio;
	float PrecipitationRatio;
	float RunOffRatio;
	float SeepageRatio;
	EHexDirection WindDirection;
	float WindStrength;

	// inner
	TSet<FIntPoint> SelectedGrids;
	TMap<FIntPoint, int32> NeighborGrids;
};