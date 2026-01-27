#pragma once

#include "CoreMinimal.h"
#include "HexTerrainCommon.h"

struct FHexCellConfigData;
enum class EHexTerrainType : uint8;
enum class EHexDirection : uint8;

struct FClimateData
{
	float CloudAmount;
	float Humidity;
	float Temperature;

	FClimateData();
	void ClearData();
};

struct FRiverFlagData
{
	bool bHasInRiver;
	bool bHasOutRiver;

	FRiverFlagData()
		: bHasInRiver(false), bHasOutRiver(false)
	{}
};

enum class EHemisphereMode : uint8
{
	North, South, Both
};

struct FBiomeData
{
	EHexTerrainType TerrainType;
	int32 PlantLevel;

	FBiomeData()
		: TerrainType(EHexTerrainType(0u)), PlantLevel(0)
	{}

	FBiomeData(EHexTerrainType InType, int32 InPlantLevel)
		: TerrainType(InType), PlantLevel(InPlantLevel)
	{}
};

struct FHexTerrainDataGenerator 
{
	FHexTerrainDataGenerator(int32 MaxTerranceElevation, const FHexTerrainNoiser& InNoiser, FHexCellConfigData& OutData);

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
	void CalcTemperature();

	void CreateRivers(int32 LandCells);
	int32 CreateOneRiver(const FIntPoint& StartGrid);

	void PaintTerrainType();
	void GetTerrainBiomeByParameters(float Humidity, float Termperature, int32 Elevation, FBiomeData& OutBiome);
	void GetGridNeighbors(const FIntPoint& CurGrid, int32 MaxDist, TArray<FIntVector>& OutNeighbors);
	static EHexDirection GetHexDirection(const FString& InTypeStr);

private:

	// inputs
	int32 MaxTerranceElevation;
	const FHexTerrainNoiser& Noiser;
	FHexCellConfigData& OutConfigData;

	// common
	FIntPoint MapSize;
	FIntPoint ChunkSize;
	int32 RandomSeed;
	FIntPoint MapBorder;
	FIntPoint ElevationRange;
	FIntPoint StartElevation;
	int32 StartWaterLevel;
	float BeachRatio;

	// regions
	TArray<FInt32Rect> MapRegions;
	FIntPoint RegionCount;
	int32 RegionBorder;

	// land
	float LandRandomness;
	int32 LandPercentage;
	FIntPoint NumOfGridsPerLand;
	float HighRiseProbability;
	float SinkProbability;
	TSet<FIntPoint> SelectedGrids;
	TMap<FIntPoint, int32> NeighborGrids;

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

	// temperature
	FVector2f TemperatureRange;
	EHemisphereMode HemisphereMode;
	float TemperatureJitter;

	static int32 NumOfHumidityLevels;
	static int32 NumOfTemperatureLevels;
	TArray<float> HumidityThresholds;
	TArray<float> TemperatureThresholds;
	TArray<TArray<FBiomeData>> BiomeMatrix;

	// rivers
	int32 RiverPercentage;
	int32 MinRiverLength;
	int32 RiverRetryTimes;
	TArray<TArray<FRiverFlagData>> TerrainRiverFlag;
};