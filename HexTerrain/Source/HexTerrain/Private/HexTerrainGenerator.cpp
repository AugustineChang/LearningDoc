#include "HexTerrainGenerator.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Kismet/GameplayStatics.h"

#define	CORNER_NUM 6

#pragma optimize("", off)

//////////////////////////////////////////////////////////////////////////
FIntPoint FHexCellData::ChunkSize{ 0, 0 };
int32 FHexCellData::ChunkCountX = 0;
int32 FHexCellData::MaxTerranceElevation = 0;
TArray<FVector> FHexCellData::HexVertices;
TArray<FVector> FHexCellData::HexSubVertices;

FHexCellData::FHexCellData(const FIntPoint& InIndex)
	: GridIndex(InIndex.X + InIndex.Y * ChunkSize.X * ChunkCountX)
	, GridCoord(CalcGridCoordinate(InIndex))
{
	GridId.X = InIndex.X / ChunkSize.X;
	GridId.Y = InIndex.Y / ChunkSize.Y;
	GridId.Z = InIndex.X - GridId.X * ChunkSize.X;
	GridId.W = InIndex.Y - GridId.Y * ChunkSize.Y;
}

void FHexCellData::LinkCell(FHexCellData& OtherCell, EHexDirection LinkDirection)
{
	EHexLinkState LinkState = CalcLinkState(OtherCell, *this);

	uint8 LinkId = static_cast<uint8>(LinkDirection);
	uint8 OtherLinkId = static_cast<uint8>(CalcOppositeDirection(LinkDirection));

	FHexCellLink& Link1 = HexNeighbors[LinkId];
	Link1.LinkedCellId = OtherCell.GridIndex;
	Link1.LinkState = LinkState;

	Link1.FromVert.Y = LinkId;
	Link1.FromVert.X = (Link1.FromVert.Y + CORNER_NUM - 1) % CORNER_NUM;
	Link1.ToVert.X = OtherLinkId;
	Link1.ToVert.Y = (Link1.ToVert.X + CORNER_NUM - 1) % CORNER_NUM;
	
	FHexCellLink& Link2 = OtherCell.HexNeighbors[OtherLinkId];
	Link2.LinkedCellId = GridIndex;
	Link2.LinkState = LinkState;

	Link2.FromVert.Y = OtherLinkId;
	Link2.FromVert.X = (Link2.FromVert.Y + CORNER_NUM - 1) % CORNER_NUM;
	Link2.ToVert.X = LinkId;
	Link2.ToVert.Y = (Link2.ToVert.X + CORNER_NUM - 1) % CORNER_NUM;
}

void FHexCellData::LinkCorner(FHexCellData& Cell1, FHexCellData& Cell2, EHexDirection LinkDirection)
{
	uint8 LinkId = static_cast<uint8>(LinkDirection);

	FHexCellCorner& Corner = HexCorners[LinkId - 4];
	Corner.LinkedCellsId.X = GridIndex;
	Corner.LinkedCellsId.Y = Cell1.GridIndex;
	Corner.LinkedCellsId.Z = Cell2.GridIndex;

	Corner.LinkState[0] = CalcLinkState(*this, Cell1);
	Corner.LinkState[1] = CalcLinkState(Cell1, Cell2);
	Corner.LinkState[2] = CalcLinkState(Cell2, *this);

	Corner.VertsId.X = LinkId - 1;
	Corner.VertsId.Y = LinkId - 3;
	Corner.VertsId.Z = (LinkId - 5 + CORNER_NUM) % CORNER_NUM;
}

bool FHexCellData::operator<(const FHexCellData& Other) const
{
	return Elevation < Other.Elevation;
}

FIntVector FHexCellData::CalcGridCoordinate(const FIntPoint& InGridIndex)
{
	int32 CoordX = InGridIndex.X - InGridIndex.Y / 2;
	int32 CoordZ = InGridIndex.Y;
	return FIntVector{ CoordX, -CoordX - CoordZ, CoordZ };
}

EHexDirection FHexCellData::CalcOppositeDirection(EHexDirection InDirection)
{
	uint8 DirNum = static_cast<uint8>(InDirection);
	return static_cast<EHexDirection>((DirNum + 3u) % 6u);
}

int32 FHexCellData::CalcGridIndexByCoord(const FIntVector& InGridCoord)
{
	int32 IndexY = InGridCoord.Z;
	int32 IndexX = InGridCoord.X + IndexY / 2;

	int32 RowSize = ChunkSize.X * ChunkCountX;
	if (IndexX >= 0 && IndexX < RowSize && IndexY >= 0)
		return IndexX + IndexY * RowSize;
	else
		return -1;
}

EHexLinkState FHexCellData::CalcLinkState(const FHexCellData& Cell1, const FHexCellData& Cell2)
{
	int32 ElevationDiff = FMath::Abs(Cell1.Elevation - Cell2.Elevation);
	EHexLinkState LinkState = EHexLinkState::Flat;
	if (ElevationDiff == 0)
		LinkState = EHexLinkState::Flat;
	else if (ElevationDiff == 1)
		LinkState = EHexLinkState::Slope;
	else if (ElevationDiff <= MaxTerranceElevation)
		LinkState = EHexLinkState::Terrace;
	else
		LinkState = EHexLinkState::Cliff;
	return LinkState;
}

double FUniqueVertexArray::VectorTolerence = 1e-2;

FHexVertexAttributeData& FUniqueVertexArray::FindOrAddVertex(const FVector& InVertex, bool& bFound)
{
	FBoxCenterAndExtent QueryBox{ InVertex, VectorTolerence * FVector::OneVector };

	float MinDistance = FLT_MAX;
	TSharedPtr<FOctreeElementId2> NearestResults;

	VectorOctree.FindElementsWithBoundsTest(QueryBox,
		[&NearestResults, &MinDistance, &InVertex](const FHexVertexAttributeData& Element)
		{
			float Distance = (InVertex - Element.VertexPos).SquaredLength();
			if (!NearestResults.IsValid() || Distance < MinDistance)
			{
				MinDistance = Distance;
				NearestResults = Element.OctreeId;
			}
		});

	bFound = NearestResults.IsValid();
	if (NearestResults.IsValid())
	{
		return VectorOctree.GetElementById(*NearestResults);
	}
	else
	{
		FHexVertexAttributeData NewElement{ InVertex, MakeShareable(new FOctreeElementId2()) };
		VectorOctree.AddElement(NewElement);

		return VectorOctree.GetElementById(*NewElement.OctreeId);
	}
}

struct FCachedSectionData
{
	TArray<FVector> Vertices;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0s;
	TArray<FVector2D> UV1s;
	TArray<FColor> VertexColors;
	TArray<int32> Triangles;
	TArray<FProcMeshTangent> Tangents;
	
	//FBox BoundingBox;
	
	void MeshSection(const FCachedSectionData &Other)
	{
		int32 BaseIndex = Vertices.Num();
		Vertices.Append(Other.Vertices);
		Normals.Append(Other.Normals);
		VertexColors.Append(Other.VertexColors);

		int32 NumOfIndices = Other.Triangles.Num();
		Triangles.Reserve(Other.Triangles.Num() + NumOfIndices);
		for (int32 Index : Other.Triangles)
			Triangles.Add(BaseIndex + Index);
	}
};

//////////////////////////////////////////////////////////////////////////

