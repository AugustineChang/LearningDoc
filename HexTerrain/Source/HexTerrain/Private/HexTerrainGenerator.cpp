#include "HexTerrainGenerator.h"

template void AHexTerrainGenerator::ArrayAddSelfItem(TArray<FVector>& InArray, int32 CopiedIndex);
template void AHexTerrainGenerator::ArrayAddSelfItem(TArray<FColor>& InArray, int32 CopiedIndex);

//////////////////////////////////////////////////////////////////////////
int32 FHexCellData::RowSize = 0;
double FHexCellData::ElevationStep = 5.0;
TArray<FVector> FHexCellData::HexVertices;

FHexCellData::FHexCellData(const FIntPoint& InIndex)
	: GridId(InIndex.X + InIndex.Y * RowSize)
	, GridIndex(InIndex)
	, GridCoord(CalcGridCoordinate(InIndex))
{
	for (int32 Index = 0; Index < 6; ++Index)
		HexNeighbors[Index] = -1;
}

void FHexCellData::LinkCell(FHexCellData& OtherCell, EHexDirection LinkDirection)
{
	uint8 LinkId = static_cast<uint8>(LinkDirection);
	HexNeighbors[LinkId] = OtherCell.GridId;

	uint8 OtherLinkId = static_cast<uint8>(CalcOppositeDirection(LinkDirection));
	OtherCell.HexNeighbors[OtherLinkId] = GridId;
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

	if (IndexX >= 0 && IndexX < RowSize && IndexY >= 0)
		return IndexX + IndexY * RowSize;
	else
		return -1;
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
};

//////////////////////////////////////////////////////////////////////////

// Sets default values
AHexTerrainGenerator::AHexTerrainGenerator()
	: HexCellRadius(100.0f)
	, HexCellBorderWidth(10.0f)
	, HexElevationStep(5.0f)
	, HexGridSize(5, 5)
{
 	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));

	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMeshComponent"));
	ProceduralMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProceduralMeshComponent->Mobility = EComponentMobility::Movable;
	ProceduralMeshComponent->SetGenerateOverlapEvents(false);
	ProceduralMeshComponent->SetupAttachment(RootComponent);

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

void AHexTerrainGenerator::GenerateTerrain()
{
	FCachedSectionData MeshSection;

	FHexCellData::RowSize = HexGridSize.X;
	FHexCellData::ElevationStep = HexElevationStep;
	FHexCellData::HexVertices.Empty(6);

	HexGrids.Empty(HexGridSize.X * HexGridSize.Y);

	// Create HexCellData
	for (int32 Y = 0; Y < HexGridSize.Y; ++Y)
	{
		for (int32 X = 0; X < HexGridSize.X; ++X)
		{
			FIntPoint GridIndex{ X, Y };

			FHexCellData OneCell{ GridIndex };
			OneCell.CellColor = FColor::MakeRandomColor();
			OneCell.Elevation = (3 - FMath::Abs(X + Y - 3)) * 4;

			int32 WIndex  = FHexCellData::CalcGridIndexByCoord(FIntVector{ OneCell.GridCoord.X - 1, OneCell.GridCoord.Y + 1, OneCell.GridCoord.Z });
			int32 NWIndex = FHexCellData::CalcGridIndexByCoord(FIntVector{ OneCell.GridCoord.X, OneCell.GridCoord.Y + 1, OneCell.GridCoord.Z - 1 });
			int32 NEIndex = FHexCellData::CalcGridIndexByCoord(FIntVector{ OneCell.GridCoord.X + 1, OneCell.GridCoord.Y, OneCell.GridCoord.Z - 1 });
			if (WIndex >= 0)
				HexGrids[WIndex].LinkCell(OneCell, EHexDirection::E);
			if (NWIndex >= 0)
				HexGrids[NWIndex].LinkCell(OneCell, EHexDirection::SE);
			if (NEIndex >= 0)
				HexGrids[NEIndex].LinkCell(OneCell, EHexDirection::SW);

			HexGrids.Add(OneCell);
		}
	}

	// Create HexCellMesh
	for (int32 Y = 0; Y < HexGridSize.Y; ++Y)
	{
		for (int32 X = 0; X < HexGridSize.X; ++X)
		{
			int32 GridId = Y * HexGridSize.X + X;
			const FHexCellData& CellData = HexGrids[GridId];

			TArray<FVector> CellVertices;
			TArray<int32> CellIndices;
			TArray<FVector> CellNormals;
			TArray<FColor> CellColors;
			GenerateHexCell(CellData, CellVertices, CellIndices, CellNormals, CellColors);

			int32 BaseIndex = MeshSection.Vertices.Num();
			MeshSection.Vertices.Append(CellVertices);
			MeshSection.Normals.Append(CellNormals);
			MeshSection.VertexColors.Append(CellColors);
			
			int32 NumOfIndices = CellVertices.Num();
			MeshSection.Triangles.Reserve(MeshSection.Triangles.Num() + NumOfIndices);
			for (int32 Index : CellIndices)
				MeshSection.Triangles.Add(BaseIndex + Index);
		}
	}
	
	// Create Mesh
	ProceduralMeshComponent->ClearAllMeshSections();
	ProceduralMeshComponent->CreateMeshSection(0, MeshSection.Vertices, MeshSection.Triangles, MeshSection.Normals, 
		MeshSection.UV0s, MeshSection.VertexColors, MeshSection.Tangents, false);
	
	// Set Material
	if (!!HexTerrainMaterial)
	{
		ProceduralMeshComponent->SetMaterial(0, HexTerrainMaterial);
	}

	// Grid Coordinates
	if (!!TextMaterial)
	{
		CoordTextComponent->SetMaterial(0, TextMaterial);
	}
	CoordTextComponent->ClearInstances();
	CoordTextComponent->SetNumCustomDataFloats(2);
	for (int32 Y = 0; Y < HexGridSize.Y; ++Y)
	{
		for (int32 X = 0; X < HexGridSize.X; ++X)
		{
			int32 GridId = Y * HexGridSize.X + X;
			
			const FHexCellData& CellData = HexGrids[GridId];
			FVector CellCenter = CalcHexCellCenter(CellData.GridIndex, CellData.Elevation);

			FTransform Instance{ CellCenter };
			CoordTextComponent->AddInstance(Instance, false);
			CoordTextComponent->SetCustomDataValue(GridId, 0, CellData.GridCoord.X);
			CoordTextComponent->SetCustomDataValue(GridId, 1, CellData.GridCoord.Z);
		}
	}
	CoordTextComponent->MarkRenderStateDirty();
}


