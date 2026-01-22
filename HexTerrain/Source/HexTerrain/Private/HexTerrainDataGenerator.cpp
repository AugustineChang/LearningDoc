#include "HexTerrainDataGenerator.h"
#include "HexTerrainGenerator.h"
#include "HexTerrainNavigator.h"
#include "Misc/ConfigCacheIni.h"

static TArray<FIntPoint> EvenAroundGrids = {
		FIntPoint{1, 0}, FIntPoint{0, 1}, FIntPoint{-1, 1}, FIntPoint{-1, 0}, FIntPoint{-1, -1}, FIntPoint{0, -1}
};
static TArray<FIntPoint> OddAroundGrids = {
	FIntPoint{1, 0}, FIntPoint{1, 1}, FIntPoint{0, 1}, FIntPoint{-1, 0}, FIntPoint{0, -1}, FIntPoint{1, -1}
};


FClimateData::FClimateData()
{
	ClearData();
}

void FClimateData::ClearData()
{
	CloudAmount = 0.0f;
	Humidity = 0.0f;
}

#pragma optimize("", off)

FHexTerrainDataGenerator::FHexTerrainDataGenerator(int32 MaxTerranceElevation, FHexCellConfigData& OutData)
	: MaxTerranceElevation(MaxTerranceElevation), OutConfigData(OutData),
	MapSize(20, 20), ChunkSize(5, 5), RandomSeed(FMath::Rand()), MapBorder(0, 0), ElevationRange(-4,10), 
	StartElevation(-2,-1), StartWaterLevel(0), BeachRatio(0.5f),
	RegionCount(1,1), RegionBorder(0), 
	LandRandomness(0.5f), LandPercentage(50), NumOfGridsPerLand(50, 200), HighRiseProbability(0.0f), SinkProbability(0.0f),
	ErosionPercentage(50),
	EvolutionSteps(50), EvaporationRatio(0.5f), PrecipitationRatio(0.25f), RunOffRatio(0.25f), SeepageRatio(0.125f), WindDirection(EHexDirection::E), WindStrength(1.0f),
	RiverPercentage(10), MinRiverLength(5), RiverRetryTimes(3)
{
	ElevationToTerrainType.Add(EHexTerrainType::Sand, 0);
	ElevationToTerrainType.Add(EHexTerrainType::Ice, ElevationRange.Y - 2);
	HumidityToTerrainType.Add(EHexTerrainType::Grass, 1.0f);
	HumidityToTerrainType.Add(EHexTerrainType::Mud, 0.8f);
	HumidityToTerrainType.Add(EHexTerrainType::Plateau, 0.6f);
	HumidityToTerrainType.Add(EHexTerrainType::SmallStone, 0.4f);
	HumidityToTerrainType.Add(EHexTerrainType::Stone, 0.2f);

	LoadConfigFromFile();
}

void FHexTerrainDataGenerator::GenerateData()
{
	FMath::RandInit(RandomSeed);
	FMath::SRandInit(RandomSeed);
	CreateRegions();

	OutConfigData.bConfigValid = true;
	OutConfigData.HexChunkSize = ChunkSize;
	OutConfigData.HexChunkCount = MapSize / ChunkSize;

	EHexTerrainType FirstLayerType = ElevationToTerrainType.Get(FSetElementId::FromInteger(0)).Key;

	// Init
	OutConfigData.ElevationsList.Empty(MapSize.Y);
	OutConfigData.ElevationsList.AddDefaulted(MapSize.Y);
	OutConfigData.WaterLevelsList.Empty(MapSize.Y);
	OutConfigData.WaterLevelsList.AddDefaulted(MapSize.Y);
	OutConfigData.TerrainTypesList.Empty(MapSize.Y);
	OutConfigData.TerrainTypesList.AddDefaulted(MapSize.Y);
	OutConfigData.FeatureValuesList.Empty(MapSize.Y);
	OutConfigData.FeatureValuesList.AddDefaulted(MapSize.Y);
	OutConfigData.CustomDataList.Empty(MapSize.Y);
	OutConfigData.CustomDataList.AddDefaulted(MapSize.Y);
	for (int32 Y = 0; Y < MapSize.Y; ++Y)
	{
		OutConfigData.ElevationsList[Y].AddZeroed(MapSize.X);
		OutConfigData.WaterLevelsList[Y].Init(StartWaterLevel, MapSize.X);
		OutConfigData.TerrainTypesList[Y].Init(FirstLayerType, MapSize.X);
		OutConfigData.FeatureValuesList[Y].AddZeroed(MapSize.X);
		OutConfigData.CustomDataList[Y].AddZeroed(MapSize.X);

		for (int32 X = 0; X < MapSize.X; ++X)
		{
			OutConfigData.ElevationsList[Y][X] = FMath::RandRange(StartElevation.X, StartElevation.Y);
		}
	}
	OutConfigData.RiversList.Empty();
	OutConfigData.RoadsList.Empty();

	// Raise Lands
	int32 LandCells = FMath::RoundToInt(MapSize.X * MapSize.Y * LandPercentage * 0.01f);
	int32 LandBudget = LandCells;
	int32 StopCounter = 0;
	int32 NumOfRegions = MapRegions.Num();
	while(LandBudget > 0 && StopCounter < 10000)
	{
		int32 RIndex = StopCounter % NumOfRegions;
		bool bSinkTerrain = FMath::FRand() < SinkProbability;
		int32 DesiredLandGrids = FMath::Min(LandBudget, FMath::RandRange(NumOfGridsPerLand.X, NumOfGridsPerLand.Y));
		int32 ChangedLandGrids = RaiseSinkTerrain(DesiredLandGrids, RIndex, bSinkTerrain);

		if (bSinkTerrain)
			LandBudget += ChangedLandGrids;
		else
			LandBudget -= ChangedLandGrids;

		++StopCounter;
	}
	if (LandBudget > 0) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to use up all land budget, remaining(%d/%d)."), LandBudget, LandCells);
		LandCells -= LandBudget;
	}

	// Erosion
	ErodeTerrain();
	
	// Climate
	TerrainClimate.Empty(MapSize.Y);
	TerrainClimate.AddDefaulted(MapSize.Y);
	LastTerrainClimate.Empty(MapSize.Y);
	LastTerrainClimate.AddDefaulted(MapSize.Y);
	for (int32 Y = 0; Y < MapSize.Y; ++Y)
	{
		TerrainClimate[Y].AddDefaulted(MapSize.X);
		LastTerrainClimate[Y].AddDefaulted(MapSize.X);
	}
	for (int32 Index = 0; Index < EvolutionSteps; ++Index)
		EvolveClimateData();

	// Create Rivers
	TerrainRiverFlag.Empty(MapSize.Y);
	TerrainRiverFlag.AddDefaulted(MapSize.Y);
	for (int32 Y = 0; Y < MapSize.Y; ++Y)
	{
		TerrainRiverFlag[Y].AddDefaulted(MapSize.X);
	}
	CreateRivers(LandCells);

	// TerrainType && Coastline
	PaintTerrainType(FirstLayerType);
}