// Sets default values
AHexTerrainGenerator::AHexTerrainGenerator()
	: NoiseTexturePath(TEXT("Content/Noise.png"))
	, HexChunkCount(4, 3)
	, HexChunkSize(5, 5)
	, HexCellRadius(100.0f)
	, HexCellBorderWidth(10.0f)
	, HexCellSubdivision(2u)
	, HexElevationStep(5.0f)
	, MaxElevationForTerrace(4)
	, PerturbingStrengthHV(1.0f, 1.0f)
	, PerturbingScalingHV(0.25f, 1.0f)

	, HexEditGridId(-1, -1)
	, HexEditElevation(0)
	, HexEditTerrainType(EHexTerrainType::None)
{
 	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));

	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMeshComponent"));
	ProceduralMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProceduralMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	ProceduralMeshComponent->Mobility = EComponentMobility::Movable;
	ProceduralMeshComponent->SetGenerateOverlapEvents(false);
	ProceduralMeshComponent->SetupAttachment(RootComponent);
	ProceduralMeshComponent->OnClicked.AddDynamic(this, &AHexTerrainGenerator::OnClicked);
	ProceduralMeshComponent->OnReleased.AddDynamic(this, &AHexTerrainGenerator::OnReleased);

	CoordTextComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GridCoordComponent"));
	CoordTextComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoordTextComponent->Mobility = EComponentMobility::Movable;
	CoordTextComponent->SetGenerateOverlapEvents(false);
	CoordTextComponent->SetCastShadow(false);
	CoordTextComponent->SetupAttachment(RootComponent);
	CoordTextComponent->SetRelativeLocation(FVector{ 0.0, 0.0, 10.0 });

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultPlane(TEXT("/Engine/BasicShapes/Plane"));
	CoordTextComponent->SetStaticMesh(DefaultPlane.Object);
}

// Called when the game starts or when spawned
void AHexTerrainGenerator::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AHexTerrainGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHexTerrainGenerator::LoadTerrain()
{
	// Load Config
	ConfigData.bConfigValid = LoadHexTerrainConfig();
}

void AHexTerrainGenerator::SaveTerrain()
{
	SaveHexTerrainConfig();
}

void AHexTerrainGenerator::GenerateTerrain()
{
	if (!ConfigData.bConfigValid)
		return;

	if (NoiseTexture.IsEmpty())
	{
		TArray<uint8> TextureBinData;
		FFileHelper::LoadFileToArray(TextureBinData, *(FPaths::ProjectDir() / NoiseTexturePath));
		CreateTextureFromData(NoiseTexture, TextureBinData, EImageFormat::PNG);
	}

	FHexCellData::ChunkSize = HexChunkSize;
	FHexCellData::ChunkCountX = HexChunkCount.X;
	FHexCellData::MaxTerranceElevation = MaxElevationForTerrace;
	FHexCellData::HexVertices.Empty(6);
	FHexCellData::HexSubVertices.Empty(6 * HexCellSubdivision);
	CachedNoiseZ.Empty(10);

	// Create HexCellData
	int32 HexGridSizeX = HexChunkCount.X * HexChunkSize.X;
	int32 HexGridSizeY = HexChunkCount.Y * HexChunkSize.Y;
	HexGrids.Empty(HexGridSizeX * HexGridSizeY);
	for (int32 Y = 0; Y < HexGridSizeY; ++Y)
	{
		for (int32 X = 0; X < HexGridSizeX; ++X)
		{
			FIntPoint GridIndex{ X, Y };

			FHexCellData OneCell{ GridIndex };

			//OneCell.LinearColor = FLinearColor::MakeRandomColor();
			//OneCell.SRGBColor = OneCell.LinearColor.ToFColorSRGB();
			ConfigData.GetHexCellTerrainData(OneCell.GridIndex, OneCell.SRGBColor, OneCell.Elevation);
			OneCell.CellCenter = CalcHexCellCenter(GridIndex, OneCell.Elevation);

			int32 WIndex  = FHexCellData::CalcGridIndexByCoord(FIntVector{ OneCell.GridCoord.X - 1, OneCell.GridCoord.Y + 1, OneCell.GridCoord.Z });
			int32 NWIndex = FHexCellData::CalcGridIndexByCoord(FIntVector{ OneCell.GridCoord.X, OneCell.GridCoord.Y + 1, OneCell.GridCoord.Z - 1 });
			int32 NEIndex = FHexCellData::CalcGridIndexByCoord(FIntVector{ OneCell.GridCoord.X + 1, OneCell.GridCoord.Y, OneCell.GridCoord.Z - 1 });
			if (WIndex >= 0)
				HexGrids[WIndex].LinkCell(OneCell, EHexDirection::E);
			if (NWIndex >= 0)
				HexGrids[NWIndex].LinkCell(OneCell, EHexDirection::SE);
			if (NEIndex >= 0)
				HexGrids[NEIndex].LinkCell(OneCell, EHexDirection::SW);
			if (WIndex >= 0 && NWIndex >= 0)
				OneCell.LinkCorner(HexGrids[NWIndex], HexGrids[WIndex], EHexDirection::NW);
			if (NWIndex >= 0 && NEIndex >= 0)
				OneCell.LinkCorner(HexGrids[NEIndex], HexGrids[NWIndex], EHexDirection::NE);

			HexGrids.Add(OneCell);
		}
	}

	// Create HexCellMesh
	FCachedSectionData CollisionMeshSection;
	ProceduralMeshComponent->ClearAllMeshSections();
	for (int32 CY = 0; CY < HexChunkCount.Y; ++CY)
	{
		for (int32 CX = 0; CX < HexChunkCount.X; ++CX)
		{
			int32 ChunkIndex = CY * HexChunkCount.X + CX;
			FCachedSectionData MeshSection;
			for (int32 GY = 0; GY < HexChunkSize.Y; ++GY)
			{
				for (int32 GX = 0; GX < HexChunkSize.X; ++GX)
				{
					int32 GridId = (CY * HexChunkSize.Y + GY) * HexGridSizeX + (CX * HexChunkSize.X + GX);
					const FHexCellData& CellData = HexGrids[GridId];

					FCachedSectionData CellMesh, CellCollisionMesh;
					GenerateHexCell(CellData, CellMesh, CellCollisionMesh);

					MeshSection.MeshSection(CellMesh);
					CollisionMeshSection.MeshSection(CellCollisionMesh);
				}
			}

			// Submit Mesh
			ProceduralMeshComponent->CreateMeshSection(ChunkIndex, MeshSection.Vertices, MeshSection.Triangles, MeshSection.Normals,
				MeshSection.UV0s, MeshSection.VertexColors, MeshSection.Tangents, false);

			// Set Material
			if (!!HexTerrainMaterial)
			{
				ProceduralMeshComponent->SetMaterial(ChunkIndex, HexTerrainMaterial);
			}
		}
	}

	// Create Collision
	ProceduralMeshComponent->CreateMeshSection(HexChunkCount.X * HexChunkCount.Y, CollisionMeshSection.Vertices, CollisionMeshSection.Triangles,
		CollisionMeshSection.Normals, CollisionMeshSection.UV0s, CollisionMeshSection.VertexColors, CollisionMeshSection.Tangents, true);
	ProceduralMeshComponent->SetMeshSectionVisible(HexChunkCount.X * HexChunkCount.Y, false);

	// Grid Coordinates
	if (!!TextMaterial)
	{
		CoordTextComponent->SetMaterial(0, TextMaterial);
	}
	CoordTextComponent->ClearInstances();
	CoordTextComponent->SetNumCustomDataFloats(2);
	for (int32 Y = 0; Y < HexGridSizeY; ++Y)
	{
		for (int32 X = 0; X < HexGridSizeX; ++X)
		{
			int32 GridId = Y * HexGridSizeX + X;
			
			const FHexCellData& CellData = HexGrids[GridId];

			FTransform Instance{ CellData.CellCenter };
			CoordTextComponent->AddInstance(Instance, false);
			CoordTextComponent->SetCustomDataValue(GridId, 0, CellData.GridCoord.X);
			CoordTextComponent->SetCustomDataValue(GridId, 1, CellData.GridCoord.Z);
		}
	}
	CoordTextComponent->MarkRenderStateDirty();
}

