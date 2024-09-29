#include "HexTerrainGenerator.h"

#define	CORNER_NUM 6

//////////////////////////////////////////////////////////////////////////
int32 FHexCellData::RowSize = 0;
int32 FHexCellData::MaxTerranceElevation = 0;
TArray<FVector> FHexCellData::HexVertices;

FHexCellData::FHexCellData(const FIntPoint& InIndex)
	: GridId(InIndex.X + InIndex.Y * RowSize)
	, GridIndex(InIndex)
	, GridCoord(CalcGridCoordinate(InIndex))
{}

void FHexCellData::LinkCell(FHexCellData& OtherCell, EHexDirection LinkDirection)
{
	EHexLinkState LinkState = CalcLinkState(OtherCell, *this);

	uint8 LinkId = static_cast<uint8>(LinkDirection);
	uint8 OtherLinkId = static_cast<uint8>(CalcOppositeDirection(LinkDirection));

	FHexCellLink& Link1 = HexNeighbors[LinkId];
	Link1.LinkedCellId = OtherCell.GridId;
	Link1.LinkState = LinkState;

	Link1.FromVert.Y = LinkId;
	Link1.FromVert.X = (Link1.FromVert.Y + CORNER_NUM - 1) % CORNER_NUM;
	Link1.ToVert.X = OtherLinkId;
	Link1.ToVert.Y = (Link1.ToVert.X + CORNER_NUM - 1) % CORNER_NUM;
	
	FHexCellLink& Link2 = OtherCell.HexNeighbors[OtherLinkId];
	Link2.LinkedCellId = GridId;
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
	Corner.LinkedCellsId.X = GridId;
	Corner.LinkedCellsId.Y = Cell1.GridId;
	Corner.LinkedCellsId.Z = Cell2.GridId;

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

	if (IndexX >= 0 && IndexX < RowSize && IndexY >= 0)
		return IndexX + IndexY * RowSize;
	else
		return -1;
}