void FHexTerrainDataGenerator::LoadConfigFromFile()
{
	FString ConfigFilePath = FPaths::ProjectConfigDir() + TEXT("GeneratingConfig.ini");
	FConfigFile ConfigFile;
	if (FPaths::FileExists(ConfigFilePath))
	{
		ConfigFile.Read(ConfigFilePath);

		static FString ConfigCommonSectionName{ TEXT("Common") };
		static FString ConfigGridsSizeXName{ TEXT("GridsSizeX") };
		static FString ConfigGridsSizeYName{ TEXT("GridsSizeY") };
		static FString ConfigChunkSizeXName{ TEXT("ChunkSizeX") };
		static FString ConfigChunkSizeYName{ TEXT("ChunkSizeY") };
		static FString ConfigRandomSeedName{ TEXT("RandomSeed") };
		static FString ConfigMapBorderXName{ TEXT("MapBorderX") };
		static FString ConfigMapBorderYName{ TEXT("MapBorderY") };
		static FString ConfigElevationMinName{ TEXT("ElevationMin") };
		static FString ConfigElevationMaxName{ TEXT("ElevationMax") };
		static FString ConfigDefaultElevationMinName{ TEXT("DefaultElevationMin") };
		static FString ConfigDefaultElevationMaxName{ TEXT("DefaultElevationMax") };
		static FString ConfigDefaultWaterLevelName{ TEXT("DefaultWaterLevel") };
		static FString ConfigBeachRatioName{ TEXT("BeachRatio") };
		static FString ConfigSandElevationName{ TEXT("SandElevation") };
		static FString ConfigIceElevationName{ TEXT("IceElevation") };
		static FString ConfigGrassHumidityName{ TEXT("GrassHumidity") };
		static FString ConfigMudHumidityName{ TEXT("MudHumidity") };
		static FString ConfigPlateauHumidityName{ TEXT("PlateauHumidity") };
		static FString ConfigSmallStoneHumidityName{ TEXT("SmallStoneHumidity") };
		static FString ConfigStoneHumidityName{ TEXT("StoneHumidity") };
		
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigGridsSizeXName, MapSize.X);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigGridsSizeYName, MapSize.Y);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigChunkSizeXName, ChunkSize.X);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigChunkSizeYName, ChunkSize.Y);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigRandomSeedName, RandomSeed);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigMapBorderXName, MapBorder.X);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigMapBorderYName, MapBorder.Y);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigElevationMinName, ElevationRange.X);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigElevationMaxName, ElevationRange.Y);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigDefaultElevationMinName, StartElevation.X);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigDefaultElevationMaxName, StartElevation.Y);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigDefaultWaterLevelName, StartWaterLevel);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigDefaultWaterLevelName, StartWaterLevel);
		ConfigFile.GetFloat(*ConfigCommonSectionName, *ConfigBeachRatioName, BeachRatio);
		StartElevation.X = FMath::Clamp(StartElevation.X, ElevationRange.X, ElevationRange.Y);
		StartElevation.Y = FMath::Clamp(StartElevation.Y, ElevationRange.X, ElevationRange.Y);

		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigSandElevationName, ElevationToTerrainType[EHexTerrainType::Sand]);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigIceElevationName, ElevationToTerrainType[EHexTerrainType::Ice]);
		ConfigFile.GetFloat(*ConfigCommonSectionName, *ConfigGrassHumidityName, HumidityToTerrainType[EHexTerrainType::Grass]);
		ConfigFile.GetFloat(*ConfigCommonSectionName, *ConfigMudHumidityName, HumidityToTerrainType[EHexTerrainType::Mud]);
		ConfigFile.GetFloat(*ConfigCommonSectionName, *ConfigPlateauHumidityName, HumidityToTerrainType[EHexTerrainType::Plateau]);
		ConfigFile.GetFloat(*ConfigCommonSectionName, *ConfigSmallStoneHumidityName, HumidityToTerrainType[EHexTerrainType::SmallStone]);
		ConfigFile.GetFloat(*ConfigCommonSectionName, *ConfigStoneHumidityName, HumidityToTerrainType[EHexTerrainType::Stone]);

		static FString ConfigRegionSectionName{ TEXT("Region") };
		static FString ConfigRegionCountXName{ TEXT("RegionCountX") };
		static FString ConfigRegionCountYName{ TEXT("RegionCountY") };
		static FString ConfigRegionBorderName{ TEXT("RegionBorder") };

		ConfigFile.GetInt(*ConfigRegionSectionName, *ConfigRegionBorderName, RegionBorder);
		ConfigFile.GetInt(*ConfigRegionSectionName, *ConfigRegionCountXName, RegionCount.X);
		ConfigFile.GetInt(*ConfigRegionSectionName, *ConfigRegionCountYName, RegionCount.Y);
		RegionCount.X = FMath::Clamp(RegionCount.X, 0, 10);
		RegionCount.Y = FMath::Clamp(RegionCount.Y, 0, 10);

		static FString ConfigRaiseSinkTerrainSectionName{ TEXT("RaiseSinkTerrain") };
		static FString ConfigLandPercentageName{ TEXT("LandPercentage") };
		static FString ConfigLandRandomnessName{ TEXT("LandRandomness") };
		static FString ConfigLandGridsRangeMinName{ TEXT("LandGridsRangeMin") };
		static FString ConfigLandGridsRangeMaxName{ TEXT("LandGridsRangeMax") };
		static FString ConfigHighRiseProbabilityName{ TEXT("HighRiseProbability") };
		static FString ConfigSinkProbabilityName{ TEXT("SinkProbability") };

		ConfigFile.GetInt(*ConfigRaiseSinkTerrainSectionName, *ConfigLandPercentageName, LandPercentage);
		ConfigFile.GetFloat(*ConfigRaiseSinkTerrainSectionName, *ConfigLandRandomnessName, LandRandomness);
		ConfigFile.GetInt(*ConfigRaiseSinkTerrainSectionName, *ConfigLandGridsRangeMinName, NumOfGridsPerLand.X);
		ConfigFile.GetInt(*ConfigRaiseSinkTerrainSectionName, *ConfigLandGridsRangeMaxName, NumOfGridsPerLand.Y);
		ConfigFile.GetFloat(*ConfigRaiseSinkTerrainSectionName, *ConfigHighRiseProbabilityName, HighRiseProbability);
		ConfigFile.GetFloat(*ConfigRaiseSinkTerrainSectionName, *ConfigSinkProbabilityName, SinkProbability);
		LandPercentage = FMath::Clamp(LandPercentage, 0, 100);
		LandRandomness = FMath::Clamp(LandRandomness, 0.0f, 1.0f);
		NumOfGridsPerLand.X = FMath::Clamp(NumOfGridsPerLand.X, 0, 1000);
		NumOfGridsPerLand.Y = FMath::Clamp(NumOfGridsPerLand.Y, 0, 1000);
		HighRiseProbability = FMath::Clamp(HighRiseProbability, 0.0f, 1.0f);
		SinkProbability = FMath::Clamp(SinkProbability, 0.0f, 1.0f);

		static FString ConfigErosionSectionName{ TEXT("Erosion") };
		static FString ConfigErosionPercentageName{ TEXT("ErosionPercentage") };

		ConfigFile.GetInt(*ConfigErosionSectionName, *ConfigErosionPercentageName, ErosionPercentage);
		ErosionPercentage = FMath::Clamp(ErosionPercentage, 0, 100);

		static FString ConfigClimateSectionName{ TEXT("Climate") };
		static FString ConfigEvolutionStepsName{ TEXT("EvolutionSteps") };
		static FString ConfigEvaporationRatioName{ TEXT("EvaporationRatio") };
		static FString ConfigPrecipitationRatioName{ TEXT("PrecipitationRatio") };
		static FString ConfigRunOffRatioName{ TEXT("RunOffRatio") };
		static FString ConfigSeepageRatioName{ TEXT("SeepageRatio") };
		static FString ConfigWindDirectionName{ TEXT("WindDirection") };
		static FString ConfigWindStrengthName{ TEXT("WindStrength") };

		ConfigFile.GetInt(*ConfigClimateSectionName, *ConfigEvolutionStepsName, EvolutionSteps);
		ConfigFile.GetFloat(*ConfigClimateSectionName, *ConfigEvaporationRatioName, EvaporationRatio);
		ConfigFile.GetFloat(*ConfigClimateSectionName, *ConfigPrecipitationRatioName, PrecipitationRatio);
		ConfigFile.GetFloat(*ConfigClimateSectionName, *ConfigRunOffRatioName, RunOffRatio);
		ConfigFile.GetFloat(*ConfigClimateSectionName, *ConfigSeepageRatioName, SeepageRatio);
		FString WindDirectionStr;
		if (ConfigFile.GetString(*ConfigClimateSectionName, *ConfigWindDirectionName, WindDirectionStr))
		{
			WindDirection = GetHexDirection(WindDirectionStr);
		}
		ConfigFile.GetFloat(*ConfigClimateSectionName, *ConfigWindStrengthName, WindStrength);
		EvolutionSteps = FMath::Clamp(EvolutionSteps, 0, 100);
		EvaporationRatio = FMath::Clamp(EvaporationRatio, 0.0f, 1.0f);
		PrecipitationRatio = FMath::Clamp(PrecipitationRatio, 0.0f, 1.0f);
		RunOffRatio = FMath::Clamp(RunOffRatio, 0.0f, 1.0f);
		SeepageRatio = FMath::Clamp(SeepageRatio, 0.0f, 1.0f);
		WindStrength = FMath::Clamp(WindStrength, 0.0f, 10.0f);

		static FString ConfigRiversSectionName{ TEXT("Rivers") };
		static FString ConfigRiverPercentageName{ TEXT("RiverPercentage") };
		static FString ConfigMinRiverLengthName{ TEXT("MinRiverLength") };
		static FString ConfigRiverRetryTimesName{ TEXT("RiverRetryTimes") };

		ConfigFile.GetInt(*ConfigRiversSectionName, *ConfigRiverPercentageName, RiverPercentage);
		ConfigFile.GetInt(*ConfigRiversSectionName, *ConfigMinRiverLengthName, MinRiverLength);
		ConfigFile.GetInt(*ConfigRiversSectionName, *ConfigRiverRetryTimesName, RiverRetryTimes);
		RiverPercentage = FMath::Clamp(RiverPercentage, 0, 100);
		MinRiverLength = FMath::Clamp(MinRiverLength, 0, 50);
		RiverRetryTimes = FMath::Clamp(RiverRetryTimes, 0, 50);
	}
}