bool AHexTerrainGenerator::LoadHexTerrainConfig()
{
	FString ConfigFilePath = FPaths::ProjectConfigDir() + TEXT("HexTerrainConfig.json");
	UE_LOG(LogTemp, Display, TEXT("Load Config From %s"), *ConfigFilePath);

	FString StructuredJson;
	if (!FFileHelper::LoadFileToString(StructuredJson, *ConfigFilePath))
		return false;

	TUniquePtr<FArchive> JsonFileReader;
	JsonFileReader.Reset(new FBufferReader(StructuredJson.GetCharArray().GetData(), sizeof(FString::ElementType) * StructuredJson.Len(), false));

	TSharedPtr<FJsonObject> JsonRoot = MakeShareable(new FJsonObject);
	TSharedRef<TJsonReader<FString::ElementType> > JsonReader = TJsonReader<FString::ElementType>::Create(JsonFileReader.Get());
	if (!FJsonSerializer::Deserialize(JsonReader, JsonRoot))
	{
		JsonRoot.Reset();
		return false;
	}

	TArray<TSharedPtr<FJsonValue>> ElevationsList = JsonRoot->GetArrayField(TEXT("Elevations"));
	TSharedPtr<FJsonObject> ColorsMap = JsonRoot->GetObjectField(TEXT("Colors"));
	TArray<TSharedPtr<FJsonValue>> TypesList = JsonRoot->GetArrayField(TEXT("HexTypes"));
	
	int32 HexGridSizeX = HexChunkCount.X * HexChunkSize.X;
	int32 HexGridSizeY = HexChunkCount.Y * HexChunkSize.Y;
	ConfigData.ElevationsList.Init(0, HexGridSizeX * HexGridSizeY);
	for (int32 Y = 0; Y < ElevationsList.Num(); ++Y)
	{
		const TArray<TSharedPtr<FJsonValue>>& OneRow = ElevationsList[Y]->AsArray();
		for (int32 X = 0; X < OneRow.Num(); ++X)
		{
			int32 TempVal = 0;
			OneRow[X]->TryGetNumber(TempVal);
			ConfigData.ElevationsList[Y * HexGridSizeX + X] = TempVal;
		}
	}
	
	for (uint8 Index = 0u; Index < uint8(EHexTerrainType::MAX); ++Index)
	{
		FString OutColorStr;
		EHexTerrainType TerrainType = EHexTerrainType(Index);
		FString InTerrainTypeStr = FHexCellConfigData::GetHexTerrainString(TerrainType);
		if (ColorsMap->TryGetStringField(InTerrainTypeStr, OutColorStr))
		{
			//FLinearColor TempVal;
			//TempVal.InitFromString(OutColorStr);
			//ConfigData.ColorsMap.Add(TerrainType, TempVal.ToFColor(false));

			FColor TempVal;
			TempVal.InitFromString(OutColorStr);
			ConfigData.ColorsMap.Add(TerrainType, TempVal);
		}
	}
	
	ConfigData.TerrainTypesList.Init(EHexTerrainType::Water, HexGridSizeX * HexGridSizeY);
	for (int32 Y = 0; Y < TypesList.Num(); ++Y)
	{
		const TArray<TSharedPtr<FJsonValue>>& OneRow = TypesList[Y]->AsArray();
		for (int32 X = 0; X < OneRow.Num(); ++X)
		{
			uint8 TempVal = 0;
			OneRow[X]->TryGetNumber(TempVal);
			TempVal = FMath::Clamp(TempVal, 0u, uint8(EHexTerrainType::MAX));
			ConfigData.TerrainTypesList[Y * HexGridSizeX + X] = static_cast<EHexTerrainType>(TempVal);
		}
	}
	
	return true;
}

void AHexTerrainGenerator::SaveHexTerrainConfig()
{
	FString StructuredJson;
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	int32 HexGridSizeX = HexChunkCount.X * HexChunkSize.X;
	int32 HexGridSizeY = HexChunkCount.Y * HexChunkSize.Y;

	TArray<TSharedPtr<FJsonValue>> ElevationsList;
	TArray<TSharedPtr<FJsonValue>> TypesList;

	for (int32 Y = 0; Y < HexGridSizeY; ++Y)
	{
		TArray<TSharedPtr<FJsonValue>> ElevationRow;
		TArray<TSharedPtr<FJsonValue>> TypeRow;

		for (int32 X = 0; X < HexGridSizeX; ++X)
		{
			int32 GridIndex = Y * HexChunkSize.X * HexChunkCount.X + X;
			ElevationRow.Add(MakeShared<FJsonValueNumber>(ConfigData.ElevationsList[GridIndex]));
			TypeRow.Add(MakeShared<FJsonValueNumber>(uint8(ConfigData.TerrainTypesList[GridIndex])));
		}

		ElevationsList.Add(MakeShared<FJsonValueArray>(ElevationRow));
		TypesList.Add(MakeShared<FJsonValueArray>(TypeRow));
	}

	TSharedPtr<FJsonObject> ColorsMap = MakeShareable(new FJsonObject());
	for (uint8 Index = 0u; Index < uint8(EHexTerrainType::MAX); ++Index)
	{
		FString OutColorStr;
		EHexTerrainType TerrainType = EHexTerrainType(Index);
		FString InTerrainTypeStr = FHexCellConfigData::GetHexTerrainString(TerrainType);
		if (!InTerrainTypeStr.IsEmpty())
		{
			ColorsMap->SetStringField(InTerrainTypeStr, ConfigData.ColorsMap[TerrainType].ToString());
		}
	}
	
	JsonObject->SetArrayField(TEXT("Elevations"), ElevationsList);
	JsonObject->SetObjectField(TEXT("Colors"), ColorsMap);
	JsonObject->SetArrayField(TEXT("HexTypes"), TypesList);

	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&StructuredJson, /*Indent=*/0);

	if (FJsonSerializer::Serialize(JsonObject, JsonWriter) == false)
	{
		UE_LOG(LogJson, Warning, TEXT("HexTerrain: Unable to write out json"));
	}
	JsonWriter->Close();

	FString ConfigFilePath = FPaths::ProjectConfigDir() + TEXT("HexTerrainConfig.json");
	FFileHelper::SaveStringToFile(StructuredJson, *ConfigFilePath);
	UE_LOG(LogTemp, Display, TEXT("Save Config To %s"), *ConfigFilePath);
}

