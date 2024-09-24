#include "HexTerrainGenerator.h"

//////////////////////////////////////////////////////////////////////////

enum class EHexDirection : uint8
{
	E, SE, SW, W, NW, NE
};

struct FHexCellData
{
	static int32 RowSize;

	int32 GridId;
	FIntPoint GridIndex;
	FIntVector GridCoord;

	int32 HexNeighbors[6];

	FHexCellData(const FIntPoint& InIndex)
		: GridId(InIndex.X + InIndex.Y * RowSize)
		, GridIndex(InIndex)
		, GridCoord(CalcGridCoordinate(InIndex))
	{
		for (int32 Index = 0; Index < 6; ++Index)
			HexNeighbors[Index] = -1;
	}

	void LinkCell(FHexCellData& OtherCell, EHexDirection LinkDirection)
	{
		uint8 LinkId = static_cast<uint8>(LinkDirection);
		HexNeighbors[LinkId] = OtherCell.GridId;

		uint8 OtherLinkId = static_cast<uint8>(CalcOppositeDirection(LinkDirection));
		OtherCell.HexNeighbors[OtherLinkId] = GridId;
	}

	static FIntVector CalcGridCoordinate(const FIntPoint& InGridIndex)
	{
		int32 CoordX = InGridIndex.X - InGridIndex.Y / 2;
		int32 CoordZ = InGridIndex.Y;
		return FIntVector{ CoordX, -CoordX - CoordZ, CoordZ };
	}

	static EHexDirection CalcOppositeDirection(EHexDirection InDirection)
	{
		uint8 DirNum = static_cast<uint8>(InDirection);
		return static_cast<EHexDirection>((DirNum + 3u) % 6u);
	}

	static int32 CalcGridIndexByCoord(const FIntVector& InGridCoord)
	{
		int32 IndexY = InGridCoord.Z;
		int32 IndexX = InGridCoord.X + IndexY / 2;

		if (IndexX >= 0 && IndexX < RowSize && IndexY >= 0)
			return IndexX + IndexY * RowSize;
		else
			return -1;
	}
};

int32 FHexCellData::RowSize = 0;

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
	TArray<FHexCellData> HexGrid;
	HexGrid.Reserve(HexGridSize.X * HexGridSize.Y);

	// Create HexCellData
	for (int32 Y = 0; Y < HexGridSize.Y; ++Y)
	{
		for (int32 X = 0; X < HexGridSize.X; ++X)
		{
			FIntPoint GridIndex{ X, Y };

			FHexCellData OneCell{ GridIndex };

			int32 WIndex  = FHexCellData::CalcGridIndexByCoord(FIntVector{ OneCell.GridCoord.X - 1, OneCell.GridCoord.Y + 1, OneCell.GridCoord.Z });
			int32 NWIndex = FHexCellData::CalcGridIndexByCoord(FIntVector{ OneCell.GridCoord.X, OneCell.GridCoord.Y + 1, OneCell.GridCoord.Z - 1 });
			int32 NEIndex = FHexCellData::CalcGridIndexByCoord(FIntVector{ OneCell.GridCoord.X + 1, OneCell.GridCoord.Y, OneCell.GridCoord.Z - 1 });
			if (WIndex >= 0)
				HexGrid[WIndex].LinkCell(OneCell, EHexDirection::E);
			if (NWIndex >= 0)
				HexGrid[NWIndex].LinkCell(OneCell, EHexDirection::SE);
			if (NEIndex >= 0)
				HexGrid[NEIndex].LinkCell(OneCell, EHexDirection::SW);

			HexGrid.Add(OneCell);
		}
	}

	// Create HexCellMesh
	for (int32 Y = 0; Y < HexGridSize.Y; ++Y)
	{
		for (int32 X = 0; X < HexGridSize.X; ++X)
		{
			FIntPoint GridIndex{ X, Y };

			TArray<FVector> CellVertices;
			TArray<int32> CellIndices;
			GenerateHexCell(GridIndex, CellVertices, CellIndices);

			int32 BaseIndex = MeshSection.Vertices.Num();
			MeshSection.Vertices.Append(CellVertices);
			
			int32 NumOfIndices = CellVertices.Num();
			MeshSection.Triangles.Reserve(MeshSection.Triangles.Num() + NumOfIndices);
			for (int32 Index : CellIndices)
				MeshSection.Triangles.Add(BaseIndex + Index);
			
			FColor CellColor = FColor::MakeRandomColor();
			int32 NumOfVertices = CellVertices.Num();
			MeshSection.VertexColors.Reserve(MeshSection.VertexColors.Num() + NumOfIndices);
			for (int32 Index = 0; Index < NumOfVertices; ++Index)
				MeshSection.VertexColors.Add(CellColor);
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

			const FIntVector& GridCoord = HexGrid[GridId].GridCoord;

			FTransform Instance{ (V0 + V3) * 0.5 };
			CoordTextComponent->AddInstance(Instance, false);
			CoordTextComponent->SetCustomDataValue(GridId, 0, GridCoord.X);
			CoordTextComponent->SetCustomDataValue(GridId, 1, GridCoord.Z);
		}
	}
	CoordTextComponent->MarkRenderStateDirty();
}


void AHexTerrainGenerator::GenerateHexCell(const FIntPoint& GridIndex, TArray<FVector>& OutVertices, TArray<int32>& OutIndices)
{
#define	CORNER_NUM 6
	static double AngleStep = UE_DOUBLE_PI / 3.0;
	static double AngleStart = UE_DOUBLE_PI / 6.0;
	double Angle = AngleStart;

	static FVector2D VertOffsetScale{ 1.732050807568877, 1.5 };

	float CellRadiusAndBorder = HexCellRadius + HexCellBorderWidth;

	FVector2D VertOffset;
	VertOffset.X = (GridIndex.X + (GridIndex.Y % 2) * 0.5) * CellRadiusAndBorder * VertOffsetScale.X;
	VertOffset.Y = GridIndex.Y * CellRadiusAndBorder * VertOffsetScale.Y;

	for (int32 Index = 0; Index < CORNER_NUM; ++Index)
	{
		FVector Vert;
		Vert.X = VertOffset.X + HexCellRadius * FMath::Cos(Angle);
		Vert.Y = VertOffset.Y + HexCellRadius * FMath::Sin(Angle);
		Vert.Z = 0.0;

		OutVertices.Add(Vert);

		Angle += AngleStep;
	}

	for (int32 Index = 1; Index < CORNER_NUM - 1; ++Index)
	{
		OutIndices.Add(0);
		OutIndices.Add(Index + 1);
		OutIndices.Add(Index);
	}
}