void FHexTerrainDataGenerator::CreateRegions()
{
	auto CalcRegionEdges = [](int32 NumOfRegions, int32 MapSize, int32 MapBorder, int32 RegionBorder, TArray<int32>& OutEdges)
		{
			int32 NumOfEdges = NumOfRegions * 2;
			OutEdges.Empty(NumOfEdges);

			for (int32 Index = 0; Index < NumOfEdges; ++Index)
			{
				int32 BoriderSign = Index % 2 == 0 ? 1 : -1;
				bool bMapEdge = (Index == 0 || Index == (NumOfEdges - 1));
				int32 Border = bMapEdge ? MapBorder : RegionBorder;
				int32 EdgeIndex = Index / 2 + (Index % 2);
				int32 EdgeOffset = bMapEdge ? 0 : ((Index + 1) % 2);
				int32 Edge = (MapSize - 1) * EdgeIndex / NumOfRegions + BoriderSign * Border + EdgeOffset;
				OutEdges.Add(Edge);
			}
		};

	TArray<int32> MapEdgesX;
	CalcRegionEdges(RegionCount.X, MapSize.X, MapBorder.X, RegionBorder, MapEdgesX);

	TArray<int32> MapEdgesY;
	CalcRegionEdges(RegionCount.Y, MapSize.Y, MapBorder.Y, RegionBorder, MapEdgesY);

	MapRegions.Empty(RegionCount.X * RegionCount.Y);
	for (int32 RY = 0; RY < RegionCount.Y; ++RY)
	{
		int32 RegionMin = MapEdgesX[RY * 2];
		int32 RegionMax = MapEdgesX[RY * 2 + 1];
		for (int32 RX = 0; RX < RegionCount.X; ++RX)
		{
			FInt32Rect Region;
			Region.Min.X = MapEdgesX[RX * 2];
			Region.Min.Y = RegionMin;

			Region.Max.X = MapEdgesX[RX * 2 + 1];
			Region.Max.Y = RegionMax;

			MapRegions.Add(Region);
		}
	}
}