void AHexTerrainGenerator::GenerateHexCell(const FHexCellData& InCellData, FCachedSectionData& OutCellMesh, FCachedSectionData& OutCellCollisionMesh)
{
// Hex Vertices Index
//      4
//   /     \
// 3         5
// |         |
// 2         0
//   \     /
//      1

	if (FHexCellData::HexVertices.IsEmpty())
	{
		static double AngleStep = UE_DOUBLE_PI / 3.0;
		static double AngleStart = UE_DOUBLE_PI / 6.0;

		double Angle = AngleStart;
		for (int32 Index = 0; Index < CORNER_NUM; ++Index)
		{
			FVector Vert;
			Vert.X = HexCellRadius * FMath::Cos(Angle);
			Vert.Y = HexCellRadius * FMath::Sin(Angle);
			Vert.Z = 0.0;

			FHexCellData::HexVertices.Add(Vert);
			Angle += AngleStep;
		}

		for (int32 Index = 0; Index < CORNER_NUM; ++Index)
		{
			const FVector& StartVert = FHexCellData::HexVertices[Index];
			const FVector& EndVert = FHexCellData::HexVertices[(Index + 1) % CORNER_NUM];
			for (int32 SubIndex = 1; SubIndex <= HexCellSubdivision; ++SubIndex)
			{
				FVector Vert = FMath::Lerp(StartVert, EndVert, float(SubIndex) / float(HexCellSubdivision + 1));
				FHexCellData::HexSubVertices.Add(Vert);
			}
		}
	}
	
	int32 NumOfVerts = CORNER_NUM * (1 + HexCellSubdivision);
	OutCellMesh.Vertices.Empty(NumOfVerts + 1);
	OutCellMesh.Triangles.Empty(NumOfVerts * 3);
	OutCellMesh.Normals.Empty(NumOfVerts + 1);
	OutCellMesh.VertexColors.Empty(NumOfVerts + 1);

	// Inner HexCell
	OutCellMesh.Vertices.Add(InCellData.CellCenter);
	PerturbingVertexInline(OutCellMesh.Vertices.Last(), InCellData.Elevation);
	OutCellMesh.Normals.Add(FVector::UpVector);
	OutCellMesh.VertexColors.Add(InCellData.SRGBColor);

	for (int32 Index = 0; Index < CORNER_NUM; ++Index)
	{
		OutCellMesh.Vertices.Add(InCellData.CellCenter + FHexCellData::HexVertices[Index]);
		PerturbingVertexInline(OutCellMesh.Vertices.Last(), InCellData.Elevation);
		OutCellMesh.Normals.Add(FVector::UpVector);
		OutCellMesh.VertexColors.Add(InCellData.SRGBColor);

		for (int32 SubIndex = 0; SubIndex < HexCellSubdivision; ++SubIndex)
		{
			int32 SubVertIndex = Index * HexCellSubdivision + SubIndex;
			OutCellMesh.Vertices.Add(InCellData.CellCenter + FHexCellData::HexSubVertices[SubVertIndex]);
			PerturbingVertexInline(OutCellMesh.Vertices.Last(), InCellData.Elevation);
			OutCellMesh.Normals.Add(FVector::UpVector);
			OutCellMesh.VertexColors.Add(InCellData.SRGBColor);
		}
	}
	
	for (int32 Index = 0; Index < NumOfVerts; ++Index)
	{
		OutCellMesh.Triangles.Add(0);
		OutCellMesh.Triangles.Add((Index + 1) % NumOfVerts + 1);
		OutCellMesh.Triangles.Add(Index + 1);
	}

	OutCellCollisionMesh = OutCellMesh;

	// Border
	int32 WIndex = InCellData.HexNeighbors[static_cast<uint8>(EHexDirection::W)].LinkedCellId;
	int32 NWIndex = InCellData.HexNeighbors[static_cast<uint8>(EHexDirection::NW)].LinkedCellId;
	int32 NEIndex = InCellData.HexNeighbors[static_cast<uint8>(EHexDirection::NE)].LinkedCellId;
	if (WIndex >= 0) // W Edge
	{
		GenerateHexBorder(InCellData, EHexDirection::W, OutCellMesh);
	}

	if (NWIndex >= 0) // NW Edge
	{
		GenerateHexBorder(InCellData, EHexDirection::NW, OutCellMesh);
	}

	if (NEIndex >= 0) // NE Edge
	{
		GenerateHexBorder(InCellData, EHexDirection::NE, OutCellMesh);
	}
	
	if (WIndex >= 0 && NWIndex >= 0) // NW Corner
	{
		GenerateHexCorner(InCellData, EHexDirection::NW, OutCellMesh);
	}

	if (NWIndex >= 0 && NEIndex >= 0) // N Corner
	{
		GenerateHexCorner(InCellData, EHexDirection::NE, OutCellMesh);
	}
}

void AHexTerrainGenerator::GenerateHexBorder(const FHexCellData& InCellData, EHexDirection BorderDirection, FCachedSectionData& OutCellMesh)
{
	uint8 BorderDirectionId = static_cast<uint8>(BorderDirection);
	const FHexCellLink& HexLink = InCellData.HexNeighbors[BorderDirectionId];
	int32 OppositeCellId = HexLink.LinkedCellId;
	const FHexCellData& OppositeCell = HexGrids[OppositeCellId];

	TArray<FVector> FromVerts;
	TArray<FVector> ToVerts;

	FromVerts.Add(InCellData.CellCenter + FHexCellData::HexVertices[HexLink.FromVert.X]);
	ToVerts.Add(OppositeCell.CellCenter + FHexCellData::HexVertices[HexLink.ToVert.X]);

	for (int32 SubIndex = 0; SubIndex < HexCellSubdivision; ++SubIndex)
	{
		int32 SubVertIndex = HexLink.FromVert.X * HexCellSubdivision + SubIndex;
		FromVerts.Add(InCellData.CellCenter + FHexCellData::HexSubVertices[SubVertIndex]);
		SubVertIndex = HexLink.ToVert.Y * HexCellSubdivision + (HexCellSubdivision - SubIndex - 1);
		ToVerts.Add(OppositeCell.CellCenter + FHexCellData::HexSubVertices[SubVertIndex]);
	}

	FromVerts.Add(InCellData.CellCenter + FHexCellData::HexVertices[HexLink.FromVert.Y]);
	ToVerts.Add(OppositeCell.CellCenter + FHexCellData::HexVertices[HexLink.ToVert.Y]);

	if (HexLink.LinkState == EHexLinkState::Terrace)
	{
		int32 NumOfZSteps = FMath::Abs(OppositeCell.Elevation - InCellData.Elevation);
		int32 ZStepDir = OppositeCell.Elevation > InCellData.Elevation ? 1 : -1;
		int32 NumOfSteps = NumOfZSteps * 2 - 1;

		int32 NumOfSegments = FromVerts.Num() - 1;
		for (int32 Index = 0; Index < NumOfSegments; ++Index)
		{
			FColor LastStepColor = InCellData.SRGBColor;
			FVector LastStepV0 = PerturbingVertex(FromVerts[Index], InCellData.Elevation);
			FVector LastStepV1 = PerturbingVertex(FromVerts[Index + 1], InCellData.Elevation);

			for (int32 StepIndex = 1; StepIndex <= NumOfSteps; ++StepIndex)
			{
				FVector CurStepV0;
				FVector CurStepV1;

				float RatioXY = float(StepIndex) / float(NumOfSteps);
				int32 StepZIndex = (StepIndex - 1) / 2 + 1;
				float RatioZ = float(StepZIndex) / float(NumOfZSteps);

				CurStepV0.X = FMath::Lerp(FromVerts[Index].X, ToVerts[Index].X, RatioXY);
				CurStepV0.Y = FMath::Lerp(FromVerts[Index].Y, ToVerts[Index].Y, RatioXY);
				CurStepV0.Z = FMath::Lerp(FromVerts[Index].Z, ToVerts[Index].Z, RatioZ);

				CurStepV1.X = FMath::Lerp(FromVerts[Index + 1].X, ToVerts[Index + 1].X, RatioXY);
				CurStepV1.Y = FMath::Lerp(FromVerts[Index + 1].Y, ToVerts[Index + 1].Y, RatioXY);
				CurStepV1.Z = FMath::Lerp(FromVerts[Index + 1].Z, ToVerts[Index + 1].Z, RatioZ);

				FColor CurStepColor;
				CurStepColor.R = FMath::Lerp(InCellData.SRGBColor.R, OppositeCell.SRGBColor.R, RatioZ);
				CurStepColor.G = FMath::Lerp(InCellData.SRGBColor.G, OppositeCell.SRGBColor.G, RatioZ);
				CurStepColor.B = FMath::Lerp(InCellData.SRGBColor.B, OppositeCell.SRGBColor.B, RatioZ);
				CurStepColor.A = FMath::Lerp(InCellData.SRGBColor.A, OppositeCell.SRGBColor.A, RatioZ);

				int32 CurElevation = InCellData.Elevation + StepZIndex * ZStepDir;
				PerturbingVertexInline(CurStepV0, CurElevation);
				PerturbingVertexInline(CurStepV1, CurElevation);

				FillQuad(LastStepV0, LastStepV1, CurStepV0, CurStepV1, LastStepColor, LastStepColor, CurStepColor, CurStepColor, OutCellMesh);

				LastStepV0 = CurStepV0;
				LastStepV1 = CurStepV1;
				LastStepColor = CurStepColor;
			}
		}
	}
	else
	{
		int32 NumOfSegments = FromVerts.Num() - 1;
		for (int32 Index = 0; Index <= NumOfSegments; ++Index)
		{
			PerturbingVertexInline(FromVerts[Index], InCellData.Elevation);
			PerturbingVertexInline(ToVerts[Index], OppositeCell.Elevation);
		}

		for (int32 Index = 0; Index < NumOfSegments; ++Index)
		{
			FillQuad(FromVerts[Index], FromVerts[Index + 1], ToVerts[Index], ToVerts[Index + 1],
				InCellData.SRGBColor, InCellData.SRGBColor, OppositeCell.SRGBColor, OppositeCell.SRGBColor, OutCellMesh);
		}		
	}
}

