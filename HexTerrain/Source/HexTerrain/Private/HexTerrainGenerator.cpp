#include "HexTerrainGenerator.h"

//////////////////////////////////////////////////////////////////////////
int32 FHexCellData::RowSize = 0;
float FHexCellData::ElevationStep = 5.0f;
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
			TArray<FColor> CellColors;
			GenerateHexCell(CellData, CellVertices, CellIndices, CellColors);

			int32 BaseIndex = MeshSection.Vertices.Num();
			MeshSection.Vertices.Append(CellVertices);
			MeshSection.VertexColors.Append(CellColors);
			
			int32 NumOfIndices = CellVertices.Num();
			MeshSection.Triangles.Reserve(MeshSection.Triangles.Num() + NumOfIndices);
			for (int32 Index : CellIndices)
				MeshSection.Triangles.Add(BaseIndex + Index);
		}
	}

	MeshSection.Normals.Init(FVector::UpVector, MeshSection.Vertices.Num());
	
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
	CoordTextComponent->SetNumCustomDataFloats(2);
	for (int32 Y = 0; Y < HexGridSize.Y; ++Y)
	{
		for (int32 X = 0; X < HexGridSize.X; ++X)
		{
			int32 GridId = Y * HexGridSize.X + X;
			
			FVector& V0 = MeshSection.Vertices[GridId * 6];
			FVector& V3 = MeshSection.Vertices[GridId * 6 + 3];

			const FIntVector& GridCoord = HexGrids[GridId].GridCoord;

			FTransform Instance{ (V0 + V3) * 0.5 };
			CoordTextComponent->AddInstance(Instance, false);
			CoordTextComponent->SetCustomDataValue(GridId, 0, GridCoord.X);
			CoordTextComponent->SetCustomDataValue(GridId, 1, GridCoord.Z);
		}
	}
	CoordTextComponent->MarkRenderStateDirty();
}


void AHexTerrainGenerator::GenerateHexCell(const FHexCellData& InCellData, TArray<FVector>& OutVertices, TArray<int32>& OutIndices, TArray<FColor> &OutColors)
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
	
	FVector CurCenter = CalcHexCellCenter(InCellData.GridIndex);

	OutVertices.Empty(CORNER_NUM * 2);
	OutColors.Empty(CORNER_NUM * 2);
	OutIndices.Empty((CORNER_NUM - 2 + CORNER_NUM * 2) * 3);

	// Inner HexCell
	for (int32 Index = 0; Index < CORNER_NUM; ++Index)
	{
		OutVertices.Add(CurCenter + FHexCellData::HexVertices[Index]);
		OutColors.Add(InCellData.CellColor);
	}

	for (int32 Index = 1; Index < CORNER_NUM - 1; ++Index)
	{
		OutIndices.Add(0);
		OutIndices.Add(Index + 1);
		OutIndices.Add(Index);
	}

	// Border
	auto AddOneBorder = [this, &OutVertices, &OutColors, &OutIndices](int32 OtherGridId, 
		int32 OtherVert0, int32 OtherVert1, int32 CurVert0, int32 CurVert1)
		{
			const FHexCellData& WCellData = HexGrids[OtherGridId];
			FVector WCenter = CalcHexCellCenter(WCellData.GridIndex);

			int32 BaseIndex = OutVertices.Num();
			OutVertices.Add(WCenter + FHexCellData::HexVertices[OtherVert0]);
			OutVertices.Add(WCenter + FHexCellData::HexVertices[OtherVert1]);
			OutColors.Add(WCellData.CellColor);
			OutColors.Add(WCellData.CellColor);

			OutIndices.Add(CurVert0);
			OutIndices.Add(BaseIndex + 1);
			OutIndices.Add(BaseIndex);

			OutIndices.Add(CurVert0);
			OutIndices.Add(CurVert1);
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
		OutIndices.Add(3); // InnerHex: NW Corner
		OutIndices.Add(8); // NW Edge:  Vert0
		OutIndices.Add(7); // W Edge:   Vert1
	}

	if (NWIndex >= 0 && NEIndex >= 0) // N Corner
	{
		OutIndices.Add(4); // InnerHex: N Corner
		OutIndices.Add(WIndex >= 0 ? 10 : 8); // NE Edge:  Vert0
		OutIndices.Add(WIndex >= 0 ?  9 : 7); // NW Edge:  Vert1
	}

#undef CORNER_NUM
}

FVector AHexTerrainGenerator::CalcHexCellCenter(const FIntPoint& GridIndex)
{
	static FVector2D VertOffsetScale{ 1.732050807568877, 1.5 };
	float CellOuterRadius = HexCellRadius + HexCellBorderWidth;

	FVector VertOffset;
	VertOffset.X = (GridIndex.X + (GridIndex.Y % 2) * 0.5) * CellOuterRadius * VertOffsetScale.X;
	VertOffset.Y = GridIndex.Y * CellOuterRadius * VertOffsetScale.Y;
	VertOffset.Z = 0.0;
	
	return VertOffset;
}