int32 FHexTerrainDataGenerator::RaiseSinkTerrain(int32 NumOfGrids, int32 RegionId, bool bSink)
{
	SelectedGrids.Empty();
	NeighborGrids.Empty();
	NeighborGrids.Add(GetRandomGridInMap(RegionId), 0);
	
	int32 RiseAmount = FMath::FRand() < HighRiseProbability ? 2 : 1;
	if (bSink) RiseAmount = -RiseAmount;

	int32 Counter = 0;
	for (int32 Index = 0; Index < NumOfGrids; ++Index)
	{
		FIntPoint SelectedGrid;
		if (!SelectGridFromNeighborsRandomly(SelectedGrid))
			break;
		
		int32& Elevation = OutConfigData.ElevationsList[SelectedGrid.Y][SelectedGrid.X];
		int32 OldElevation = Elevation;
		int32 NewElevation = OldElevation + RiseAmount;
		if (NewElevation < ElevationRange.X || NewElevation > ElevationRange.Y)
			continue;
		Elevation = NewElevation;

		if ((OldElevation < StartWaterLevel) != (Elevation < StartWaterLevel))
			++Counter;
	}

	return Counter;
}

bool FHexTerrainDataGenerator::SelectGridFromNeighborsRandomly(FIntPoint& OutGrid)
{
	if (NeighborGrids.Num() <= 0)
		return false;

	NeighborGrids.ValueSort([](const int32& Val0, const int32& Val1){ return Val0 < Val1; });
	int32 RandIndex = FMath::FRand() < LandRandomness ? FMath::RandHelper(NeighborGrids.Num()) : 0;
	TPair<FIntPoint, int32> SelectedGrid = NeighborGrids.Get(FSetElementId::FromInteger(RandIndex));
	OutGrid = SelectedGrid.Key;

	NeighborGrids.Remove(OutGrid);
	SelectedGrids.Add(OutGrid);

	FIntPoint FirstGrid = SelectedGrids.Get(FSetElementId::FromInteger(0));
	FIntVector FirstCoord = FHexCellData::CalcGridCoordinate(FirstGrid);

	const TArray<FIntPoint>& AroundGrids = OutGrid.Y % 2 == 0 ? EvenAroundGrids : OddAroundGrids;
	int32 NumOfAroundGrids = AroundGrids.Num();
	for (int32 Index = 0; Index < NumOfAroundGrids; ++Index)
	{
		FIntPoint AroundGrid = OutGrid + AroundGrids[Index];
		if (AroundGrid.X < 0 || AroundGrid.X >= MapSize.X ||
			AroundGrid.Y < 0 || AroundGrid.Y >= MapSize.Y)
			continue;

		if (NeighborGrids.Contains(AroundGrid) || SelectedGrids.Contains(AroundGrid))
			continue;

		FIntVector AroundCoord = FHexCellData::CalcGridCoordinate(AroundGrid);
		NeighborGrids.Add(AroundGrid, AHexTerrainNavigator::CalcDirectDistance(FirstCoord, AroundCoord));
	}
	
	return true;
}

FIntPoint FHexTerrainDataGenerator::GetRandomGridInMap(int32 RegionId)
{
	FIntPoint OutGrid;
	const FInt32Rect& Region = MapRegions[RegionId];

	OutGrid.X = FMath::RandRange(Region.Min.X, Region.Max.X);
	OutGrid.Y = FMath::RandRange(Region.Min.Y, Region.Max.Y);

	return OutGrid;
}

void FHexTerrainDataGenerator::ErodeTerrain()
{
	TSet<FIntPoint> ErodibleGrids;
	TArray<FSetElementId> SetIdsList;
	
	for (int32 Y = 0; Y < MapSize.Y; ++Y)
	{
		for (int32 X = 0; X < MapSize.X; ++X)
		{
			FIntPoint CurGrid{ X,Y };

			TArray<FIntVector> Neighbors;
			GetGridNeighbors(CurGrid, 1, Neighbors);

			if (CheckErodible(CurGrid, Neighbors))
			{
				SetIdsList.Add(ErodibleGrids.Add(CurGrid));
			}
		}
	}

	int32 ErosionTarget = FMath::RoundToInt(ErodibleGrids.Num() * (100 - ErosionPercentage) * 0.01f);
	while (ErodibleGrids.Num() > ErosionTarget)
	{
		TSet<FIntPoint> CheckSet;
		FIntPoint SourceGrid{ ForceInitToZero };
		{
			int32 RandIndex = FMath::RandHelper(ErodibleGrids.Num());
			FSetElementId RandId = SetIdsList[RandIndex];
			SourceGrid = ErodibleGrids[RandId];
			CheckSet.Add(SourceGrid);
		}

		// Erode
		FIntPoint TargetGrid{ ForceInitToZero };
		{
			TArray<FIntVector> SourceNeighbors;
			GetGridNeighbors(SourceGrid, 1, SourceNeighbors);

			TArray<FIntPoint> ErosionTargets;
			for (const FIntVector& AroundGrid : SourceNeighbors)
			{
				FIntPoint Neighbor{ AroundGrid.X, AroundGrid.Y };
				if (CheckErodible(SourceGrid, Neighbor))
				{
					ErosionTargets.Add(Neighbor);
				}
				CheckSet.Add(Neighbor);
			}
			TargetGrid = ErosionTargets[FMath::RandHelper(ErosionTargets.Num())];
			OutConfigData.ElevationsList[SourceGrid.Y][SourceGrid.X] -= 1;
			OutConfigData.ElevationsList[TargetGrid.Y][TargetGrid.X] += 1;
		}
		
		// Update ErodibleGrids
		{
			TArray<FIntVector> TargetNeighbors;
			GetGridNeighbors(TargetGrid, 1, TargetNeighbors);

			for (const FIntVector& AroundGrid : TargetNeighbors)
			{
				FIntPoint Neighbor{ AroundGrid.X, AroundGrid.Y };
				CheckSet.Add(Neighbor);
			}
		}

		for (const FIntPoint& CheckGrid : CheckSet)
		{
			TArray<FIntVector> CheckNeighbors;
			GetGridNeighbors(CheckGrid, 1, CheckNeighbors);

			bool IsErodible = CheckErodible(CheckGrid, CheckNeighbors);
			if (IsErodible && !ErodibleGrids.Contains(CheckGrid))
			{
				SetIdsList.Add(ErodibleGrids.Add(CheckGrid));
			}
			else if (!IsErodible)
			{
				FSetElementId SetId = ErodibleGrids.FindId(CheckGrid);
				if (SetId.IsValidId())
				{
					ErodibleGrids.Remove(SetId);
					SetIdsList.RemoveSwap(SetId);
				}
			}
		}
	}
}