void AHexTerrainGenerator::GenerateHexCorner(const FHexCellData& InCellData, EHexDirection CornerDirection, FCachedSectionData& OutCellMesh)
{
	int32 CIndex = static_cast<uint8>(CornerDirection) - 4;
	const FHexCellCorner& CornerData = InCellData.HexCorners[CIndex];

	int32 NumOfTerraces = 0;	
	if (CornerData.LinkState[0] == EHexLinkState::Terrace)
		++NumOfTerraces;
	if (CornerData.LinkState[1] == EHexLinkState::Terrace)
		++NumOfTerraces;
	if (CornerData.LinkState[2] == EHexLinkState::Terrace)
		++NumOfTerraces;

	const FHexCellData& Cell1 = HexGrids[CornerData.LinkedCellsId.X];
	const FHexCellData& Cell2 = HexGrids[CornerData.LinkedCellsId.Y];
	const FHexCellData& Cell3 = HexGrids[CornerData.LinkedCellsId.Z];
	
	if (NumOfTerraces == 0)
		GenerateNoTerraceCorner(Cell1, Cell2, Cell3, CornerData, OutCellMesh);
	else
		GenerateCornerWithTerrace(Cell1, Cell2, Cell3, CornerData, OutCellMesh);	
}

void AHexTerrainGenerator::GenerateNoTerraceCorner(const FHexCellData& Cell1, const FHexCellData& Cell2, const FHexCellData& Cell3,
	const FHexCellCorner& CornerData, FCachedSectionData& OutCellMesh)
{
	int32 BaseIndex = OutCellMesh.Vertices.Num();
	OutCellMesh.Vertices.Add(Cell1.CellCenter + FHexCellData::HexVertices[CornerData.VertsId.X]); // InnerHex: NW Corner
	OutCellMesh.Vertices.Add(Cell2.CellCenter + FHexCellData::HexVertices[CornerData.VertsId.Y]); // NW Edge:  Vert0
	OutCellMesh.Vertices.Add(Cell3.CellCenter + FHexCellData::HexVertices[CornerData.VertsId.Z]); // W  Edge:  Vert1

	PerturbingVertexInline(OutCellMesh.Vertices.Last(0), Cell3.Elevation);
	PerturbingVertexInline(OutCellMesh.Vertices.Last(1), Cell2.Elevation);
	PerturbingVertexInline(OutCellMesh.Vertices.Last(2), Cell1.Elevation);

	OutCellMesh.VertexColors.Add(Cell1.SRGBColor);
	OutCellMesh.VertexColors.Add(Cell2.SRGBColor);
	OutCellMesh.VertexColors.Add(Cell3.SRGBColor);

	FVector FaceNormal = CalcFaceNormal(OutCellMesh.Vertices[BaseIndex], OutCellMesh.Vertices[BaseIndex + 2], OutCellMesh.Vertices[BaseIndex + 1]);
	OutCellMesh.Normals.Add(FaceNormal);
	OutCellMesh.Normals.Add(FaceNormal);
	OutCellMesh.Normals.Add(FaceNormal);

	OutCellMesh.Triangles.Add(BaseIndex);
	OutCellMesh.Triangles.Add(BaseIndex + 1);
	OutCellMesh.Triangles.Add(BaseIndex + 2);
}