void AHexTerrainGenerator::GenerateHexCell(const FHexCellData& InCellData, 
	TArray<FVector>& OutVertices, TArray<int32>& OutIndices, TArray<FVector>& OutNormals, TArray<FColor> &OutColors)
{
// Hex Vertices Index
//      4
//   /     \
// 3         5
// |         |
// 2         0
//   \     /
//      1

#define	CORNER_NUM 6
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
	}
	
	FVector CurCenter = CalcHexCellCenter(InCellData.GridIndex, InCellData.Elevation);

	OutVertices.Empty(CORNER_NUM * 2);
	OutIndices.Empty(CORNER_NUM * 2 * 3);
	OutNormals.Empty(CORNER_NUM * 2);
	OutColors.Empty(CORNER_NUM * 2);

	// Inner HexCell
	for (int32 Index = 0; Index < CORNER_NUM; ++Index)
	{
		OutVertices.Add(CurCenter + FHexCellData::HexVertices[Index]);
		OutNormals.Add(FVector::UpVector);
		OutColors.Add(InCellData.CellColor);
	}

	for (int32 Index = 1; Index < CORNER_NUM - 1; ++Index)
	{
		OutIndices.Add(0);
		OutIndices.Add(Index + 1);
		OutIndices.Add(Index);	
	}

	// Border
	auto AddOneBorder = [this, &OutVertices, &OutIndices, &OutNormals, &OutColors](int32 OtherGridId,
		int32 OtherVert0, int32 OtherVert1, int32 CurVert0, int32 CurVert1)
		{
			const FHexCellData& OtherCellData = HexGrids[OtherGridId];
			FVector OtherCenter = CalcHexCellCenter(OtherCellData.GridIndex, OtherCellData.Elevation);

			int32 BaseIndex = OutVertices.Num();
			OutVertices.Add(OtherCenter + FHexCellData::HexVertices[OtherVert0]);
			OutVertices.Add(OtherCenter + FHexCellData::HexVertices[OtherVert1]);
			ArrayAddSelfItem(OutVertices, CurVert0);
			ArrayAddSelfItem(OutVertices, CurVert1);
			
			OutColors.Add(OtherCellData.CellColor);
			OutColors.Add(OtherCellData.CellColor);
			ArrayAddSelfItem(OutColors, CurVert0);
			ArrayAddSelfItem(OutColors, CurVert1);
			
			FVector FaceNormal = CalcFaceNormal(OutVertices[BaseIndex], OutVertices[BaseIndex + 1], OutVertices[BaseIndex + 2]);
			OutNormals.Add(FaceNormal);
			OutNormals.Add(FaceNormal);
			OutNormals.Add(FaceNormal);
			OutNormals.Add(FaceNormal);

			OutIndices.Add(BaseIndex + 2);
			OutIndices.Add(BaseIndex + 1);
			OutIndices.Add(BaseIndex);
			
			OutIndices.Add(BaseIndex + 2);
			OutIndices.Add(BaseIndex + 3);
			OutIndices.Add(BaseIndex + 1);
		};

	int32 WIndex = InCellData.HexNeighbors[3];
	int32 NWIndex = InCellData.HexNeighbors[4];
	int32 NEIndex = InCellData.HexNeighbors[5];
	if (WIndex >= 0) // W Edge
	{
		AddOneBorder(WIndex, 0, 5, 2, 3);
	}

	if (NWIndex >= 0) // NW Edge
	{
		AddOneBorder(NWIndex, 1, 0, 3, 4);
	}

	if (NEIndex >= 0) // NE Edge
	{
		AddOneBorder(NEIndex, 2, 1, 4, 5);
	}
	
	if (WIndex >= 0 && NWIndex >= 0) // NW Corner
	{
		int32 BaseIndex = OutVertices.Num();
		ArrayAddSelfItem(OutVertices, 3);  // InnerHex: NW Corner
		ArrayAddSelfItem(OutVertices, 10); // NW Edge:  Vert0
		ArrayAddSelfItem(OutVertices, 7);  // W Edge:   Vert1

		ArrayAddSelfItem(OutColors, 3);
		ArrayAddSelfItem(OutColors, 10);
		ArrayAddSelfItem(OutColors, 7);

		FVector FaceNormal = CalcFaceNormal(OutVertices[BaseIndex], OutVertices[BaseIndex + 2], OutVertices[BaseIndex + 1]);
		OutNormals.Add(FaceNormal);
		OutNormals.Add(FaceNormal);
		OutNormals.Add(FaceNormal);

		OutIndices.Add(BaseIndex); 
		OutIndices.Add(BaseIndex + 1);
		OutIndices.Add(BaseIndex + 2);
	}

	if (NWIndex >= 0 && NEIndex >= 0) // N Corner
	{
		int32 BaseIndex = OutVertices.Num();
		ArrayAddSelfItem(OutVertices, 4); // InnerHex: N Corner
		ArrayAddSelfItem(OutVertices, WIndex >= 0 ? 14 : 10); // NE Edge:  Vert0
		ArrayAddSelfItem(OutVertices, WIndex >= 0 ? 11 : 7);  // NW Edge:   Vert1

		ArrayAddSelfItem(OutColors, 4);
		ArrayAddSelfItem(OutColors, WIndex >= 0 ? 14 : 10);
		ArrayAddSelfItem(OutColors, WIndex >= 0 ? 11 : 7);

		FVector FaceNormal = CalcFaceNormal(OutVertices[BaseIndex], OutVertices[BaseIndex + 2], OutVertices[BaseIndex + 1]);
		OutNormals.Add(FaceNormal);
		OutNormals.Add(FaceNormal);
		OutNormals.Add(FaceNormal);

		OutIndices.Add(BaseIndex);
		OutIndices.Add(BaseIndex + 1);
		OutIndices.Add(BaseIndex + 2);
	}

#undef CORNER_NUM
}

FVector AHexTerrainGenerator::CalcHexCellCenter(const FIntPoint& GridIndex, int32 Elevation)
{
	static FVector2D VertOffsetScale{ 1.732050807568877, 1.5 };
	float CellOuterRadius = HexCellRadius + HexCellBorderWidth;

	FVector VertOffset;
	VertOffset.X = (GridIndex.X + (GridIndex.Y % 2) * 0.5) * CellOuterRadius * VertOffsetScale.X;
	VertOffset.Y = GridIndex.Y * CellOuterRadius * VertOffsetScale.Y;
	VertOffset.Z = Elevation * FHexCellData::ElevationStep;
	
	return VertOffset;
}

FVector AHexTerrainGenerator::CalcFaceNormal(const FVector& V0, const FVector& V1, const FVector& V2)
{
	FVector Edge1 = (V1 - V0);
	FVector Edge2 = (V2 - V0);
	return FVector::CrossProduct(Edge1, Edge2);
}

template<typename T>
void AHexTerrainGenerator::ArrayAddSelfItem(TArray<T>& InArray, int32 CopiedIndex)
{
	T CopiedItem = InArray[CopiedIndex];
	InArray.Add(CopiedItem);
}