bool FHexTerrainDataGenerator::CheckErodible(const FIntPoint& CurGrid, const TArray<FIntVector>& Neighbors)
{	
	const int32& CurElevation = OutConfigData.ElevationsList[CurGrid.Y][CurGrid.X];
	for (const FIntVector& AroundGrid : Neighbors)
	{
		const int32& AroundElevation = OutConfigData.ElevationsList[AroundGrid.Y][AroundGrid.X];
		if (CurElevation - AroundElevation > MaxTerranceElevation)
		{
			return true;
		}
	}

	return false;
}

bool FHexTerrainDataGenerator::CheckErodible(const FIntPoint& CurGrid, const FIntPoint& Neighbor)
{
	const int32& CurElevation = OutConfigData.ElevationsList[CurGrid.Y][CurGrid.X];
	const int32& NeighborElevation = OutConfigData.ElevationsList[Neighbor.Y][Neighbor.X];

	return (CurElevation - NeighborElevation > MaxTerranceElevation);
}

void FHexTerrainDataGenerator::EvolveClimateData()
{
	LastTerrainClimate = TerrainClimate;
	for (int32 Y = 0; Y < MapSize.Y; ++Y)
	{
		for (int32 X = 0; X < MapSize.X; ++X)
			TerrainClimate[Y][X].ClearData();
	}

	for (int32 Y = 0; Y < MapSize.Y; ++Y)
	{
		for (int32 X = 0; X < MapSize.X; ++X)
		{
			const int32& Elevation = OutConfigData.ElevationsList[Y][X];
			bool IsUnderWater = Elevation < StartWaterLevel;

			FClimateData& GridClimate = TerrainClimate[Y][X];
			const FClimateData& LastGridClimate = LastTerrainClimate[Y][X];
			float CurCloudAmount = LastGridClimate.CloudAmount;
			float CurHumidity = LastGridClimate.Humidity;

			// evaporation
			if (IsUnderWater)
			{
				CurHumidity = 1.0f;
				CurCloudAmount += EvaporationRatio;
			}
			else
			{
				float Evaporation = CurHumidity * EvaporationRatio;
				CurHumidity -= Evaporation;
				CurCloudAmount += Evaporation;
			}
			
			// precipitation
			float Precipitation = CurCloudAmount * PrecipitationRatio;
			CurCloudAmount -= Precipitation;
			if (!IsUnderWater)
				CurHumidity += Precipitation;

			// neighbors
			TArray<FIntVector> Neighbors;
			GetGridNeighbors(FIntPoint{ X,Y }, 1, Neighbors);
			TArray<FIntVector> RunOffNeighbors;
			TArray<FIntVector> SeepageNeighbors;

			// disperse & wind
			float Dispersal = CurCloudAmount / (6.0f + WindStrength);
			GridClimate.CloudAmount += Dispersal;
			int32 WindDirectionId = static_cast<uint8>(WindDirection);
			
			for (const FIntVector& AroundGrid : Neighbors)
			{
				const int32& AroundElevation = OutConfigData.ElevationsList[AroundGrid.Y][AroundGrid.X];
				
				FClimateData& AroundClimate = TerrainClimate[AroundGrid.Y][AroundGrid.X];
				AroundClimate.CloudAmount += (WindDirectionId == AroundGrid.Z) ? Dispersal * WindStrength : Dispersal;

				if (!IsUnderWater)
				{
					if (Elevation > AroundElevation && AroundElevation >= StartWaterLevel)
						RunOffNeighbors.Add(AroundGrid);

					if (Elevation == AroundElevation)
						SeepageNeighbors.Add(AroundGrid);
				}
				else
				{
					if (StartWaterLevel >= AroundElevation)
						SeepageNeighbors.Add(AroundGrid);
				}

				/*if (!IsUnderWater && Elevation > FMath::Max(AroundElevation, StartWaterLevel))
					RunOffNeighbors.Add(AroundGrid);

				if (!IsUnderWater)
				{
					if (Elevation == AroundElevation)
						SeepageNeighbors.Add(AroundGrid);
				}
				else
				{
					if (WaterLevel >= AroundElevation)
						SeepageNeighbors.Add(AroundGrid);
				}*/
			}

			float CurHumidity0 = CurHumidity;

			// runoff
			if (RunOffNeighbors.Num() > 0 && RunOffRatio > 0.0f)
			{
				float RunOff = FMath::Min(CurHumidity0 * RunOffRatio, CurHumidity);
				CurHumidity -= RunOff;
				RunOff /= RunOffNeighbors.Num();

				for (const FIntVector& AroundGrid : RunOffNeighbors)
				{
					FClimateData& AroundClimate = TerrainClimate[AroundGrid.Y][AroundGrid.X];
					AroundClimate.Humidity += RunOff;
				}
			}

			// seepage
			if (SeepageNeighbors.Num() > 0 && SeepageRatio > 0.0f)
			{
				float Seepage = FMath::Min(CurHumidity0 * SeepageRatio, CurHumidity);
				if (!IsUnderWater)
					CurHumidity -= Seepage;
				Seepage /= SeepageNeighbors.Num();

				for (const FIntVector& AroundGrid : SeepageNeighbors)
				{
					FClimateData& AroundClimate = TerrainClimate[AroundGrid.Y][AroundGrid.X];
					AroundClimate.Humidity += Seepage;
				}
			}

			GridClimate.Humidity += CurHumidity;
		}
	}

	// rain shadow
	for (int32 Y = 0; Y < MapSize.Y; ++Y)
	{
		for (int32 X = 0; X < MapSize.X; ++X)
		{
			const int32& Elevation = OutConfigData.ElevationsList[Y][X];
			bool IsUnderWater = Elevation < StartWaterLevel;

			FClimateData& GridClimate = TerrainClimate[Y][X];
			if (IsUnderWater)
			{
				GridClimate.CloudAmount = FMath::Min(1.0f, GridClimate.CloudAmount);
			}
			else
			{
				float CurElevation = Elevation - StartWaterLevel;
				float MaxElevation = 1.0f + ElevationRange.Y - StartWaterLevel;
				float MaxCloudAmount = 1.0f - CurElevation / MaxElevation;

				if (GridClimate.CloudAmount > MaxCloudAmount)
				{
					GridClimate.Humidity += GridClimate.CloudAmount - MaxCloudAmount;
					GridClimate.CloudAmount = MaxCloudAmount;
				}
			}

			GridClimate.Humidity = FMath::Min(1.0f, GridClimate.Humidity);
		}
	}
}

