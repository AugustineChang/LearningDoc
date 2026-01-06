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

FHexTerrainDataGenerator::FHexTerrainDataGenerator(const FIntPoint& GridSize, const FIntPoint& ChunkSize, FHexCellConfigData& OutData)
	: GridSize(GridSize), ChunkSize(ChunkSize), OutConfigData(OutData),
	ElevationRange(-4,10), StartElevation(-1), StartWaterLevel(0),
	LandRandomness(0.5f), LandPercentage(50), NumOfGridsPerLand(50, 200), HighRiseProbability(0.0f), SinkProbability(0.0f)
{
	ElevationToTerrainType.Add(EHexTerrainType::Sand, 0);
	ElevationToTerrainType.Add(EHexTerrainType::Grass, 2);
	ElevationToTerrainType.Add(EHexTerrainType::Stone, 4);
	ElevationToTerrainType.Add(EHexTerrainType::Moor, 6);
	ElevationToTerrainType.Add(EHexTerrainType::Ice, ElevationRange.Y);

	LoadConfigFromFile();
	StartElevation = FMath::Clamp(StartElevation, ElevationRange.X, ElevationRange.Y);
}

void FHexTerrainDataGenerator::GenerateData()
{
	OutConfigData.bConfigValid = true;
	OutConfigData.HexChunkSize = ChunkSize;
	OutConfigData.HexChunkCount = GridSize / ChunkSize;

	EHexTerrainType FirstLayerType = ElevationToTerrainType.Get(FSetElementId::FromInteger(0)).Key;
	EHexTerrainType SecondLayerType = ElevationToTerrainType.Get(FSetElementId::FromInteger(1)).Key;

	// Init
	OutConfigData.ElevationsList.Empty(GridSize.Y);
	OutConfigData.ElevationsList.AddDefaulted(GridSize.Y);
	OutConfigData.WaterLevelsList.Empty(GridSize.Y);
	OutConfigData.WaterLevelsList.AddDefaulted(GridSize.Y);
	OutConfigData.TerrainTypesList.Empty(GridSize.Y);
	OutConfigData.TerrainTypesList.AddDefaulted(GridSize.Y);
	OutConfigData.FeatureValuesList.Empty(GridSize.Y);
	OutConfigData.FeatureValuesList.AddDefaulted(GridSize.Y);
	for (int32 Y = 0; Y < GridSize.Y; ++Y)
	{
		OutConfigData.ElevationsList[Y].Init(StartElevation, GridSize.X);
		OutConfigData.WaterLevelsList[Y].Init(StartWaterLevel, GridSize.X);
		OutConfigData.TerrainTypesList[Y].Init(FirstLayerType, GridSize.X);
		OutConfigData.FeatureValuesList[Y].AddZeroed(GridSize.X);
	}
	
	// Raise Lands
	int32 LandBudget = FMath::RoundToInt(GridSize.X * GridSize.Y * LandPercentage * 0.01f);
	while(LandBudget > 0)
	{
		bool bSinkTerrain = FMath::FRand() < SinkProbability;
		int32 DesiredLandGrids = FMath::Min(LandBudget, FMath::RandRange(NumOfGridsPerLand.X, NumOfGridsPerLand.Y));
		int32 ChangedLandGrids = RaiseSinkTerrain(DesiredLandGrids, bSinkTerrain);
		if (bSinkTerrain)
			LandBudget += ChangedLandGrids;
		else 
			LandBudget -= ChangedLandGrids;
	}

	// Coastline
	GenerateCoastline(FirstLayerType, SecondLayerType);
}

void FHexTerrainDataGenerator::LoadConfigFromFile()
{
	FString ConfigFilePath = FPaths::ProjectConfigDir() + TEXT("GeneratingConfig.ini");
	FConfigFile ConfigFile;
	if (FPaths::FileExists(ConfigFilePath))
	{
		ConfigFile.Read(ConfigFilePath);

		static FString ConfigCommonSectionName{ TEXT("Common") };
		static FString ConfigElevationMinName{ TEXT("ElevationMin") };
		static FString ConfigElevationMaxName{ TEXT("ElevationMax") };
		static FString ConfigDefaultElevationName{ TEXT("DefaultElevation") };
		static FString ConfigDefaultWaterLevelName{ TEXT("DefaultWaterLevel") };
		static FString ConfigSandElevationName{ TEXT("SandElevation") };
		static FString ConfigGrassElevationName{ TEXT("GrassElevation") };
		static FString ConfigStoneElevationName{ TEXT("StoneElevation") };
		static FString ConfigMoorElevationName{ TEXT("MoorElevation") };
		static FString ConfigIceElevationName{ TEXT("IceElevation") };

		static FString ConfigRaiseSinkTerrainSectionName{ TEXT("RaiseSinkTerrain") };
		static FString ConfigLandPercentageName{ TEXT("LandPercentage") };
		static FString ConfigLandRandomnessName{ TEXT("LandRandomness") };
		static FString ConfigLandGridsRangeMinName{ TEXT("LandGridsRangeMin") };
		static FString ConfigLandGridsRangeMaxName{ TEXT("LandGridsRangeMax") };
		static FString ConfigHighRiseProbabilityName{ TEXT("HighRiseProbability") };
		static FString ConfigSinkProbabilityName{ TEXT("SinkProbability") };
		
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigElevationMinName, ElevationRange.X);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigElevationMaxName, ElevationRange.Y);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigDefaultElevationName, StartElevation);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigDefaultWaterLevelName, StartWaterLevel);

		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigSandElevationName, ElevationToTerrainType[EHexTerrainType::Sand]);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigGrassElevationName, ElevationToTerrainType[EHexTerrainType::Grass]);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigStoneElevationName, ElevationToTerrainType[EHexTerrainType::Stone]);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigMoorElevationName, ElevationToTerrainType[EHexTerrainType::Moor]);
		ConfigFile.GetInt(*ConfigCommonSectionName, *ConfigIceElevationName, ElevationToTerrainType[EHexTerrainType::Ice]);
		ElevationToTerrainType.ValueSort([](const int32& Val0, const int32& Val1) { return Val0 < Val1; });

		ConfigFile.GetInt(*ConfigRaiseSinkTerrainSectionName, *ConfigLandPercentageName, LandPercentage);
		ConfigFile.GetFloat(*ConfigRaiseSinkTerrainSectionName, *ConfigLandRandomnessName, LandRandomness);
		ConfigFile.GetInt(*ConfigRaiseSinkTerrainSectionName, *ConfigLandGridsRangeMinName, NumOfGridsPerLand.X);
		ConfigFile.GetInt(*ConfigRaiseSinkTerrainSectionName, *ConfigLandGridsRangeMaxName, NumOfGridsPerLand.Y);
		ConfigFile.GetFloat(*ConfigRaiseSinkTerrainSectionName, *ConfigHighRiseProbabilityName, HighRiseProbability);
		ConfigFile.GetFloat(*ConfigRaiseSinkTerrainSectionName, *ConfigSinkProbabilityName, SinkProbability);
	}
}