void AHexTerrainGenerator::GenerateCornerWithTerrace(const FHexCellData& Cell1, const FHexCellData& Cell2, const FHexCellData& Cell3, const FHexCellCorner& CornerData, FCachedSectionData& OutCellMesh)
{
	const FHexCellData* CellsList[3];
	EHexLinkState LinkState[3];
	int32 VertsList[3];

	int32 LowestZ = FMath::Min3(Cell1.Elevation, Cell2.Elevation, Cell3.Elevation);
	if (LowestZ == Cell2.Elevation)
	{
		CellsList[0] = &Cell2; 
		CellsList[1] = &Cell3; 
		CellsList[2] = &Cell1;
		VertsList[0] = CornerData.VertsId.Y;
		VertsList[1] = CornerData.VertsId.Z;
		VertsList[2] = CornerData.VertsId.X;
		LinkState[0] = CornerData.LinkState[1];
		LinkState[1] = CornerData.LinkState[2];
		LinkState[2] = CornerData.LinkState[0];
	}
	else if (LowestZ == Cell3.Elevation)
	{
		CellsList[0] = &Cell3; 
		CellsList[1] = &Cell1; 
		CellsList[2] = &Cell2;
		VertsList[0] = CornerData.VertsId.Z;
		VertsList[1] = CornerData.VertsId.X;
		VertsList[2] = CornerData.VertsId.Y;
		LinkState[0] = CornerData.LinkState[2];
		LinkState[1] = CornerData.LinkState[0];
		LinkState[2] = CornerData.LinkState[1];
	}
	else
	{
		CellsList[0] = &Cell1; 
		CellsList[1] = &Cell2; 
		CellsList[2] = &Cell3;
		VertsList[0] = CornerData.VertsId.X;
		VertsList[1] = CornerData.VertsId.Y;
		VertsList[2] = CornerData.VertsId.Z;
		LinkState[0] = CornerData.LinkState[0];
		LinkState[1] = CornerData.LinkState[1];
		LinkState[2] = CornerData.LinkState[2];
	}

	FVector Vert0 = CellsList[0]->CellCenter + FHexCellData::HexVertices[VertsList[0]];
	FVector Vert1 = CellsList[1]->CellCenter + FHexCellData::HexVertices[VertsList[1]];
	FVector Vert2 = CellsList[2]->CellCenter + FHexCellData::HexVertices[VertsList[2]];
	
	FVector DisturbedVert0 = PerturbingVertex(Vert0, CellsList[0]->Elevation);
	FVector DisturbedVert1 = PerturbingVertex(Vert1, CellsList[1]->Elevation);
	FVector DisturbedVert2 = PerturbingVertex(Vert2, CellsList[2]->Elevation);

	auto CalcTerraceVerts = [this](TArray<TArray<FVector>>& OutVerts, TArray<TArray<FColor>>& OutColors,
		const FVector& ToVert, const FVector& FromVert, const FColor& ToColor, const FColor& FromColor,
		int32 ToElevation, int32 FromElevation)
		{
			int32 NumOfZSteps = ToElevation - FromElevation;
			int32 NumOfSteps = NumOfZSteps * 2 - 1;

			int32 BaseIndex = OutVerts.Num();
			OutVerts.AddDefaulted(NumOfZSteps);
			OutColors.AddDefaulted(NumOfZSteps);
			
			for (int32 StepIndex = 1; StepIndex <= NumOfSteps; ++StepIndex)
			{
				float RatioXY = float(StepIndex) / float(NumOfSteps);
				int32 StepZIndex = (StepIndex - 1) / 2 + 1;
				float RatioZ = float(StepZIndex) / float(NumOfZSteps);

				FVector CurStepVert;
				CurStepVert.X = FMath::Lerp(FromVert.X, ToVert.X, RatioXY);
				CurStepVert.Y = FMath::Lerp(FromVert.Y, ToVert.Y, RatioXY);
				CurStepVert.Z = FMath::Lerp(FromVert.Z, ToVert.Z, RatioZ);

				PerturbingVertexInline(CurStepVert, FromElevation + StepZIndex);

				FColor CurStepColor;
				CurStepColor.R = FMath::Lerp(FromColor.R, ToColor.R, RatioZ);
				CurStepColor.G = FMath::Lerp(FromColor.G, ToColor.G, RatioZ);
				CurStepColor.B = FMath::Lerp(FromColor.B, ToColor.B, RatioZ);
				CurStepColor.A = FMath::Lerp(FromColor.A, ToColor.A, RatioZ);

				OutVerts[BaseIndex + StepZIndex - 1].Add(CurStepVert);
				OutColors[BaseIndex + StepZIndex - 1].Add(CurStepColor);
			}
		};

	auto CalcLinearVerts = [](TArray<TArray<FVector>>& OutVerts, TArray<TArray<FColor>>& OutColors,
		const FVector& ToVert, const FVector& FromVert, const FColor& ToColor, const FColor& FromColor,
		int32 ToElevation, int32 FromElevation)
		{
			int32 NumOfSteps = ToElevation - FromElevation;
			if (NumOfSteps == 0)
			{
				OutVerts.Last()[0] = ToVert;
				OutColors.Last()[0] = ToColor;
			}
			else
			{
				int32 BaseIndex = OutVerts.Num();
				OutVerts.AddDefaulted(NumOfSteps);
				OutColors.AddDefaulted(NumOfSteps);

				for (int32 StepIndex = 1; StepIndex <= NumOfSteps; ++StepIndex)
				{
					float Ratio = float(StepIndex) / float(NumOfSteps);

					FVector CurStepVert = FMath::Lerp(FromVert, ToVert, Ratio);
					FColor CurStepColor;
					CurStepColor.R = FMath::Lerp(FromColor.R, ToColor.R, Ratio);
					CurStepColor.G = FMath::Lerp(FromColor.G, ToColor.G, Ratio);
					CurStepColor.B = FMath::Lerp(FromColor.B, ToColor.B, Ratio);
					CurStepColor.A = FMath::Lerp(FromColor.A, ToColor.A, Ratio);
					
					OutVerts[BaseIndex + StepIndex - 1].Add(CurStepVert);
					OutColors[BaseIndex + StepIndex - 1].Add(CurStepColor);
				}
			}
		};

	TArray<TArray<FVector>> Verts01;
	TArray<TArray<FVector>> Verts02;
	TArray<TArray<FColor>> Colors01;
	TArray<TArray<FColor>> Colors02;

	Verts01.AddDefaulted();
	Verts01[0].Add(DisturbedVert0);
	Verts02.AddDefaulted();
	Verts02[0].Add(DisturbedVert0);
	Colors01.AddDefaulted();
	Colors01[0].Add(CellsList[0]->SRGBColor);
	Colors02.AddDefaulted();
	Colors02[0].Add(CellsList[0]->SRGBColor);

	if (LinkState[0] == EHexLinkState::Terrace)
	{
		CalcTerraceVerts(Verts01, Colors01, Vert1, Vert0, CellsList[1]->SRGBColor, CellsList[0]->SRGBColor, CellsList[1]->Elevation, CellsList[0]->Elevation);
	}
	else
	{
		CalcLinearVerts(Verts01, Colors01, DisturbedVert1, DisturbedVert0, CellsList[1]->SRGBColor, CellsList[0]->SRGBColor, CellsList[1]->Elevation, CellsList[0]->Elevation);
	}

	if (LinkState[2] == EHexLinkState::Terrace)
	{
		CalcTerraceVerts(Verts02, Colors02, Vert2, Vert0, CellsList[2]->SRGBColor, CellsList[0]->SRGBColor, CellsList[2]->Elevation, CellsList[0]->Elevation);
	}
	else
	{
		CalcLinearVerts(Verts02, Colors02, DisturbedVert2, DisturbedVert0, CellsList[2]->SRGBColor, CellsList[0]->SRGBColor, CellsList[2]->Elevation, CellsList[0]->Elevation);
	}
	
	if (LinkState[1] == EHexLinkState::Terrace)
	{
		if (CellsList[1]->Elevation < CellsList[2]->Elevation) // 1 -> 2
		{
			CalcTerraceVerts(Verts01, Colors01, Vert2, Vert1, CellsList[2]->SRGBColor, CellsList[1]->SRGBColor, CellsList[2]->Elevation, CellsList[1]->Elevation);
		}
		else // 2 -> 1
		{
			CalcTerraceVerts(Verts02, Colors02, Vert1, Vert2, CellsList[1]->SRGBColor, CellsList[2]->SRGBColor, CellsList[1]->Elevation, CellsList[2]->Elevation);
		}
	}
	else
	{
		if (CellsList[1]->Elevation < CellsList[2]->Elevation) // 1 -> 2
		{
			CalcLinearVerts(Verts01, Colors01, DisturbedVert2, DisturbedVert1, CellsList[2]->SRGBColor, CellsList[1]->SRGBColor, CellsList[2]->Elevation, CellsList[1]->Elevation);
		}
		else if (CellsList[1]->Elevation > CellsList[2]->Elevation)// 2 -> 1
		{
			CalcLinearVerts(Verts02, Colors02, DisturbedVert1, DisturbedVert2, CellsList[1]->SRGBColor, CellsList[2]->SRGBColor, CellsList[1]->Elevation, CellsList[2]->Elevation);
		}
	}
	
	int32 NumOfLayers01 = Verts01.Num();
	int32 NumOfLayers02 = Verts02.Num();
	check(NumOfLayers01 == NumOfLayers02);

	for (int32 Index = 1; Index < NumOfLayers01; ++Index)
	{
		// Cross Elevation
		FillQuad(Verts02[Index - 1].Last(), Verts01[Index - 1].Last(), Verts02[Index][0], Verts01[Index][0],
			Colors02[Index - 1].Last(), Colors01[Index - 1].Last(), Colors02[Index][0], Colors01[Index][0], OutCellMesh);

		// Current Elevation
		FillQuad(Verts02[Index][0], Verts01[Index][0], Verts02[Index].Last(), Verts01[Index].Last(),
			Colors02[Index][0], Colors01[Index][0], Colors02[Index].Last(), Colors01[Index].Last(), OutCellMesh);
	}
}