void FHexTerrainDataGenerator::CreateRivers(int32 LandCells)
{
	// Get origins
	TArray<TArray<FIntPoint>> RiverOrigins;
	RiverOrigins.AddDefaulted(3);

	for (int32 Y = 0; Y < MapSize.Y; ++Y)
	{
		for (int32 X = 0; X < MapSize.X; ++X)
		{
			const int32& Elevation = OutConfigData.ElevationsList[Y][X];
			const FClimateData& Climate = TerrainClimate[Y][X];

			FIntPoint CurGrid{ X,Y };

			float RiverOriginWeight = Climate.Humidity * float(Elevation - StartWaterLevel) / float(ElevationRange.Y - StartWaterLevel);

			if (RiverOriginWeight > 0.75f)
				RiverOrigins[0].Add(CurGrid);
			else if (RiverOriginWeight > 0.5f)
				RiverOrigins[1].Add(CurGrid);
			else if (RiverOriginWeight > 0.25f)
				RiverOrigins[2].Add(CurGrid);
		}
	}

	UE_LOG(LogTemp, Display, TEXT("RiverOrigins: %d %d %d"), RiverOrigins[0].Num(), RiverOrigins[1].Num(), RiverOrigins[2].Num());

	// Select origin & Create river
	int32 RiverCells = FMath::RoundToInt(LandCells * RiverPercentage * 0.01f);
	int32 RiverBudget = RiverCells;
	while (RiverBudget > 0)
	{
		int32 NumOfPrimes = RiverOrigins[0].Num();
		int32 NumOfGoods = RiverOrigins[1].Num();
		int32 NumOfAcceptables = RiverOrigins[2].Num();

		int32 NumOfPrimeTickets = NumOfPrimes * 4;
		int32 NumOfGoodTickets = NumOfGoods * 2;
		int32 NumOfAcceptableTickets = NumOfAcceptables;
		int32 NumOfAllTickets = NumOfPrimeTickets + NumOfGoodTickets + NumOfAcceptableTickets;
		if (NumOfAllTickets <= 0)
			break;

		int32 RandRiverOriginIndex = FMath::RandHelper(NumOfAllTickets);

		FIntPoint RiverOrigin{ ForceInitToZero };
		if (RandRiverOriginIndex < NumOfPrimeTickets)
		{
			int32 PrimeIndex = RandRiverOriginIndex / 4;
			RiverOrigin = RiverOrigins[0][PrimeIndex];
			RiverOrigins[0].RemoveAtSwap(PrimeIndex);
		}
		else if (RandRiverOriginIndex < NumOfPrimeTickets + NumOfGoodTickets)
		{
			int32 GoodIndex = (RandRiverOriginIndex - NumOfPrimeTickets) / 2;
			RiverOrigin = RiverOrigins[1][GoodIndex];
			RiverOrigins[1].RemoveAtSwap(GoodIndex);
		}
		else
		{
			int32 AcceptableIndex = (RandRiverOriginIndex - NumOfPrimeTickets - NumOfGoodTickets);
			RiverOrigin = RiverOrigins[2][AcceptableIndex];
			RiverOrigins[2].RemoveAtSwap(AcceptableIndex);
		}

		bool bIsRiverOriginValid = true;
		TArray<FIntVector> Neighbors;
		GetGridNeighbors(RiverOrigin, 1, Neighbors);
		for (const FIntVector& AroundGrid : Neighbors)
		{
			const int32& AroundElevation = OutConfigData.ElevationsList[AroundGrid.Y][AroundGrid.X];
			const int32& AroundWaterLevel = OutConfigData.WaterLevelsList[AroundGrid.Y][AroundGrid.X];
			const bool bAroundUnderWater = AroundWaterLevel > AroundElevation;

			const FRiverFlagData& AroundRiverFlags = TerrainRiverFlag[AroundGrid.Y][AroundGrid.X];
			const bool bAroundHasRiver = AroundRiverFlags.bHasInRiver || AroundRiverFlags.bHasOutRiver;

			if (bAroundUnderWater || bAroundHasRiver)
			{
				bIsRiverOriginValid = false;
				break;
			}
		}

		if (bIsRiverOriginValid)
		{
			RiverBudget -= CreateOneRiver(RiverOrigin);
		}
	}

	if (RiverBudget > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to use up all river budget, remaining(%d/%d)."), RiverBudget, RiverCells);
	}
}