int32 FHexTerrainDataGenerator::RaiseSinkTerrain(int32 NumOfGrids, bool bSink)
{
	SelectedGrids.Empty();
	NeighborGrids.Empty();
	NeighborGrids.Add(FIntPoint{ FMath::RandHelper(GridSize.X) , FMath::RandHelper(GridSize.Y) }, 0);
	
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

		const int32& WaterLevel = OutConfigData.WaterLevelsList[SelectedGrid.Y][SelectedGrid.X];
		OutConfigData.TerrainTypesList[SelectedGrid.Y][SelectedGrid.X] = GetTerrainTypeByElevation(Elevation - WaterLevel);
		if ((OldElevation < WaterLevel) != (Elevation < WaterLevel)) 
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
		if (AroundGrid.X < 0 || AroundGrid.X >= GridSize.X ||
			AroundGrid.Y < 0 || AroundGrid.Y >= GridSize.Y)
			continue;

		if (NeighborGrids.Contains(AroundGrid) || SelectedGrids.Contains(AroundGrid))
			continue;

		FIntVector AroundCoord = FHexCellData::CalcGridCoordinate(AroundGrid);
		NeighborGrids.Add(AroundGrid, AHexTerrainNavigator::CalcDirectDistance(FirstCoord, AroundCoord));
	}
	
	return true;
}

void FHexTerrainDataGenerator::GetGridNeighbors(const FIntPoint& CurGrid, int32 MaxDist, TArray<FIntVector>& OutNeighbors)
{
	TMap<FIntPoint, int32> OutputGrids;
	TMap<FIntPoint, int32> CheckGrids;
	CheckGrids.Add(CurGrid, 0);

	FIntVector CurCoord = FHexCellData::CalcGridCoordinate(CurGrid);

	while (CheckGrids.Num() > 0)
	{
		FSetElementId ZeroId = FSetElementId::FromInteger(0);
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
			if (AroundGrid.X < 0 || AroundGrid.X >= GridSize.X ||
				AroundGrid.Y < 0 || AroundGrid.Y >= GridSize.Y)
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

EHexTerrainType FHexTerrainDataGenerator::GetTerrainTypeByElevation(int32 InElevation)
{
	for (TMap<EHexTerrainType, int32>::TConstIterator PairIt(ElevationToTerrainType); PairIt; ++PairIt)
	{
		if (InElevation <= PairIt->Value)
			return PairIt->Key;
	}

	return EHexTerrainType::None;
}

void FHexTerrainDataGenerator::GenerateCoastline(EHexTerrainType FirstLayer, EHexTerrainType SecondLayer)
{
	for (int32 Y = 0; Y < GridSize.Y; ++Y)
	{
		const TArray<FIntPoint>& AroundGrids = Y % 2 == 0 ? EvenAroundGrids : OddAroundGrids;
		for (int32 X = 0; X < GridSize.X; ++X)
		{
			const int32& Elevation = OutConfigData.ElevationsList[Y][X];
			const int32& WaterLevel = OutConfigData.WaterLevelsList[Y][X];
			EHexTerrainType& TerrainType = OutConfigData.TerrainTypesList[Y][X];

			int32 Diff = Elevation - WaterLevel;
			if (Diff < 0 || Diff > 1)
				continue;
			
			TArray<FIntVector> Neighbors;
			GetGridNeighbors(FIntPoint{ X,Y }, 2, Neighbors);

			int32 DistToWater = 3;
			for (const FIntVector& AroundGrid : Neighbors)
			{
				const int32& AroundElevation = OutConfigData.ElevationsList[AroundGrid.Y][AroundGrid.X];
				const int32& AroundWaterLevel = OutConfigData.WaterLevelsList[AroundGrid.Y][AroundGrid.X];
				if (AroundElevation < AroundWaterLevel)
				{
					DistToWater = FMath::Min(DistToWater, AroundGrid.Z);
				}
			}

			if (DistToWater == 1)
				TerrainType = FirstLayer;
			else if (DistToWater == 2)
				TerrainType = FMath::FRand() < 0.25f ? FirstLayer : SecondLayer;
			else
				TerrainType = SecondLayer;
		}
	}
}