FVector AHexTerrainGenerator::CalcHexCellCenter(const FIntPoint& GridId, int32 Elevation)
{
	static FVector2D VertOffsetScale{ 1.732050807568877, 1.5 };
	float CellOuterRadius = HexCellRadius + HexCellBorderWidth;

	FVector VertOffset;
	VertOffset.X = (GridId.X + (GridId.Y % 2) * 0.5) * CellOuterRadius * VertOffsetScale.X;
	VertOffset.Y = GridId.Y * CellOuterRadius * VertOffsetScale.Y;
	VertOffset.Z = Elevation * HexElevationStep;
	
	return VertOffset;
}

FVector AHexTerrainGenerator::CalcFaceNormal(const FVector& V0, const FVector& V1, const FVector& V2)
{
	FVector Edge1 = (V1 - V0);
	FVector Edge2 = (V2 - V0);
	FVector NormalVector = FVector::CrossProduct(Edge1, Edge2);
	return NormalVector.GetSafeNormal();
}

void AHexTerrainGenerator::FillQuad(const FVector& FromV0, const FVector& FromV1, const FVector& ToV0, const FVector& ToV1,
	const FColor& FromC0, const FColor& FromC1, const FColor& ToC0, const FColor& ToC1, FCachedSectionData& OutCellMesh)
{
	int32 BaseIndex = OutCellMesh.Vertices.Num();
	if ((FromV0 - FromV1).IsNearlyZero())
	{
		OutCellMesh.Vertices.Add(ToV0);
		OutCellMesh.Vertices.Add(ToV1);
		OutCellMesh.Vertices.Add(FromV0);

		OutCellMesh.VertexColors.Add(ToC0);
		OutCellMesh.VertexColors.Add(ToC1);
		OutCellMesh.VertexColors.Add(FromC0);

		FVector FaceNormal = CalcFaceNormal(ToV0, ToV1, FromV0);
		OutCellMesh.Normals.Add(FaceNormal);
		OutCellMesh.Normals.Add(FaceNormal);
		OutCellMesh.Normals.Add(FaceNormal);

		OutCellMesh.Triangles.Add(BaseIndex + 2);
		OutCellMesh.Triangles.Add(BaseIndex + 1);
		OutCellMesh.Triangles.Add(BaseIndex);
	}
	else if ((ToV0 - ToV1).IsNearlyZero())
	{
		OutCellMesh.Vertices.Add(ToV0);
		OutCellMesh.Vertices.Add(FromV0);
		OutCellMesh.Vertices.Add(FromV1);

		OutCellMesh.VertexColors.Add(ToC0);
		OutCellMesh.VertexColors.Add(FromC0);
		OutCellMesh.VertexColors.Add(FromC1);

		FVector FaceNormal = CalcFaceNormal(ToV0, FromV1, FromV0);
		OutCellMesh.Normals.Add(FaceNormal);
		OutCellMesh.Normals.Add(FaceNormal);
		OutCellMesh.Normals.Add(FaceNormal);

		OutCellMesh.Triangles.Add(BaseIndex + 1);
		OutCellMesh.Triangles.Add(BaseIndex + 2);
		OutCellMesh.Triangles.Add(BaseIndex);
	}
	else if ((ToV0 - FromV0).IsNearlyZero())
	{
		OutCellMesh.Vertices.Add(ToV0);
		OutCellMesh.Vertices.Add(ToV1);
		OutCellMesh.Vertices.Add(FromV1);

		OutCellMesh.VertexColors.Add(ToC0);
		OutCellMesh.VertexColors.Add(ToC1);
		OutCellMesh.VertexColors.Add(FromC1);

		FVector FaceNormal = CalcFaceNormal(ToV0, ToV1, FromV1);
		OutCellMesh.Normals.Add(FaceNormal);
		OutCellMesh.Normals.Add(FaceNormal);
		OutCellMesh.Normals.Add(FaceNormal);

		OutCellMesh.Triangles.Add(BaseIndex + 2);
		OutCellMesh.Triangles.Add(BaseIndex + 1);
		OutCellMesh.Triangles.Add(BaseIndex);
	}
	else if ((ToV1 - FromV1).IsNearlyZero())
	{
		OutCellMesh.Vertices.Add(ToV0);
		OutCellMesh.Vertices.Add(ToV1);
		OutCellMesh.Vertices.Add(FromV0);

		OutCellMesh.VertexColors.Add(ToC0);
		OutCellMesh.VertexColors.Add(ToC1);
		OutCellMesh.VertexColors.Add(FromC0);

		FVector FaceNormal = CalcFaceNormal(ToV0, ToV1, FromV0);
		OutCellMesh.Normals.Add(FaceNormal);
		OutCellMesh.Normals.Add(FaceNormal);
		OutCellMesh.Normals.Add(FaceNormal);

		OutCellMesh.Triangles.Add(BaseIndex + 2);
		OutCellMesh.Triangles.Add(BaseIndex + 1);
		OutCellMesh.Triangles.Add(BaseIndex);
	}
	else
	{
		OutCellMesh.Vertices.Add(ToV0);
		OutCellMesh.Vertices.Add(ToV1);
		OutCellMesh.Vertices.Add(FromV0);
		OutCellMesh.Vertices.Add(FromV1);

		OutCellMesh.VertexColors.Add(ToC0);
		OutCellMesh.VertexColors.Add(ToC1);
		OutCellMesh.VertexColors.Add(FromC0);
		OutCellMesh.VertexColors.Add(FromC1);

		FVector FaceNormal = CalcFaceNormal(ToV0, ToV1, FromV0);
		OutCellMesh.Normals.Add(FaceNormal);
		OutCellMesh.Normals.Add(FaceNormal);
		OutCellMesh.Normals.Add(FaceNormal);
		OutCellMesh.Normals.Add(FaceNormal);

		OutCellMesh.Triangles.Add(BaseIndex + 2);
		OutCellMesh.Triangles.Add(BaseIndex + 1);
		OutCellMesh.Triangles.Add(BaseIndex);

		OutCellMesh.Triangles.Add(BaseIndex + 2);
		OutCellMesh.Triangles.Add(BaseIndex + 3);
		OutCellMesh.Triangles.Add(BaseIndex + 1);
	}
};

void AHexTerrainGenerator::PerturbingVertexInline(FVector& Vertex, int32 Elevation)
{
	if (NoiseTexture.IsEmpty())
		return;

	/*bool bFound = false;
	FHexVertexAttributeData& VertAttribute = CacehdVertexData.FindOrAddVertex(Vertex, bFound);
	if (!bFound)
	{
		FLinearColor NoiseVector = SampleTextureBilinear(NoiseTexture, Vertex);
		VertAttribute.NoiseVector.X = NoiseVector.R * 2.0f - 1.0f;
		VertAttribute.NoiseVector.Y = NoiseVector.G * 2.0f - 1.0f;
		VertAttribute.NoiseVector.Z = NoiseVector.B * 2.0f - 1.0f;
	}
	
	Vertex.X += VertAttribute.NoiseVector.X * PerturbingStrength;
	Vertex.Y += VertAttribute.NoiseVector.Y * PerturbingStrength;*/

	FLinearColor NoiseVector = SampleTextureBilinear(NoiseTexture, Vertex * PerturbingScalingHV.X);
	Vertex.X += (NoiseVector.R * 2.0f - 1.0f) * PerturbingStrengthHV.X;
	Vertex.Y += (NoiseVector.G * 2.0f - 1.0f) * PerturbingStrengthHV.X;

	if (!CachedNoiseZ.Contains(Elevation))
	{
		FLinearColor NoiseVectorZ = SampleTextureBilinear(NoiseTexture, FMath::RoundToInt(Elevation * PerturbingScalingHV.Y), 0);
		CachedNoiseZ.Add(Elevation, NoiseVectorZ.B * 2.0f - 1.0f);
	}

	Vertex.Z += CachedNoiseZ[Elevation] * PerturbingStrengthHV.Y;
}