EHexLinkState FHexCellData::CalcLinkState(const FHexCellData& Cell1, const FHexCellData& Cell2)
{
	int32 ElevationDiff = FMath::Abs(Cell1.Elevation - Cell2.Elevation);
	EHexLinkState LinkState = EHexLinkState::Plane;
	if (ElevationDiff == 0)
		LinkState = EHexLinkState::Plane;
	else if (ElevationDiff == 1)
		LinkState = EHexLinkState::Slope;
	else if (ElevationDiff <= MaxTerranceElevation)
		LinkState = EHexLinkState::Terrace;
	else
		LinkState = EHexLinkState::Cliff;
	return LinkState;
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
	: HexGridSize(5, 5)
	, HexCellRadius(100.0f)
	, HexCellBorderWidth(10.0f)
	, HexElevationStep(5.0f)
	, MaxElevationForTerrace(4)
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
	FHexCellData::MaxTerranceElevation = MaxElevationForTerrace;
	FHexCellData::HexVertices.Empty(6);

	HexGrids.Empty(HexGridSize.X * HexGridSize.Y);

	// Create HexCellData
	for (int32 Y = 0; Y < HexGridSize.Y; ++Y)
	{
		for (int32 X = 0; X < HexGridSize.X; ++X)
		{
			FIntPoint GridIndex{ X, Y };

			FHexCellData OneCell{ GridIndex };
			//OneCell.LinearColor = FLinearColor::MakeRandomColor();
			//OneCell.SRGBColor = OneCell.LinearColor.ToFColorSRGB();
			OneCell.SRGBColor = FColor::MakeRandomColor();
			if (OneCell.GridId < DebugElevation.Num())
				OneCell.Elevation = DebugElevation[OneCell.GridId];
			else
				OneCell.Elevation = 0;//(3 - FMath::Abs(X + Y - 3)) * 4;
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
	for (int32 Y = 0; Y < HexGridSize.Y; ++Y)
	{
		for (int32 X = 0; X < HexGridSize.X; ++X)
		{
			int32 GridId = Y * HexGridSize.X + X;
			const FHexCellData& CellData = HexGrids[GridId];

			FCachedSectionData CellMesh;
			GenerateHexCell(CellData, CellMesh);

			int32 BaseIndex = MeshSection.Vertices.Num();
			MeshSection.Vertices.Append(CellMesh.Vertices);
			MeshSection.Normals.Append(CellMesh.Normals);
			MeshSection.VertexColors.Append(CellMesh.VertexColors);
			
			int32 NumOfIndices = CellMesh.Vertices.Num();
			MeshSection.Triangles.Reserve(MeshSection.Triangles.Num() + NumOfIndices);
			for (int32 Index : CellMesh.Triangles)
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

			FTransform Instance{ CellData.CellCenter };
			CoordTextComponent->AddInstance(Instance, false);
			CoordTextComponent->SetCustomDataValue(GridId, 0, CellData.GridCoord.X);
			CoordTextComponent->SetCustomDataValue(GridId, 1, CellData.GridCoord.Z);
		}
	}
	CoordTextComponent->MarkRenderStateDirty();
}


void AHexTerrainGenerator::GenerateHexCell(const FHexCellData& InCellData, FCachedSectionData& OutCellMesh)
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
	}
	
	OutCellMesh.Vertices.Empty(CORNER_NUM * 2);
	OutCellMesh.Triangles.Empty(CORNER_NUM * 2 * 3);
	OutCellMesh.Normals.Empty(CORNER_NUM * 2);
	OutCellMesh.VertexColors.Empty(CORNER_NUM * 2);

	// Inner HexCell
	for (int32 Index = 0; Index < CORNER_NUM; ++Index)
	{
		OutCellMesh.Vertices.Add(InCellData.CellCenter + FHexCellData::HexVertices[Index]);
		OutCellMesh.Normals.Add(FVector::UpVector);
		OutCellMesh.VertexColors.Add(InCellData.SRGBColor);
	}

	for (int32 Index = 1; Index < CORNER_NUM - 1; ++Index)
	{
		OutCellMesh.Triangles.Add(0);
		OutCellMesh.Triangles.Add(Index + 1);
		OutCellMesh.Triangles.Add(Index);
	}

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

	const FVector& OppositeCenter = OppositeCell.CellCenter;
	FVector ToV0 = OppositeCenter + FHexCellData::HexVertices[HexLink.ToVert.X];
	FVector ToV1 = OppositeCenter + FHexCellData::HexVertices[HexLink.ToVert.Y];
	FVector FromV0 = OutCellMesh.Vertices[HexLink.FromVert.X];
	FVector FromV1 = OutCellMesh.Vertices[HexLink.FromVert.Y];

	if (HexLink.LinkState == EHexLinkState::Terrace)
	{
		int32 NumOfZSteps = FMath::Abs(OppositeCell.Elevation - InCellData.Elevation);
		int32 NumOfSteps = NumOfZSteps * 2 - 1;

		FVector LastStepV0 = FromV0;
		FVector LastStepV1 = FromV1;
		FColor LastStepColor = InCellData.SRGBColor;
		for (int32 StepIndex = 1; StepIndex <= NumOfSteps; ++StepIndex)
		{
			FVector CurStepV0;
			FVector CurStepV1;

			float RatioXY = float(StepIndex) / float(NumOfSteps);
			int32 StepZIndex = (StepIndex - 1) / 2 + 1;
			float RatioZ = float(StepZIndex) / float(NumOfZSteps);

			CurStepV0.X = FMath::Lerp(FromV0.X, ToV0.X, RatioXY);
			CurStepV0.Y = FMath::Lerp(FromV0.Y, ToV0.Y, RatioXY);
			CurStepV0.Z = FMath::Lerp(FromV0.Z, ToV0.Z, RatioZ);
			
			CurStepV1.X = FMath::Lerp(FromV1.X, ToV1.X, RatioXY);
			CurStepV1.Y = FMath::Lerp(FromV1.Y, ToV1.Y, RatioXY);
			CurStepV1.Z = FMath::Lerp(FromV1.Z, ToV1.Z, RatioZ);

			FColor CurStepColor;
			CurStepColor.R = FMath::Lerp(InCellData.SRGBColor.R, OppositeCell.SRGBColor.R, RatioZ);
			CurStepColor.G = FMath::Lerp(InCellData.SRGBColor.G, OppositeCell.SRGBColor.G, RatioZ);
			CurStepColor.B = FMath::Lerp(InCellData.SRGBColor.B, OppositeCell.SRGBColor.B, RatioZ);
			CurStepColor.A = FMath::Lerp(InCellData.SRGBColor.A, OppositeCell.SRGBColor.A, RatioZ);
			
			FillQuad(LastStepV0, LastStepV1, CurStepV0, CurStepV1, LastStepColor, LastStepColor, CurStepColor, CurStepColor, OutCellMesh);

			LastStepV0 = CurStepV0;
			LastStepV1 = CurStepV1;
			LastStepColor = CurStepColor;
		}
	}
	else
	{
		FillQuad(FromV0, FromV1, ToV0, ToV1, InCellData.SRGBColor, InCellData.SRGBColor, OppositeCell.SRGBColor, OppositeCell.SRGBColor, OutCellMesh);
	}
}