int32 FHexTerrainDataGenerator::CreateOneRiver(const FIntPoint& StartGrid)
{
	const FRiverFlagData& RiverFlags = TerrainRiverFlag[StartGrid.Y][StartGrid.X];
	if (RiverFlags.bHasOutRiver)
		return 0;

	TArray<FIntVector> RiverGrids;
	RiverGrids.Add(FIntVector{ StartGrid.X, StartGrid.Y, -1 });

	auto AddToRiver = [this, &RiverGrids](const FIntVector& NewGrid, const FIntVector& OldGrid)
		{
			RiverGrids.Add(NewGrid);
			TerrainRiverFlag[OldGrid.Y][OldGrid.X].bHasOutRiver = true;
			TerrainRiverFlag[NewGrid.Y][NewGrid.X].bHasInRiver = true;
		};

	auto RemoveFromRiver = [this, &RiverGrids](const FIntVector& FromGrid, const FIntVector& ToGrid)
		{
			TerrainRiverFlag[FromGrid.Y][FromGrid.X].bHasOutRiver = false;
			TerrainRiverFlag[ToGrid.Y][ToGrid.X].bHasInRiver = false;
		};

	int32 RetryTimes = RiverRetryTimes;
	while (true)
	{
		const FIntVector& CurGrid = RiverGrids.Last();
		const int32& Elevation = OutConfigData.ElevationsList[CurGrid.Y][CurGrid.X];
		const int32& WaterLevel = OutConfigData.WaterLevelsList[CurGrid.Y][CurGrid.X];
		const FRiverFlagData& CurRiverFlags = TerrainRiverFlag[CurGrid.Y][CurGrid.X];
		if (Elevation < WaterLevel || CurRiverFlags.bHasOutRiver)
			break;

		TArray<FIntVector> Neighbors;
		GetGridNeighbors(FIntPoint{ CurGrid.X, CurGrid.Y }, 1, Neighbors);

		int32 PrevDirection = FHexCellData::CalcPreviousDirection(CurGrid.Z);
		int32 NextDirection = FHexCellData::CalcNextDirection(CurGrid.Z);

		int32 NumOfBlocksByRiver = 0;
		TArray<FIntVector> FilteredNeighbors;
		for (const FIntVector& AroundGrid : Neighbors)
		{
			const int32& AroundElevation = OutConfigData.ElevationsList[AroundGrid.Y][AroundGrid.X];
			const int32& AroundWaterLevel = OutConfigData.WaterLevelsList[AroundGrid.Y][AroundGrid.X];
			const FRiverFlagData& AroundRiverFlags = TerrainRiverFlag[AroundGrid.Y][AroundGrid.X];
			bool AroundRiverValid = !AroundRiverFlags.bHasInRiver || AroundElevation < AroundWaterLevel;
			if (AroundRiverValid && AroundElevation <= Elevation)
			{
				if (AroundElevation < Elevation)
				{
					FilteredNeighbors.Add(AroundGrid);
					FilteredNeighbors.Add(AroundGrid);
				}

				if (CurGrid.Z >= 0 && (CurGrid.Z == AroundGrid.Z || CurGrid.Z == PrevDirection || CurGrid.Z == NextDirection))
				{
					FilteredNeighbors.Add(AroundGrid);
				}

				FilteredNeighbors.Add(AroundGrid);
			}
			else if (!AroundRiverValid && AroundElevation <= Elevation)
				++NumOfBlocksByRiver;
		}

		int32 NumOfValidNeighbors = FilteredNeighbors.Num();
		if (NumOfValidNeighbors <= 0)
		{
			// Retry
			int32 NumOfRiverGrids = RiverGrids.Num();
			bool bNeedRetry = NumOfRiverGrids > 1 && (NumOfBlocksByRiver > 0 || NumOfRiverGrids < MinRiverLength);
			if (bNeedRetry && RetryTimes > 0)
			{
				FIntVector ToGrid = RiverGrids.Pop(true);
				FIntVector FromGrid = RiverGrids.Last();
				RemoveFromRiver(FromGrid, ToGrid);

				--RetryTimes;
			}
			else
				break;
		}
		else
		{
			const FIntVector& NewGrid = FilteredNeighbors[FMath::RandHelper(NumOfValidNeighbors)];
			AddToRiver(NewGrid, CurGrid);
		}
	};

	int32 NumOfRiverGrids = RiverGrids.Num();
	const bool bRiverHasValidLen = NumOfRiverGrids >= MinRiverLength;
	if (bRiverHasValidLen)
	{
		// Ending lake
		TArray<FIntVector> Neighbors;
		const FIntVector& EndGrid = RiverGrids.Last();
		int32& EndElevation = OutConfigData.ElevationsList[EndGrid.Y][EndGrid.X];
		int32& EndWaterLevel = OutConfigData.WaterLevelsList[EndGrid.Y][EndGrid.X];
		if (EndElevation >= EndWaterLevel)
		{
			int32 MinAroundElevation = ElevationRange.Y;
			GetGridNeighbors(FIntPoint{ EndGrid.X, EndGrid.Y }, 1, Neighbors);
			for (const FIntVector& AroundGrid : Neighbors)
			{
				const int32& AroundElevation = OutConfigData.ElevationsList[AroundGrid.Y][AroundGrid.X];
				MinAroundElevation = FMath::Min(MinAroundElevation, AroundElevation);
			}

			if (MinAroundElevation > EndElevation)
			{
				EndWaterLevel = MinAroundElevation;
			}
		}

		// Fill river config
		FHexRiverRoadConfigData& RiverConfig = OutConfigData.RiversList.AddDefaulted_GetRef();
		RiverConfig.StartPoint = StartGrid;
		RiverConfig.ExtensionDirections.Empty(NumOfRiverGrids - 1);

		for (int32 Index = 1; Index < NumOfRiverGrids; ++Index)
		{
			uint8 RiverDirection = uint8(RiverGrids[Index].Z);
			RiverConfig.ExtensionDirections.Add(static_cast<EHexDirection>(RiverDirection));
		}

		UE_LOG(LogTemp, Warning, TEXT("River: Len%d End(%d,%d)"), NumOfRiverGrids, EndGrid.X, EndGrid.Y);
	}
	else if (NumOfRiverGrids > 1)
	{
		FIntVector FromGrid = RiverGrids[0];
		FIntVector ToGrid = FromGrid;
		for (int32 Index = 1; Index < NumOfRiverGrids; ++Index)
		{
			ToGrid = RiverGrids[Index];
			RemoveFromRiver(FromGrid, ToGrid);
			FromGrid = ToGrid;
		}
	}

	return bRiverHasValidLen ? NumOfRiverGrids : 0;
}