FVector AHexTerrainGenerator::PerturbingVertex(const FVector& Vertex, int32 Elevation)
{
	FVector NewVec = Vertex;
	PerturbingVertexInline(NewVec, Elevation);
	return NewVec;
}

FLinearColor AHexTerrainGenerator::SampleTextureBilinear(const TArray<TArray<FColor>>& InTexture, const FVector& SamplePos)
{
	int32 SizeY = InTexture.Num();
	int32 SizeX = InTexture[0].Num();

	int32 SampleX = FMath::FloorToInt(SamplePos.X);
	int32 SampleY = FMath::FloorToInt(SamplePos.Y);
	float RatioX = SamplePos.X - SampleX;
	float RatioY = SamplePos.Y - SampleY;

	if (SampleX < 0)
		SampleX += SizeX * (1 - SampleX / SizeX);
	if (SampleY < 0)
		SampleY += SizeY * (1 - SampleY / SizeY);

	SampleX = SampleX % SizeX;
	SampleY = SampleY % SizeY;
	int32 NextSampleX = (SampleX + 1) % SizeX;
	int32 NextSampleY = (SampleY + 1) % SizeY;

	const FColor& LTColor = InTexture[SampleY][SampleX];
	const FColor& RTColor = InTexture[SampleY][NextSampleX];
	const FColor& LDColor = InTexture[NextSampleY][SampleX];
	const FColor& RDColor = InTexture[NextSampleY][NextSampleX];

	FLinearColor TColor = FMath::Lerp(FLinearColor::FromSRGBColor(LTColor), FLinearColor::FromSRGBColor(RTColor), RatioX);
	FLinearColor DColor = FMath::Lerp(FLinearColor::FromSRGBColor(LDColor), FLinearColor::FromSRGBColor(RDColor), RatioX);

	return FMath::Lerp(TColor, DColor, RatioY);
}

FLinearColor AHexTerrainGenerator::SampleTextureBilinear(const TArray<TArray<FColor>>& InTexture, int32 SampleX, int32 SampleY)
{
	int32 SizeY = InTexture.Num();
	int32 SizeX = InTexture[0].Num();

	if (SampleX < 0)
		SampleX += SizeX * (1 - SampleX / SizeX);
	if (SampleY < 0)
		SampleY += SizeY * (1 - SampleY / SizeY);

	SampleX = SampleX % SizeX;
	SampleY = SampleY % SizeY;
	
	return FLinearColor::FromSRGBColor(InTexture[SampleY][SampleX]);
}

void AHexTerrainGenerator::CreateTextureFromData(TArray<TArray<FColor>> &OutTexture, const TArray<uint8>& InBineryData, EImageFormat InFormat)
{
	if (InBineryData.IsEmpty())
	{
		return;
	}

	static IImageWrapperModule* ImageWrapperModule = FModuleManager::LoadModulePtr<IImageWrapperModule>("ImageWrapper");
	if (ImageWrapperModule == nullptr)
	{
		return;
	}
	
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule->CreateImageWrapper(InFormat);
	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(InBineryData.GetData(), InBineryData.Num()))
	{
		return;
	}

	TArray<uint8> Uncompressed;
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, Uncompressed))
	{
		return;
	}

	OutTexture.Empty(ImageWrapper->GetHeight());
	OutTexture.AddDefaulted(ImageWrapper->GetHeight());
	for (int32 Y = 0; Y < ImageWrapper->GetHeight(); ++Y)
	{
		OutTexture[Y].AddUninitialized(ImageWrapper->GetWidth());
		for (int32 X = 0; X < ImageWrapper->GetWidth(); ++X)
		{
			int32 Index = X + Y * ImageWrapper->GetWidth();
			
			OutTexture[Y][X].B = Uncompressed[Index * 4];
			OutTexture[Y][X].G = Uncompressed[Index * 4 + 1];
			OutTexture[Y][X].R = Uncompressed[Index * 4 + 2];
			OutTexture[Y][X].A = Uncompressed[Index * 4 + 3];
		}
	}
}

void AHexTerrainGenerator::OnClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	if (!ConfigData.bConfigValid)
		return;

	if (PlayerController.IsNull())
		PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController.IsNull())
		return;

	FHitResult HitResult;
	if (PlayerController->GetHitResultUnderCursor(ECC_Visibility, true, HitResult))
	{
		static FVector2D VertOffsetScale{ 1.732050807568877, 1.5 };
		float CellOuterRadius = HexCellRadius + HexCellBorderWidth;

		FIntPoint GridId;
		GridId.Y = FMath::RoundToInt(HitResult.Location.Y / (CellOuterRadius * VertOffsetScale.Y));
		GridId.X = FMath::RoundToInt(HitResult.Location.X / (CellOuterRadius * VertOffsetScale.X) - (GridId.Y % 2) * 0.5);

		//UE_LOG(LogTemp, Display, TEXT("HitPos:%s %s"), *HitResult.Location.ToString(), *GridId.ToString());

		HexEditGridId.X = GridId.X;
		HexEditGridId.Y = GridId.Y;

		int32 GridIndex = GridId.Y * HexChunkSize.X * HexChunkCount.X + GridId.X;
		HexEditElevation = ConfigData.ElevationsList[GridIndex];
		HexEditTerrainType = ConfigData.TerrainTypesList[GridIndex];
	}
	else
	{
		HexEditGridId = FIntPoint(-1, -1);
		HexEditElevation = 0;
		HexEditTerrainType = EHexTerrainType::None;
	}
}

void AHexTerrainGenerator::OnReleased(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	//UE_LOG(LogTemp, Display, TEXT("OnRelease!!"));
}

void AHexTerrainGenerator::PostLoad()
{
	Super::PostLoad();

	LoadTerrain();
}

#if WITH_EDITOR

void AHexTerrainGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FProperty* MemberPropertyThatChanged = PropertyChangedEvent.MemberProperty;
	const FName MemberPropertyName = MemberPropertyThatChanged != NULL ? MemberPropertyThatChanged->GetFName() : NAME_None;

	static FName Name_HexEditElevation = GET_MEMBER_NAME_CHECKED(AHexTerrainGenerator, HexEditElevation);
	static FName Name_HexEditTerrainType = GET_MEMBER_NAME_CHECKED(AHexTerrainGenerator, HexEditTerrainType);
	if (MemberPropertyName == Name_HexEditElevation || MemberPropertyName == Name_HexEditTerrainType)
	{
		if (HexEditGridId.X >= 0 && HexEditGridId.Y >= 0)
		{
			int32 GridIndex = HexEditGridId.Y * HexChunkSize.X * HexChunkCount.X + HexEditGridId.X;
			ConfigData.ElevationsList[GridIndex] = HexEditElevation;
			ConfigData.TerrainTypesList[GridIndex] = HexEditTerrainType;

			GenerateTerrain();
		}
	}
}

#endif

#pragma optimize("", on)