void AHexTerrainGenerator::GenerateHexCorner(const FHexCellData& InCellData, EHexDirection CornerDirection, FCachedSectionData& OutCellMesh)
{
	int32 CIndex = static_cast<uint8>(CornerDirection) - 4;
	const FHexCellCorner& CornerData = InCellData.HexCorners[CIndex];

	int32 NumOfPlanes = 0, NumOfSlopes = 0, NumOfTerraces = 0, NumOfCliffs = 0;
	auto CountStateNumber = [&NumOfPlanes, &NumOfSlopes, &NumOfTerraces, &NumOfCliffs](EHexLinkState InState) {
		switch (InState)
		{
		case EHexLinkState::Plane:
			++NumOfPlanes;
			break;
		case EHexLinkState::Slope:
			++NumOfSlopes;
			break;
		case EHexLinkState::Terrace:
			++NumOfTerraces;
			break;
		case EHexLinkState::Cliff:
			++NumOfCliffs;
			break;
		}
	};
	
	CountStateNumber(CornerData.LinkState[0]);
	CountStateNumber(CornerData.LinkState[1]);
	CountStateNumber(CornerData.LinkState[2]);

	const FHexCellData& Cell1 = HexGrids[CornerData.LinkedCellsId.X];
	const FHexCellData& Cell2 = HexGrids[CornerData.LinkedCellsId.Y];
	const FHexCellData& Cell3 = HexGrids[CornerData.LinkedCellsId.Z];

	//GenerateNoTerraceCorner(Cell1, Cell2, Cell3, CornerData, OutCellMesh);
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
	
	auto CalcTerraceVerts = [](TArray<TArray<FVector>>& OutVerts, TArray<TArray<FColor>>& OutColors,
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
	Verts01[0].Add(Vert0);
	Verts02.AddDefaulted();
	Verts02[0].Add(Vert0);
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
		CalcLinearVerts(Verts01, Colors01, Vert1, Vert0, CellsList[1]->SRGBColor, CellsList[0]->SRGBColor, CellsList[1]->Elevation, CellsList[0]->Elevation);
	}

	if (LinkState[2] == EHexLinkState::Terrace)
	{
		CalcTerraceVerts(Verts02, Colors02, Vert2, Vert0, CellsList[2]->SRGBColor, CellsList[0]->SRGBColor, CellsList[2]->Elevation, CellsList[0]->Elevation);
	}
	else
	{
		CalcLinearVerts(Verts02, Colors02, Vert2, Vert0, CellsList[2]->SRGBColor, CellsList[0]->SRGBColor, CellsList[2]->Elevation, CellsList[0]->Elevation);
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
			CalcLinearVerts(Verts01, Colors01, Vert2, Vert1, CellsList[2]->SRGBColor, CellsList[1]->SRGBColor, CellsList[2]->Elevation, CellsList[1]->Elevation);
		}
		else if (CellsList[1]->Elevation > CellsList[2]->Elevation)// 2 -> 1
		{
			CalcLinearVerts(Verts02, Colors02, Vert1, Vert2, CellsList[1]->SRGBColor, CellsList[2]->SRGBColor, CellsList[1]->Elevation, CellsList[2]->Elevation);
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

FVector AHexTerrainGenerator::CalcHexCellCenter(const FIntPoint& GridIndex, int32 Elevation)
{
	static FVector2D VertOffsetScale{ 1.732050807568877, 1.5 };
	float CellOuterRadius = HexCellRadius + HexCellBorderWidth;

	FVector VertOffset;
	VertOffset.X = (GridIndex.X + (GridIndex.Y % 2) * 0.5) * CellOuterRadius * VertOffsetScale.X;
	VertOffset.Y = GridIndex.Y * CellOuterRadius * VertOffsetScale.Y;
	VertOffset.Z = Elevation * HexElevationStep;
	
	return VertOffset;
}

FVector AHexTerrainGenerator::CalcFaceNormal(const FVector& V0, const FVector& V1, const FVector& V2)
{
	FVector Edge1 = (V1 - V0);
	FVector Edge2 = (V2 - V0);
	return FVector::CrossProduct(Edge1, Edge2);
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

		FVector FaceNormal = CalcFaceNormal(OutCellMesh.Vertices[BaseIndex], OutCellMesh.Vertices[BaseIndex + 1], OutCellMesh.Vertices[BaseIndex + 2]);
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

		FVector FaceNormal = CalcFaceNormal(OutCellMesh.Vertices[BaseIndex], OutCellMesh.Vertices[BaseIndex + 2], OutCellMesh.Vertices[BaseIndex + 1]);
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

		FVector FaceNormal = CalcFaceNormal(OutCellMesh.Vertices[BaseIndex], OutCellMesh.Vertices[BaseIndex + 1], OutCellMesh.Vertices[BaseIndex + 2]);
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

		FVector FaceNormal = CalcFaceNormal(OutCellMesh.Vertices[BaseIndex], OutCellMesh.Vertices[BaseIndex + 1], OutCellMesh.Vertices[BaseIndex + 2]);
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

		FVector FaceNormal = CalcFaceNormal(OutCellMesh.Vertices[BaseIndex], OutCellMesh.Vertices[BaseIndex + 1], OutCellMesh.Vertices[BaseIndex + 2]);
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