void FHexTerrainDataGenerator::PaintTerrainType(EHexTerrainType FirstLayer)
{
	for (int32 Y = 0; Y < MapSize.Y; ++Y)
	{
		for (int32 X = 0; X < MapSize.X; ++X)
		{
			const int32& Elevation = OutConfigData.ElevationsList[Y][X];
			EHexTerrainType& TerrainType = OutConfigData.TerrainTypesList[Y][X];
			
			const FClimateData& Climate = TerrainClimate[Y][X];
			TerrainType = GetTerrainTypeByParameters(Climate.Humidity, Elevation - StartWaterLevel);

			OutConfigData.CustomDataList[Y][X].X = TerrainRiverFlag[Y][X].bHasInRiver ? 1.0f : 0.0f;
			OutConfigData.CustomDataList[Y][X].Y = TerrainRiverFlag[Y][X].bHasOutRiver ? 1.0f : 0.0f;

			// Coastline
			int32 Diff = Elevation - StartWaterLevel;
			if (Diff != 0)
				continue;

			TArray<FIntVector> Neighbors;
			GetGridNeighbors(FIntPoint{ X,Y }, 2, Neighbors);

			int32 DistToWater = 3;
			for (const FIntVector& AroundGrid : Neighbors)
			{
				const int32& AroundElevation = OutConfigData.ElevationsList[AroundGrid.Y][AroundGrid.X];
				if (AroundElevation < StartWaterLevel)
				{
					DistToWater = FMath::Min(DistToWater, AroundGrid.Z);
				}
			}

			if (DistToWater == 1)
				TerrainType = FMath::FRand() < BeachRatio ? FirstLayer : TerrainType;
		}
	}
}

EHexTerrainType FHexTerrainDataGenerator::GetTerrainTypeByParameters(float Humidity, int32 InElevation)
{
	EHexTerrainType OutTerrainType = EHexTerrainType::None;
	if (Humidity <= HumidityToTerrainType[EHexTerrainType::Stone])
		OutTerrainType = EHexTerrainType::Stone;
	else if (Humidity <= HumidityToTerrainType[EHexTerrainType::SmallStone])
		OutTerrainType = EHexTerrainType::SmallStone;
	else if (Humidity <= HumidityToTerrainType[EHexTerrainType::Plateau])
		OutTerrainType = EHexTerrainType::Plateau;
	else if (Humidity <= HumidityToTerrainType[EHexTerrainType::Mud])
		OutTerrainType = EHexTerrainType::Mud;
	else if (Humidity <= HumidityToTerrainType[EHexTerrainType::Grass])
		OutTerrainType = EHexTerrainType::Grass;

	if (InElevation < ElevationToTerrainType[EHexTerrainType::Sand])
		OutTerrainType = EHexTerrainType::Sand;
	else if (InElevation >= ElevationToTerrainType[EHexTerrainType::Ice])
	{
		if (Humidity > 0.2f)
			OutTerrainType = EHexTerrainType::Ice;
	}
	return OutTerrainType;
}

void FHexTerrainDataGenerator::GetGridNeighbors(const FIntPoint& CurGrid, int32 MaxDist, TArray<FIntVector>& OutNeighbors)
{
	if (MaxDist <= 0)
	{
		OutNeighbors.Empty();
	}
	else if (MaxDist == 1)
	{
		const TArray<FIntPoint>& AroundGrids = CurGrid.Y % 2 == 0 ? EvenAroundGrids : OddAroundGrids;
		int32 NumOfAroundGrids = AroundGrids.Num();
		OutNeighbors.Empty(NumOfAroundGrids);
		for (int32 Index = 0; Index < NumOfAroundGrids; ++Index)
		{
			FIntPoint AroundGrid = CurGrid + AroundGrids[Index];
			if (AroundGrid.X < 0 || AroundGrid.X >= MapSize.X ||
				AroundGrid.Y < 0 || AroundGrid.Y >= MapSize.Y)
				continue;

			OutNeighbors.Add(FIntVector{ AroundGrid.X, AroundGrid.Y, Index });
		}
	}
	else
	{
		TMap<FIntPoint, int32> OutputGrids;
		TMap<FIntPoint, int32> CheckGrids;
		CheckGrids.Add(CurGrid, 0);

		FIntVector CurCoord = FHexCellData::CalcGridCoordinate(CurGrid);
		FSetElementId ZeroId = FSetElementId::FromInteger(0);

		while (CheckGrids.Num() > 0)
		{
			if (!CheckGrids.IsValidId(ZeroId))
			{
				CheckGrids.Compact();
			}

			TPair<FIntPoint, int32> CheckPair = CheckGrids.Get(ZeroId);
			FIntPoint OneCheckGrid = CheckPair.Key;
			CheckGrids.Remove(OneCheckGrid);
			if (CheckPair.Value > 0)
				OutputGrids.Add(OneCheckGrid, CheckPair.Value);

			if (CheckPair.Value >= MaxDist)
				continue;

			const TArray<FIntPoint>& AroundGrids = OneCheckGrid.Y % 2 == 0 ? EvenAroundGrids : OddAroundGrids;
			int32 NumOfAroundGrids = AroundGrids.Num();
			for (int32 Index = 0; Index < NumOfAroundGrids; ++Index)
			{
				FIntPoint AroundGrid = OneCheckGrid + AroundGrids[Index];
				if (AroundGrid.X < 0 || AroundGrid.X >= MapSize.X ||
					AroundGrid.Y < 0 || AroundGrid.Y >= MapSize.Y)
					continue;

				if (CheckGrids.Contains(AroundGrid) || OutputGrids.Contains(AroundGrid))
					continue;

				FIntVector AroundCoord = FHexCellData::CalcGridCoordinate(AroundGrid);
				int32 Distance = AHexTerrainNavigator::CalcDirectDistance(CurCoord, AroundCoord);

				CheckGrids.Add(AroundGrid, Distance);
			}
		}

		OutNeighbors.Empty(OutputGrids.Num());
		for (TMap<FIntPoint, int32>::TConstIterator SetIt(OutputGrids); SetIt; ++SetIt)
		{
			OutNeighbors.Add(FIntVector{ SetIt->Key.X, SetIt->Key.Y, SetIt->Value });
		}
	}
}

EHexDirection FHexTerrainDataGenerator::GetHexDirection(const FString& InTypeStr)
{
	if (InTypeStr.Equals(TEXT("E")))
		return EHexDirection::E;
	else if (InTypeStr.Equals(TEXT("SE")))
		return EHexDirection::SE;
	else if (InTypeStr.Equals(TEXT("SW")))
		return EHexDirection::SW;
	else if (InTypeStr.Equals(TEXT("W")))
		return EHexDirection::W;
	else if (InTypeStr.Equals(TEXT("NW")))
		return EHexDirection::NW;
	else if (InTypeStr.Equals(TEXT("NE")))
		return EHexDirection::NE;
	else
	{
		check(0);
		return EHexDirection::E;
	}
}

#pragma optimize("", on)