#include "HexTerrainGenerator.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Kismet/GameplayStatics.h"

//#pragma optimize("", off)

//////////////////////////////////////////////////////////////////////////
FIntPoint FHexCellData::ChunkSize{ 0, 0 };
int32 FHexCellData::ChunkCountX = 0;
uint8 FHexCellData::CellSubdivision = 0u;
int32 FHexCellData::MaxTerranceElevation = 0;
TArray<FVector> FHexCellData::HexVertices;
TArray<FVector> FHexCellData::HexSubVertices;

FHexCellData::FHexCellData(const FIntPoint& InIndex)
	: GridIndex(InIndex.X + InIndex.Y * ChunkSize.X * ChunkCountX)
	, GridCoord(CalcGridCoordinate(InIndex))
	, CellCenter(EForceInit::ForceInitToZero)
	, SRGBColor(0xffffffff), Elevation(0), WaterLevel(0)
{
	GridId.X = InIndex.X / ChunkSize.X;
	GridId.Y = InIndex.Y / ChunkSize.Y;
	GridId.Z = InIndex.X - GridId.X * ChunkSize.X;
	GridId.W = InIndex.Y - GridId.Y * ChunkSize.Y;
}

void FHexCellData::LinkBorder(FHexCellData& OtherCell, EHexDirection LinkDirection)
{
	EHexBorderState LinkState = CalcLinkState(OtherCell, *this);

	uint8 LinkId = static_cast<uint8>(LinkDirection);
	uint8 OtherLinkId = static_cast<uint8>(CalcOppositeDirection(LinkDirection));

	FHexCellBorder& Border1 = HexNeighbors[LinkId];
	Border1.LinkedCellIndex = OtherCell.GridIndex;
	Border1.LinkState = LinkState;

	Border1.FromVert.Y = LinkId;
	Border1.FromVert.X = (Border1.FromVert.Y + CORNER_NUM - 1) % CORNER_NUM;
	Border1.ToVert.X = OtherLinkId;
	Border1.ToVert.Y = (Border1.ToVert.X + CORNER_NUM - 1) % CORNER_NUM;
	
	FHexCellBorder& Border2 = OtherCell.HexNeighbors[OtherLinkId];
	Border2.LinkedCellIndex = GridIndex;
	Border2.LinkState = LinkState;

	Border2.FromVert.Y = OtherLinkId;
	Border2.FromVert.X = (Border2.FromVert.Y + CORNER_NUM - 1) % CORNER_NUM;
	Border2.ToVert.X = LinkId;
	Border2.ToVert.Y = (Border2.ToVert.X + CORNER_NUM - 1) % CORNER_NUM;
}

void FHexCellData::LinkCorner(FHexCellData& Cell1, FHexCellData& Cell2, EHexDirection LinkDirection)
{
	uint8 LinkId = static_cast<uint8>(LinkDirection);

	FHexCellCorner& Corner = HexCorners[LinkId - 4];
	Corner.LinkedCellsIndex.X = GridIndex;
	Corner.LinkedCellsIndex.Y = Cell1.GridIndex;
	Corner.LinkedCellsIndex.Z = Cell2.GridIndex;

	Corner.LinkState[0] = CalcLinkState(*this, Cell1);
	Corner.LinkState[1] = CalcLinkState(Cell1, Cell2);
	Corner.LinkState[2] = CalcLinkState(Cell2, *this);

	Corner.VertsId.X = LinkId - 1;
	Corner.VertsId.Y = LinkId - 3;
	Corner.VertsId.Z = (LinkId - 5 + CORNER_NUM) % CORNER_NUM;
}

void FHexCellData::LinkRoad(int32 RoadIndex, EHexDirection LinkDirection)
{
	switch (HexRiver.RiverState)
	{
	case EHexRiverState::None:
		HexRoad.LinkRoad(RoadIndex, LinkDirection);
		break;

	case EHexRiverState::StartPoint:
		if (LinkDirection != HexRiver.OutgoingDirection)
			HexRoad.LinkRoad(RoadIndex, LinkDirection);
		break;

	case EHexRiverState::EndPoint:
		if (LinkDirection != HexRiver.IncomingDirection)
			HexRoad.LinkRoad(RoadIndex, LinkDirection);
		break;

	case EHexRiverState::PassThrough:
		if (LinkDirection != HexRiver.IncomingDirection && LinkDirection != HexRiver.OutgoingDirection)
			HexRoad.LinkRoad(RoadIndex, LinkDirection);
		break;
	}
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
	return static_cast<EHexDirection>((DirNum + CORNER_HALF_UNUM) % CORNER_UNUM);
}

EHexDirection FHexCellData::CalcPreviousDirection(EHexDirection InDirection)
{
	uint8 DirNum = static_cast<uint8>(InDirection);
	return static_cast<EHexDirection>((DirNum - 1u + CORNER_UNUM) % CORNER_UNUM);
}

EHexDirection FHexCellData::CalcNextDirection(EHexDirection InDirection)
{
	uint8 DirNum = static_cast<uint8>(InDirection);
	return static_cast<EHexDirection>((DirNum + 1u) % CORNER_UNUM);
}

uint8 FHexCellData::CalcOppositeDirection(uint8 InDirection)
{
	return (InDirection + CORNER_HALF_UNUM) % CORNER_UNUM;
}

uint8 FHexCellData::CalcPreviousDirection(uint8 InDirection)
{
	return (InDirection - 1u + CORNER_UNUM) % CORNER_UNUM;
}

uint8 FHexCellData::CalcNextDirection(uint8 InDirection)
{
	return (InDirection + 1u) % CORNER_UNUM;
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

EHexBorderState FHexCellData::CalcLinkState(const FHexCellData& Cell1, const FHexCellData& Cell2)
{
	int32 ElevationDiff = FMath::Abs(Cell1.Elevation - Cell2.Elevation);
	EHexBorderState LinkState = EHexBorderState::Flat;
	if (ElevationDiff == 0)
		LinkState = EHexBorderState::Flat;
	else if (ElevationDiff == 1)
		LinkState = EHexBorderState::Slope;
	else if (ElevationDiff <= MaxTerranceElevation)
		LinkState = EHexBorderState::Terrace;
	else
		LinkState = EHexBorderState::Cliff;
	return LinkState;
}

uint8 FHexCellData::GetVertIdFromDirection(EHexDirection InDirection, bool bSubVert, uint8 InState)
{
	uint8 DirectionId = static_cast<uint8>(InDirection);
	DirectionId = CalcPreviousDirection(DirectionId);

	if (bSubVert)
	{
		InState = FMath::Clamp<uint8>(InState, 0u, 2u);

		uint8 SubCenterIndex = 0u;
		switch (InState)
		{
		case 0u:
			break;

		case 1u:
		default:
			SubCenterIndex = (CellSubdivision - 1) / 2;
			break;

		case 2u:
			SubCenterIndex = CellSubdivision - 1;
			break;
		}

		return DirectionId * CellSubdivision + SubCenterIndex;
	}
	else
	{
		return DirectionId;
	}
}

bool FHexCellData::IsValidRiverDirection(const FHexCellData& FromCell, const FHexCellData& ToCell)
{
	return FromCell.Elevation >= ToCell.Elevation || FromCell.WaterLevel == ToCell.Elevation;
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

FHexVertexData::FHexVertexData(const FVector& InPos)
	:Position(InPos), bHasNormal(false), bHasUV0(false), bHasUV1(false), bHasVertexColor(false), VertexState(0u), VertexIndex(-1)
{}

FHexVertexData::FHexVertexData(const FVector& InPos, const FColor& InColor)
	:Position(InPos), VertexColor(InColor)
	, bHasNormal(false), bHasUV0(false), bHasUV1(false), bHasVertexColor(true), VertexState(0u), VertexIndex(-1)
{}

FHexVertexData::FHexVertexData(const FVector& InPos, const FVector2D& InUV0)
	:Position(InPos), UV0(InUV0)
	, bHasNormal(false), bHasUV0(true), bHasUV1(false), bHasVertexColor(false), VertexState(0u), VertexIndex(-1)
{}

FHexVertexData::FHexVertexData(const FVector& InPos, const FColor& InColor, const FVector& InNormal)
	:Position(InPos), Normal(InNormal), VertexColor(InColor)
	, bHasNormal(true), bHasUV0(false), bHasUV1(false), bHasVertexColor(true), VertexState(0u), VertexIndex(-1)
{}

FHexVertexData::FHexVertexData(const FVector& InPos, const FColor& InColor, const FVector2D& InUV0)
	:Position(InPos), UV0(InUV0), VertexColor(InColor)
	, bHasNormal(false), bHasUV0(true), bHasUV1(false), bHasVertexColor(true), VertexState(0u), VertexIndex(-1)
{}

FHexVertexData::FHexVertexData(const FVector& InPos, const FColor& InColor, const FVector& InNormal, const FVector2D& InUV0)
	:Position(InPos), Normal(InNormal), UV0(InUV0), VertexColor(InColor)
	, bHasNormal(true), bHasUV0(true), bHasUV1(false), bHasVertexColor(true), VertexState(0u), VertexIndex(-1)
{}

FHexVertexData FHexVertexData::LerpVertex(const FHexVertexData& FromV, const FHexVertexData& ToV, FVector PosRatio, float AttrRatio)
{
	FVector NewVertex;
	NewVertex.X = FMath::Lerp(FromV.Position.X, ToV.Position.X, PosRatio.X);
	NewVertex.Y = FMath::Lerp(FromV.Position.Y, ToV.Position.Y, PosRatio.Y);
	NewVertex.Z = FMath::Lerp(FromV.Position.Z, ToV.Position.Z, PosRatio.Z);

	FHexVertexData OutVertex{ NewVertex };
	
	bool bAllHasVertexColor = FromV.bHasVertexColor && ToV.bHasVertexColor;
	if (bAllHasVertexColor)
	{
		FColor NewColor;
		NewColor.R = FMath::Lerp(FromV.VertexColor.R, ToV.VertexColor.R, PosRatio.Z);
		NewColor.G = FMath::Lerp(FromV.VertexColor.G, ToV.VertexColor.G, PosRatio.Z);
		NewColor.B = FMath::Lerp(FromV.VertexColor.B, ToV.VertexColor.B, PosRatio.Z);
		NewColor.A = FMath::Lerp(FromV.VertexColor.A, ToV.VertexColor.A, PosRatio.Z);

		OutVertex.SetVertexColor(NewColor);
	}
	
	bool bAllHasUV0 = FromV.bHasUV0 && ToV.bHasUV0;
	if (bAllHasUV0)
	{
		FVector2D NewUV0;
		NewUV0.X = FMath::Lerp(FromV.UV0.X, ToV.UV0.X, AttrRatio);
		NewUV0.Y = FMath::Lerp(FromV.UV0.Y, ToV.UV0.Y, AttrRatio);

		OutVertex.SetUV0(NewUV0);
	}

	bool bAllHasUV1 = FromV.bHasUV1 && ToV.bHasUV1;
	if (bAllHasUV1)
	{
		FVector2D NewUV1;
		NewUV1.X = FMath::Lerp(FromV.UV1.X, ToV.UV1.X, AttrRatio);
		NewUV1.Y = FMath::Lerp(FromV.UV1.Y, ToV.UV1.Y, AttrRatio);

		OutVertex.SetUV1(NewUV1);
	}

	return OutVertex;
}

FHexVertexData FHexVertexData::ApplyOverride(const FVector& InPosOffset, const FColor* InOverrideColor, const FVector2D* InOverrideUV0) const
{
	FHexVertexData CopiedVertex = *this;
	CopiedVertex.VertexIndex = -1;
	CopiedVertex.ApplyOverrideInline(InPosOffset, InOverrideColor, InOverrideUV0);
	return CopiedVertex;
}

void FHexVertexData::ApplyOverrideInline(const FVector& InPosOffset, const FColor* InOverrideColor, const FVector2D* InOverrideUV0)
{
	Position += InPosOffset;

	if (InOverrideColor != nullptr)
		SetVertexColor(*InOverrideColor);

	if (InOverrideUV0 != nullptr)
		SetUV0(*InOverrideUV0);
}

struct FCachedSectionData
{
protected:
	TArray<FVector> Vertices;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0s;
	TArray<FVector2D> UV1s;
	TArray<FColor> VertexColors;
	TArray<int32> Triangles;
	TArray<FProcMeshTangent> Tangents;
	
	//FBox BoundingBox;
	
public:
	void MeshSection(const FCachedSectionData &Other)
	{
		if (Other.IsEmpty())
			return;

		int32 BaseIndex = Vertices.Num();
		Vertices.Append(Other.Vertices);
		Normals.Append(Other.Normals);
		UV0s.Append(Other.UV0s);
		UV1s.Append(Other.UV1s);
		VertexColors.Append(Other.VertexColors);

		int32 NumOfIndices = Other.Triangles.Num();
		Triangles.Reserve(Other.Triangles.Num() + NumOfIndices);
		for (int32 Index : Other.Triangles)
			Triangles.Add(BaseIndex + Index);
	}

	void AddTriangle(const FHexVertexData& V0, const FHexVertexData& V1, const FHexVertexData& V2)
	{
		int32 BaseIndex = Vertices.Num();
		Vertices.Add(V0.Position);
		Vertices.Add(V1.Position);
		Vertices.Add(V2.Position);

		if (V0.bHasVertexColor && V1.bHasVertexColor && V2.bHasVertexColor)
		{
			VertexColors.Add(V0.VertexColor);
			VertexColors.Add(V1.VertexColor);
			VertexColors.Add(V2.VertexColor);
		}

		if (V0.bHasNormal && V1.bHasNormal && V2.bHasNormal)
		{
			Normals.Add(V0.Normal);
			Normals.Add(V1.Normal);
			Normals.Add(V2.Normal);
		}
		else
		{
			FVector FaceNormal = CalcFaceNormal(V0.Position, V1.Position, V2.Position);
			Normals.Add(FaceNormal);
			Normals.Add(FaceNormal);
			Normals.Add(FaceNormal);
		}

		if (V0.bHasUV0 && V1.bHasUV0 && V2.bHasUV0)
		{
			UV0s.Add(V0.UV0);
			UV0s.Add(V1.UV0);
			UV0s.Add(V2.UV0);
		}
		
		if (V0.bHasUV1 && V1.bHasUV1 && V2.bHasUV1)
		{
			UV1s.Add(V0.UV1);
			UV1s.Add(V1.UV1);
			UV1s.Add(V2.UV1);
		}

		Triangles.Add(BaseIndex);
		Triangles.Add(BaseIndex + 1);
		Triangles.Add(BaseIndex + 2);
	}

	void AddQuad(const FHexVertexData& V0, const FHexVertexData& V1, const FHexVertexData& V2, const FHexVertexData& V3)
	{
		int32 BaseIndex = Vertices.Num();
		Vertices.Add(V0.Position);
		Vertices.Add(V1.Position);
		Vertices.Add(V2.Position);
		Vertices.Add(V3.Position);

		if (V0.bHasVertexColor && V1.bHasVertexColor && V2.bHasVertexColor && V3.bHasVertexColor)
		{
			VertexColors.Add(V0.VertexColor);
			VertexColors.Add(V1.VertexColor);
			VertexColors.Add(V2.VertexColor);
			VertexColors.Add(V3.VertexColor);
		}

		if (V0.bHasNormal && V1.bHasNormal && V2.bHasNormal && V3.bHasNormal)
		{
			Normals.Add(V0.Normal);
			Normals.Add(V1.Normal);
			Normals.Add(V2.Normal);
			Normals.Add(V3.Normal);
		}
		else
		{
			FVector FaceNormal = CalcFaceNormal(V0.Position, V2.Position, V1.Position);
			Normals.Add(FaceNormal);
			Normals.Add(FaceNormal);
			Normals.Add(FaceNormal);
			Normals.Add(FaceNormal);
		}

		if (V0.bHasUV0 && V1.bHasUV0 && V2.bHasUV0 && V3.bHasUV0)
		{
			UV0s.Add(V0.UV0);
			UV0s.Add(V1.UV0);
			UV0s.Add(V2.UV0);
			UV0s.Add(V3.UV0);
		}

		if (V0.bHasUV1 && V1.bHasUV1 && V2.bHasUV1 && V3.bHasUV1)
		{
			UV1s.Add(V0.UV1);
			UV1s.Add(V1.UV1);
			UV1s.Add(V2.UV1);
			UV1s.Add(V3.UV1);
		}

		Triangles.Add(BaseIndex);
		Triangles.Add(BaseIndex + 2);
		Triangles.Add(BaseIndex + 1);

		Triangles.Add(BaseIndex + 1);
		Triangles.Add(BaseIndex + 2);
		Triangles.Add(BaseIndex + 3);
	}

	void Reset(int32 NumOfVerts)
	{
		Vertices.Empty(NumOfVerts + 1);
		Triangles.Empty(NumOfVerts * 3);
		UV0s.Empty(NumOfVerts + 1);
		UV1s.Empty(NumOfVerts + 1);
		Normals.Empty(NumOfVerts + 1);
		VertexColors.Empty(NumOfVerts + 1);
	}

	FVector CalcFaceNormal(const FVector& V0, const FVector& V1, const FVector& V2)
	{
		FVector Edge1 = (V1 - V0);
		FVector Edge2 = (V2 - V0);
		FVector NormalVector = FVector::CrossProduct(Edge2, Edge1);
		return NormalVector.GetSafeNormal();
	}

	int32 AddVertex(const FHexVertexData& InVertex)
	{
		int32 Index = Vertices.Add(InVertex.Position);

		if (InVertex.bHasVertexColor)
			Normals.Add(InVertex.Normal);

		if (InVertex.bHasUV0)
			UV0s.Add(InVertex.UV0);

		if (InVertex.bHasUV1)
			UV1s.Add(InVertex.UV1);

		if (InVertex.bHasVertexColor)
			VertexColors.Add(InVertex.VertexColor);

		return Index;
	}

	void AddFace(int32 I0, int32 I1, int32 I2)
	{
		Triangles.Add(I0);
		Triangles.Add(I1);
		Triangles.Add(I2);
	}

	const TArray<FVector>& GetVertices() const { return Vertices; }
	const TArray<FVector>& GetNormals() const { return Normals; }
	const TArray<FVector2D>& GetUV0s() const { return UV0s; }
	const TArray<FVector2D>& GetUV1s() const { return UV1s; }
	const TArray<FColor>& GetVertexColors() const { return VertexColors; }
	const TArray<int32>& GetTriangles() const { return Triangles; }
	const TArray<FProcMeshTangent>& GetTangents() const { return Tangents; }
	bool IsEmpty() const { return Vertices.IsEmpty(); }
};

struct FCachedChunkData
{
	// Terrain
	FCachedSectionData GroundSection;
	FCachedSectionData RoadSection;

	FCachedSectionData WaterSection;
	FCachedSectionData EstuarySection;
	FCachedSectionData RiverSection;

	FCachedSectionData CollisionSection;

	//Features
	TArray<FTransform> FeatureTransforms;
};

struct FCachedTerrainData 
{
	TArray<FCachedChunkData> TerrainChunksSection;
};

int32 FHexCellConfigData::DefaultElevation = 0;
int32 FHexCellConfigData::DefaultWaterLevel = 0;
EHexTerrainType FHexCellConfigData::DefaultTerrainType = EHexTerrainType::Water;

//////////////////////////////////////////////////////////////////////////

// Sets default values
AHexTerrainGenerator::AHexTerrainGenerator()
	: NoiseTexturePath(TEXT("Content/Noise.png"))
	, ConfigFileName(TEXT("HexTerrainConfig.json"))
	, HexChunkCount(4, 3)
	, HexChunkSize(5, 5)
	, HexCellRadius(100.0f)
	, HexCellBorderWidth(10.0f)
	, HexCellSubdivision(3u)
	, HexElevationStep(5.0f)
	, MaxElevationForTerrace(4)
	, RiverElevationOffset(-1)
	, RiverSubdivision(2u)
	, RoadElevationOffset(0.5f)
	, RoadWidthRatio(0.5f)
	, PerturbingStrengthHV(1.0f, 1.0f)
	, PerturbingScalingHV(0.25f, 1.0f)

	, HexEditMode(EHexEditMode::Ground)
	, HexEditGridId(-1, -1)
	, HexEditElevation(0)
	, HexEditWaterLevel(0)
	, HexEditTerrainType(EHexTerrainType::None)
	, HexEditRiverId(-1)
	, HexEditRiverStartPoint(-1, -1)
	, HexEditRiverLastPoint(-1, -1)
	, HexEditRoadFirstPoint(-1, -1)
{
 	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));

	TerrainMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMeshComponent"));
	TerrainMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TerrainMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	TerrainMeshComponent->Mobility = EComponentMobility::Movable;
	TerrainMeshComponent->SetGenerateOverlapEvents(false);
	TerrainMeshComponent->SetRenderCustomDepth(true);
	TerrainMeshComponent->SetupAttachment(RootComponent);
	TerrainMeshComponent->OnClicked.AddDynamic(this, &AHexTerrainGenerator::OnClicked);
	TerrainMeshComponent->OnReleased.AddDynamic(this, &AHexTerrainGenerator::OnReleased);

	FeatureMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FeatureMeshComponent"));
	FeatureMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FeatureMeshComponent->Mobility = EComponentMobility::Movable;
	FeatureMeshComponent->SetGenerateOverlapEvents(false);
	FeatureMeshComponent->SetupAttachment(RootComponent);

	CoordTextComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GridCoordComponent"));
	CoordTextComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoordTextComponent->Mobility = EComponentMobility::Movable;
	CoordTextComponent->SetGenerateOverlapEvents(false);
	CoordTextComponent->SetCastShadow(false);
	CoordTextComponent->SetupAttachment(RootComponent);
	CoordTextComponent->SetRelativeLocation(FVector{ 0.0, 0.0, 12.0 });

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

	UpdateHexGridsData();
}

void AHexTerrainGenerator::SaveTerrain()
{
	SaveHexTerrainConfig();
}

void AHexTerrainGenerator::GenerateTerrain()
{
	if (HexGrids.IsEmpty())
		return;

	int32 HexGridSizeX = HexChunkCount.X * HexChunkSize.X;
	int32 HexGridSizeY = HexChunkCount.Y * HexChunkSize.Y;

	// Create HexCellMesh
	FCachedTerrainData CachedTerrain;

	TObjectPtr<UMaterialInterface> GroundMaterial = MaterialsLibrary.FindOrAdd("Ground", nullptr);
	TObjectPtr<UMaterialInterface> RoadMaterial = MaterialsLibrary.FindOrAdd("Road", nullptr);
	TObjectPtr<UMaterialInterface> WaterMaterial = MaterialsLibrary.FindOrAdd("Water", nullptr);
	TObjectPtr<UMaterialInterface> EstuaryMaterial = MaterialsLibrary.FindOrAdd("Estuary", nullptr);
	TObjectPtr<UMaterialInterface> RiverMaterial = MaterialsLibrary.FindOrAdd("River", nullptr);

	TerrainMeshComponent->ClearAllMeshSections();
	for (int32 CY = 0; CY < HexChunkCount.Y; ++CY)
	{
		for (int32 CX = 0; CX < HexChunkCount.X; ++CX)
		{
			int32 ChunkIndex = CY * HexChunkCount.X + CX;
			FCachedChunkData& CurrentChunkSection = CachedTerrain.TerrainChunksSection.AddDefaulted_GetRef();

			for (int32 GY = 0; GY < HexChunkSize.Y; ++GY)
			{
				for (int32 GX = 0; GX < HexChunkSize.X; ++GX)
				{
					int32 GridIndex = (CY * HexChunkSize.Y + GY) * HexGridSizeX + (CX * HexChunkSize.X + GX);
					const FHexCellData& CellData = HexGrids[GridIndex];

					GenerateHexCell(CellData, CurrentChunkSection);
					GenerateHexWaterCell(CellData, CurrentChunkSection);
				}
			}

			// Create Ground
			{
				FCachedSectionData& TerrainChunkSection = CurrentChunkSection.GroundSection;
				TerrainMeshComponent->CreateMeshSection(ChunkIndex, TerrainChunkSection.GetVertices(), TerrainChunkSection.GetTriangles(), TerrainChunkSection.GetNormals(),
					TerrainChunkSection.GetUV0s(), TerrainChunkSection.GetVertexColors(), TerrainChunkSection.GetTangents(), false);

				// Set Material
				if (!!GroundMaterial)
				{
					TerrainMeshComponent->SetMaterial(ChunkIndex, GroundMaterial);
				}
			}

			// Create Road
			if (!CurrentChunkSection.RoadSection.IsEmpty())
			{
				FCachedSectionData& RoadSection = CurrentChunkSection.RoadSection;
				int32 RoadSectionId = TerrainMeshComponent->GetNumSections();
				TerrainMeshComponent->CreateMeshSection(RoadSectionId, RoadSection.GetVertices(), RoadSection.GetTriangles(),
					RoadSection.GetNormals(), RoadSection.GetUV0s(), RoadSection.GetVertexColors(), RoadSection.GetTangents(), false);
				
				if (!!RoadMaterial)
				{
					TerrainMeshComponent->SetMaterial(RoadSectionId, RoadMaterial);
				}
			}

			// Create Water
			if (!CurrentChunkSection.WaterSection.IsEmpty())
			{
				FCachedSectionData& WaterSection = CurrentChunkSection.WaterSection;
				int32 WaterSectionId = TerrainMeshComponent->GetNumSections();
				TerrainMeshComponent->CreateMeshSection(WaterSectionId, WaterSection.GetVertices(), WaterSection.GetTriangles(),
					WaterSection.GetNormals(), WaterSection.GetUV0s(), WaterSection.GetVertexColors(), WaterSection.GetTangents(), false);

				if (!!WaterMaterial)
				{
					TerrainMeshComponent->SetMaterial(WaterSectionId, WaterMaterial);
				}
			}
			if (!CurrentChunkSection.EstuarySection.IsEmpty())
			{
				TArray<FVector2D> EmptyUV;
				FCachedSectionData& EstuarySection = CurrentChunkSection.EstuarySection;
				int32 EstuarySectionId = TerrainMeshComponent->GetNumSections();
				TerrainMeshComponent->CreateMeshSection(EstuarySectionId, EstuarySection.GetVertices(), EstuarySection.GetTriangles(),
					EstuarySection.GetNormals(), EstuarySection.GetUV0s(), EstuarySection.GetUV1s(), EmptyUV, EmptyUV, 
					EstuarySection.GetVertexColors(), EstuarySection.GetTangents(), false);

				if (!!EstuaryMaterial)
				{
					TerrainMeshComponent->SetMaterial(EstuarySectionId, EstuaryMaterial);
				}
			}

			// Create River
			if (!CurrentChunkSection.RiverSection.IsEmpty())
			{
				FCachedSectionData& RiverSection = CurrentChunkSection.RiverSection;
				int32 RiverSectionId = TerrainMeshComponent->GetNumSections();
				TerrainMeshComponent->CreateMeshSection(RiverSectionId, RiverSection.GetVertices(), RiverSection.GetTriangles(),
					RiverSection.GetNormals(), RiverSection.GetUV0s(), RiverSection.GetVertexColors(), RiverSection.GetTangents(), false);

				if (!!RiverMaterial)
				{
					TerrainMeshComponent->SetMaterial(RiverSectionId, RiverMaterial);
				}
			}

			// Create Collision
			if (!CurrentChunkSection.CollisionSection.IsEmpty())
			{
				FCachedSectionData& CollisionSection = CurrentChunkSection.CollisionSection;
				int32 CollisionSectionId = TerrainMeshComponent->GetNumSections();
				TerrainMeshComponent->CreateMeshSection(CollisionSectionId, CollisionSection.GetVertices(), CollisionSection.GetTriangles(),
					CollisionSection.GetNormals(), CollisionSection.GetUV0s(), CollisionSection.GetVertexColors(), CollisionSection.GetTangents(), true);
				TerrainMeshComponent->SetMeshSectionVisible(CollisionSectionId, false);
			}
		}
	}

	// Features
	AddTerrainFeatures(CachedTerrain);

	// Grid Coordinates
	AddGridCoordinates(HexGridSizeX, HexGridSizeY);
}

void AHexTerrainGenerator::AddTerrainFeatures(const FCachedTerrainData& CachedTerrain)
{
	TObjectPtr<UStaticMesh> FeatureModel = ModelsLibrary.FindOrAdd("Feature", nullptr);
	TObjectPtr<UMaterialInterface> FeatureMaterial = MaterialsLibrary.FindOrAdd("Feature", nullptr);
	if (!FeatureMeshComponent->GetStaticMesh() && !!FeatureModel)
	{
		FeatureMeshComponent->SetStaticMesh(FeatureModel.Get());
	}
	if (!!FeatureMaterial)
	{
		FeatureMeshComponent->SetMaterial(0, FeatureMaterial);
	}

	FeatureMeshComponent->ClearInstances();
	int32 NumOfChunks = CachedTerrain.TerrainChunksSection.Num();
	for (int32 ChunkId = 0; ChunkId < NumOfChunks; ++ChunkId)
	{
		const FCachedChunkData& ChunkData = CachedTerrain.TerrainChunksSection[ChunkId];
		int32 NumOfFeaturesPerChunk = ChunkData.FeatureTransforms.Num();
		
		for (int32 FeatureId = 0; FeatureId < NumOfFeaturesPerChunk; ++FeatureId)
		{
			FeatureMeshComponent->AddInstance(ChunkData.FeatureTransforms[FeatureId], false);
		}
	}
	//FeatureMeshComponent->MarkRenderStateDirty();
}

void AHexTerrainGenerator::AddGridCoordinates(int32 HexGridSizeX, int32 HexGridSizeY)
{
	TObjectPtr<UMaterialInterface> TextMaterial = MaterialsLibrary.FindOrAdd("Text", nullptr);
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
			int32 GridIndex = Y * HexGridSizeX + X;
			const FHexCellData& CellData = HexGrids[GridIndex];

			FTransform Instance{ CellData.CellCenter };
			CoordTextComponent->AddInstance(Instance, false);
			CoordTextComponent->SetCustomDataValue(GridIndex, 0, CellData.GridCoord.X);
			CoordTextComponent->SetCustomDataValue(GridIndex, 1, CellData.GridCoord.Z);
		}
	}
	CoordTextComponent->MarkRenderStateDirty();
}

bool AHexTerrainGenerator::LoadHexTerrainConfig()
{
	FString ConfigFilePath = FPaths::ProjectConfigDir() + ConfigFileName;
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

	TArray<TSharedPtr<FJsonValue>> ChunkSizeData = JsonRoot->GetArrayField(TEXT("ChunkSize"));
	ChunkSizeData[0]->TryGetNumber(HexChunkSize.X);
	ChunkSizeData[1]->TryGetNumber(HexChunkSize.Y);
	ChunkSizeData[2]->TryGetNumber(HexChunkCount.X);
	ChunkSizeData[3]->TryGetNumber(HexChunkCount.Y);

	TArray<TSharedPtr<FJsonValue>> ElevationsList = JsonRoot->GetArrayField(TEXT("Elevations"));
	TArray<TSharedPtr<FJsonValue>> WaterLevelsList = JsonRoot->GetArrayField(TEXT("WaterLevels"));
	TSharedPtr<FJsonObject> ColorsMap = JsonRoot->GetObjectField(TEXT("Colors"));
	TArray<TSharedPtr<FJsonValue>> TypesList = JsonRoot->GetArrayField(TEXT("HexTypes"));
	TArray<TSharedPtr<FJsonValue>> RiversList = JsonRoot->GetArrayField(TEXT("Rivers"));
	TArray<TSharedPtr<FJsonValue>> RoadsList = JsonRoot->GetArrayField(TEXT("Roads"));

	int32 HexGridSizeX = HexChunkCount.X * HexChunkSize.X;
	int32 HexGridSizeY = HexChunkCount.Y * HexChunkSize.Y;
	ConfigData.ElevationsList.Empty(HexGridSizeY);
	ConfigData.ElevationsList.AddDefaulted(HexGridSizeY);
	for (int32 Y = 0; Y < HexGridSizeY; ++Y)
	{
		ConfigData.ElevationsList[Y].Init(FHexCellConfigData::DefaultElevation, HexGridSizeX);
		if (Y >= ElevationsList.Num())
			continue;
		
		const TArray<TSharedPtr<FJsonValue>>& OneRow = ElevationsList[Y]->AsArray();
		for (int32 X = 0; X < HexGridSizeX; ++X)
		{
			int32 TempVal = FHexCellConfigData::DefaultElevation;
			if (X < OneRow.Num())
				OneRow[X]->TryGetNumber(TempVal);
			ConfigData.ElevationsList[Y][X] = TempVal;
		}
	}
	
	ConfigData.WaterLevelsList.Empty(HexGridSizeY);
	ConfigData.WaterLevelsList.AddDefaulted(HexGridSizeY);
	for (int32 Y = 0; Y < HexGridSizeY; ++Y)
	{
		ConfigData.WaterLevelsList[Y].Init(FHexCellConfigData::DefaultWaterLevel, HexGridSizeX);
		if (Y >= ElevationsList.Num())
			continue;

		const TArray<TSharedPtr<FJsonValue>>& OneRow = WaterLevelsList[Y]->AsArray();
		for (int32 X = 0; X < HexGridSizeX; ++X)
		{
			int32 TempVal = FHexCellConfigData::DefaultWaterLevel;
			if (X < OneRow.Num())
				OneRow[X]->TryGetNumber(TempVal);
			ConfigData.WaterLevelsList[Y][X] = TempVal;
		}
	}

	for (uint8 Index = 0u; Index < uint8(EHexTerrainType::MAX); ++Index)
	{
		FString OutColorStr;
		EHexTerrainType TerrainType = EHexTerrainType(Index);
		FString InTerrainTypeStr = FHexCellConfigData::GetHexTerrainString(TerrainType);
		if (ColorsMap->TryGetStringField(InTerrainTypeStr, OutColorStr))
		{
			FColor TempVal;
			TempVal.InitFromString(OutColorStr);
			ConfigData.ColorsMap.Add(TerrainType, TempVal);
		}
	}
	
	ConfigData.TerrainTypesList.Empty(HexGridSizeY);
	ConfigData.TerrainTypesList.AddDefaulted(HexGridSizeY);
	for (int32 Y = 0; Y < HexGridSizeY; ++Y)
	{
		ConfigData.TerrainTypesList[Y].Init(FHexCellConfigData::DefaultTerrainType, HexGridSizeX);
		if (Y >= TypesList.Num())
			continue;

		const TArray<TSharedPtr<FJsonValue>>& OneRow = TypesList[Y]->AsArray();
		for (int32 X = 0; X < HexGridSizeX; ++X)
		{
			uint8 TempVal = uint8(FHexCellConfigData::DefaultTerrainType);
			if (X < OneRow.Num())
			{
				OneRow[X]->TryGetNumber(TempVal);
				TempVal = FMath::Clamp(TempVal, 0u, uint8(EHexTerrainType::MAX));
			}
			ConfigData.TerrainTypesList[Y][X] = static_cast<EHexTerrainType>(TempVal);
		}
	}
	
	auto ProcessRiverRoadConfig = [](const TArray<TSharedPtr<FJsonValue>>& InConfigList, TArray<FHexRiverRoadConfigData>& OutDataList)
		{
			int32 NumOfRivers = InConfigList.Num();
			OutDataList.Empty(NumOfRivers);
			OutDataList.AddDefaulted(NumOfRivers);
			for (int32 Index = 0; Index < NumOfRivers; ++Index)
			{
				FHexRiverRoadConfigData& OneData = OutDataList[Index];
				TSharedPtr<FJsonObject> OneConfig = InConfigList[Index]->AsObject();

				TArray<TSharedPtr<FJsonValue>> StartPoint = OneConfig->GetArrayField(TEXT("StartPoint"));
				StartPoint[0]->TryGetNumber(OneData.StartPoint.X);
				StartPoint[1]->TryGetNumber(OneData.StartPoint.Y);

				TArray<TSharedPtr<FJsonValue>> ExtensionDirections = OneConfig->GetArrayField(TEXT("Directions"));
				for (TSharedPtr<FJsonValue> Direction : ExtensionDirections)
				{
					uint8 DirectionId = 0u;
					Direction->TryGetNumber(DirectionId);
					OneData.ExtensionDirections.Add(static_cast<EHexDirection>(DirectionId));
				}
			}
		};

	ProcessRiverRoadConfig(RiversList, ConfigData.RiversList);
	ProcessRiverRoadConfig(RoadsList, ConfigData.RoadsList);

	return true;
}

void AHexTerrainGenerator::SaveHexTerrainConfig()
{
	FString StructuredJson;
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	int32 HexGridSizeX = HexChunkCount.X * HexChunkSize.X;
	int32 HexGridSizeY = HexChunkCount.Y * HexChunkSize.Y;

	TArray<TSharedPtr<FJsonValue>> ChunkSizeData;
	ChunkSizeData.Add(MakeShared<FJsonValueNumber>(HexChunkSize.X));
	ChunkSizeData.Add(MakeShared<FJsonValueNumber>(HexChunkSize.Y));
	ChunkSizeData.Add(MakeShared<FJsonValueNumber>(HexChunkCount.X));
	ChunkSizeData.Add(MakeShared<FJsonValueNumber>(HexChunkCount.Y));

	TArray<TSharedPtr<FJsonValue>> ElevationsList;
	TArray<TSharedPtr<FJsonValue>> WaterLevelsList;
	TArray<TSharedPtr<FJsonValue>> TypesList;

	for (int32 Y = 0; Y < HexGridSizeY; ++Y)
	{
		TArray<TSharedPtr<FJsonValue>> ElevationRow;
		TArray<TSharedPtr<FJsonValue>> WaterLevelRow;
		TArray<TSharedPtr<FJsonValue>> TypeRow;

		for (int32 X = 0; X < HexGridSizeX; ++X)
		{
			ElevationRow.Add(MakeShared<FJsonValueNumber>(ConfigData.ElevationsList[Y][X]));
			WaterLevelRow.Add(MakeShared<FJsonValueNumber>(ConfigData.WaterLevelsList[Y][X]));
			TypeRow.Add(MakeShared<FJsonValueNumber>(uint8(ConfigData.TerrainTypesList[Y][X])));
		}

		ElevationsList.Add(MakeShared<FJsonValueArray>(ElevationRow));
		WaterLevelsList.Add(MakeShared<FJsonValueArray>(WaterLevelRow));
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
	
	TArray<TSharedPtr<FJsonValue>> RiversList;
	int32 NumOfRivers = ConfigData.RiversList.Num();
	for (int32 Index = 0; Index < NumOfRivers; ++Index)
	{
		FHexRiverRoadConfigData& RiverConfig = ConfigData.RiversList[Index];
		TSharedRef<FJsonObject> RiverData = MakeShareable(new FJsonObject());
		
		TArray<TSharedPtr<FJsonValue>> StartPoint;
		StartPoint.Add(MakeShared<FJsonValueNumber>(RiverConfig.StartPoint.X));
		StartPoint.Add(MakeShared<FJsonValueNumber>(RiverConfig.StartPoint.Y));
		RiverData->SetArrayField(TEXT("StartPoint"), StartPoint);
		
		TArray<TSharedPtr<FJsonValue>> Directions;
		for (EHexDirection Direction : RiverConfig.ExtensionDirections)
		{
			Directions.Add(MakeShared<FJsonValueNumber>(static_cast<uint8>(Direction)));
		}
		RiverData->SetArrayField(TEXT("Directions"), Directions);

		RiversList.Add(MakeShared<FJsonValueObject>(RiverData));
	}

	TArray<TSharedPtr<FJsonValue>> RoadsList;
	int32 NumOfRoads = ConfigData.RoadsList.Num();
	for (int32 Index = 0; Index < NumOfRoads; ++Index)
	{
		FHexRiverRoadConfigData& RoadConfig = ConfigData.RoadsList[Index];
		TSharedRef<FJsonObject> RoadData = MakeShareable(new FJsonObject());

		TArray<TSharedPtr<FJsonValue>> StartPoint;
		StartPoint.Add(MakeShared<FJsonValueNumber>(RoadConfig.StartPoint.X));
		StartPoint.Add(MakeShared<FJsonValueNumber>(RoadConfig.StartPoint.Y));
		RoadData->SetArrayField(TEXT("StartPoint"), StartPoint);

		TArray<TSharedPtr<FJsonValue>> Directions;
		for (EHexDirection Direction : RoadConfig.ExtensionDirections)
		{
			Directions.Add(MakeShared<FJsonValueNumber>(static_cast<uint8>(Direction)));
		}
		RoadData->SetArrayField(TEXT("Directions"), Directions);

		RoadsList.Add(MakeShared<FJsonValueObject>(RoadData));
	}

	JsonObject->SetArrayField(TEXT("ChunkSize"), ChunkSizeData);
	JsonObject->SetArrayField(TEXT("Elevations"), ElevationsList);
	JsonObject->SetArrayField(TEXT("WaterLevels"), WaterLevelsList);
	JsonObject->SetObjectField(TEXT("Colors"), ColorsMap);
	JsonObject->SetArrayField(TEXT("HexTypes"), TypesList);
	JsonObject->SetArrayField(TEXT("Rivers"), RiversList);
	JsonObject->SetArrayField(TEXT("Roads"), RoadsList);

	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&StructuredJson, /*Indent=*/0);

	if (FJsonSerializer::Serialize(JsonObject, JsonWriter) == false)
	{
		UE_LOG(LogJson, Warning, TEXT("HexTerrain: Unable to write out json"));
	}
	JsonWriter->Close();

	FString ConfigFilePath = FPaths::ProjectConfigDir() + ConfigFileName;
	FFileHelper::SaveStringToFile(StructuredJson, *ConfigFilePath);
	UE_LOG(LogTemp, Display, TEXT("Save Config To %s"), *ConfigFilePath);
}

void AHexTerrainGenerator::UpdateHexGridsData()
{
	if (!ConfigData.bConfigValid)
		return;

	FHexCellData::ChunkSize = HexChunkSize;
	FHexCellData::ChunkCountX = HexChunkCount.X;
	FHexCellData::CellSubdivision = HexCellSubdivision;
	FHexCellData::MaxTerranceElevation = MaxElevationForTerrace;
	FHexCellData::HexVertices.Empty(CORNER_NUM);
	FHexCellData::HexSubVertices.Empty(CORNER_NUM * HexCellSubdivision);
	CachedNoiseZ.Empty(10);

	// Create HexCellData
	int32 HexGridSizeX = HexChunkCount.X * HexChunkSize.X;
	int32 HexGridSizeY = HexChunkCount.Y * HexChunkSize.Y;
	HexGrids.Empty(HexGridSizeX * HexGridSizeY);
	for (int32 Y = 0; Y < HexGridSizeY; ++Y)
	{
		for (int32 X = 0; X < HexGridSizeX; ++X)
		{
			FIntPoint GridId{ X, Y };

			FHexCellData OneCell{ GridId };
			ConfigData.GetHexCellTerrainData(GridId, OneCell);
			OneCell.CellCenter = CalcHexCellCenter(GridId, OneCell.Elevation);

			int32 WIndex = FHexCellData::CalcGridIndexByCoord(FIntVector{ OneCell.GridCoord.X - 1, OneCell.GridCoord.Y + 1, OneCell.GridCoord.Z });
			int32 NWIndex = FHexCellData::CalcGridIndexByCoord(FIntVector{ OneCell.GridCoord.X, OneCell.GridCoord.Y + 1, OneCell.GridCoord.Z - 1 });
			int32 NEIndex = FHexCellData::CalcGridIndexByCoord(FIntVector{ OneCell.GridCoord.X + 1, OneCell.GridCoord.Y, OneCell.GridCoord.Z - 1 });
			if (WIndex >= 0)
				HexGrids[WIndex].LinkBorder(OneCell, EHexDirection::E);
			if (NWIndex >= 0)
				HexGrids[NWIndex].LinkBorder(OneCell, EHexDirection::SE);
			if (NEIndex >= 0)
				HexGrids[NEIndex].LinkBorder(OneCell, EHexDirection::SW);
			if (WIndex >= 0 && NWIndex >= 0)
				OneCell.LinkCorner(HexGrids[NWIndex], HexGrids[WIndex], EHexDirection::NW);
			if (NWIndex >= 0 && NEIndex >= 0)
				OneCell.LinkCorner(HexGrids[NEIndex], HexGrids[NWIndex], EHexDirection::NE);

			HexGrids.Add(OneCell);
		}
	}

	// Fill RiverData
	int32 NumOfGrids = HexGrids.Num();
	for (int32 Index = 0; Index < ConfigData.RiversList.Num(); ++Index)
	{
		const FHexRiverRoadConfigData& OneRiver = ConfigData.RiversList[Index];
		int32 LenOfRiver = OneRiver.ExtensionDirections.Num();

		int32 FirstIndex = OneRiver.StartPoint.Y * HexGridSizeX + OneRiver.StartPoint.X;
		if (FirstIndex >= NumOfGrids)
			continue;

		FHexCellData& FirstRiverNode = HexGrids[FirstIndex];
		FirstRiverNode.HexRiver.RiverIndex = Index;
		FirstRiverNode.HexRiver.RiverState = EHexRiverState::StartPoint;
		
		FHexCellData* LastRiverNode = &FirstRiverNode;
		for (int32 Step = 0; Step < LenOfRiver; ++Step)
		{
			EHexDirection StepDirection = OneRiver.ExtensionDirections[Step];

			int32 CurGridIndex = LastRiverNode->HexNeighbors[static_cast<uint8>(StepDirection)].LinkedCellIndex;
			if (CurGridIndex < 0)
			{
				LastRiverNode->HexRiver.RiverState = EHexRiverState::EndPoint;
				break;
			}

			FHexCellData& CurRiverNode = HexGrids[CurGridIndex];
			if (!FHexCellData::IsValidRiverDirection(*LastRiverNode, CurRiverNode))
			{
				LastRiverNode->HexRiver.RiverState = EHexRiverState::EndPoint;
				break;
			}

			CurRiverNode.HexRiver.RiverIndex = Index;
			CurRiverNode.HexRiver.RiverState = (Step == LenOfRiver - 1) ? EHexRiverState::EndPoint : EHexRiverState::PassThrough;

			LastRiverNode->HexRiver.OutgoingDirection = StepDirection;
			CurRiverNode.HexRiver.IncomingDirection = FHexCellData::CalcOppositeDirection(StepDirection);

			LastRiverNode = &CurRiverNode;
		}

		if (FirstRiverNode.HexRiver.RiverState == EHexRiverState::EndPoint)
		{
			FirstRiverNode.HexRiver.Clear();
		}
	}

	// Fill RoadData
	for (int32 Index = 0; Index < ConfigData.RoadsList.Num(); ++Index)
	{
		const FHexRiverRoadConfigData& OneRoad = ConfigData.RoadsList[Index];
		int32 LenOfRoad = OneRoad.ExtensionDirections.Num();

		int32 FirstIndex = OneRoad.StartPoint.Y * HexGridSizeX + OneRoad.StartPoint.X;
		if (FirstIndex >= NumOfGrids)
			continue;

		FHexCellData& RoadStartNode = HexGrids[FirstIndex];
		for (int32 RoadIndex = 0; RoadIndex < LenOfRoad; ++RoadIndex)
		{
			EHexDirection RoadDirection = OneRoad.ExtensionDirections[RoadIndex];

			int32 EndGridIndex = RoadStartNode.HexNeighbors[static_cast<uint8>(RoadDirection)].LinkedCellIndex;
			if (EndGridIndex < 0)
			{
				break;
			}

			FHexCellData& RoadEndNode = HexGrids[EndGridIndex];
			int32 ElevationDiff = FMath::Abs(RoadEndNode.Elevation - RoadStartNode.Elevation);
			if (ElevationDiff > MaxElevationForTerrace)
			{
				break;
			}
			
			RoadEndNode.LinkRoad(Index, FHexCellData::CalcOppositeDirection(RoadDirection));
			RoadStartNode.LinkRoad(Index, RoadDirection);
		}
	}
}

void AHexTerrainGenerator::GenerateHexCell(const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh)
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

	// Inner HexCell
	GenerateHexCenter(InCellData, OutTerrainMesh);

	OutTerrainMesh.CollisionSection = OutTerrainMesh.GroundSection;

	// Border
	int32 WIndex = InCellData.HexNeighbors[static_cast<uint8>(EHexDirection::W)].LinkedCellIndex;
	int32 NWIndex = InCellData.HexNeighbors[static_cast<uint8>(EHexDirection::NW)].LinkedCellIndex;
	int32 NEIndex = InCellData.HexNeighbors[static_cast<uint8>(EHexDirection::NE)].LinkedCellIndex;
	if (WIndex >= 0) // W Edge
	{
		GenerateHexBorder(InCellData, EHexDirection::W, OutTerrainMesh);
	}

	if (NWIndex >= 0) // NW Edge
	{
		GenerateHexBorder(InCellData, EHexDirection::NW, OutTerrainMesh);
	}

	if (NEIndex >= 0) // NE Edge
	{
		GenerateHexBorder(InCellData, EHexDirection::NE, OutTerrainMesh);
	}
	
	if (WIndex >= 0 && NWIndex >= 0) // NW Corner
	{
		GenerateHexCorner(InCellData, EHexDirection::NW, OutTerrainMesh);
	}

	if (NWIndex >= 0 && NEIndex >= 0) // N Corner
	{
		GenerateHexCorner(InCellData, EHexDirection::NE, OutTerrainMesh);
	}
}

void AHexTerrainGenerator::GenerateHexCenter(const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh)
{
	switch (InCellData.HexRiver.RiverState)
	{
	case EHexRiverState::None:
		GenerateNoRiverCenter(InCellData, OutTerrainMesh);
		break;

	case EHexRiverState::StartPoint:
	case EHexRiverState::EndPoint:
		GenerateCenterWithRiverEnd(InCellData, OutTerrainMesh);
		break;

	case EHexRiverState::PassThrough:
		GenerateCenterWithRiverThrough(InCellData, OutTerrainMesh);
		break;
	}
}

void AHexTerrainGenerator::GenerateHexBorder(const FHexCellData& InCellData, EHexDirection BorderDirection, FCachedChunkData& OutTerrainMesh)
{
	uint8 BorderDirectionId = static_cast<uint8>(BorderDirection);
	const FHexCellBorder& HexBorder = InCellData.HexNeighbors[BorderDirectionId];
	const FHexCellData& OppositeCell = HexGrids[HexBorder.LinkedCellIndex];
	
	TArray<FHexVertexData> FromVerts;
	TArray<FHexVertexData> ToVerts;

	FromVerts.Add(CalcHexCellVertex(InCellData, HexBorder.FromVert.X, false));
	ToVerts.Add(CalcHexCellVertex(OppositeCell, HexBorder.ToVert.X, false));
	
	for (int32 SubIndex = 0; SubIndex < HexCellSubdivision; ++SubIndex)
	{
		int32 SubVertIndex = HexBorder.FromVert.X * HexCellSubdivision + SubIndex;
		FromVerts.Add(CalcHexCellVertex(InCellData, SubVertIndex, true));

		SubVertIndex = HexBorder.ToVert.Y * HexCellSubdivision + (HexCellSubdivision - SubIndex - 1);
		ToVerts.Add(CalcHexCellVertex(OppositeCell, SubVertIndex, true));
	}

	FromVerts.Add(CalcHexCellVertex(InCellData, HexBorder.FromVert.Y, false));
	ToVerts.Add(CalcHexCellVertex(OppositeCell, HexBorder.ToVert.Y, false));
	
	int32 NumOfVerts = FromVerts.Num();
	int32 NumOfZSteps = FMath::Abs(OppositeCell.Elevation - InCellData.Elevation);
	int32 NumOfSegments = NumOfVerts - 1;
	int32 MidIndex = NumOfSegments / 2;
	bool bLowToHigh = InCellData.Elevation <= OppositeCell.Elevation;
	if (HexBorder.LinkState == EHexBorderState::Terrace)
	{
		for (int32 Index = 0; Index < NumOfSegments; ++Index)
		{
			bool bRotTriangle = bLowToHigh ? Index >= MidIndex : Index < MidIndex;
			FillStrip(FromVerts[Index], FromVerts[Index + 1], ToVerts[Index], ToVerts[Index + 1], OutTerrainMesh.GroundSection, NumOfZSteps, true, bRotTriangle);
		}
	}
	else
	{
		for (int32 Index = 0; Index < NumOfSegments; ++Index)
		{
			bool bRotTriangle = bLowToHigh ? Index >= MidIndex : Index < MidIndex;
			FillQuad(FromVerts[Index], FromVerts[Index + 1], ToVerts[Index], ToVerts[Index + 1], OutTerrainMesh.GroundSection, bRotTriangle);
		}
	}
	
	int32 RiverIndex = -1;
	int32 RoadIndex = -1;
	for (int32 Index = 0; Index < NumOfVerts; ++Index)
	{
		if (FromVerts[Index].VertexState == 1u)
		{
			RiverIndex = Index;
			break;
		}
		else if (FromVerts[Index].VertexState == 2u)
		{
			RoadIndex = Index;
			break;
		}
	}

	// River
	int32 FromWaterDepth = InCellData.GetWaterDepth();
	int32 ToWaterDepth = OppositeCell.GetWaterDepth();
	if (RiverIndex >= 0 && (FromWaterDepth <= 0 || ToWaterDepth <= 0))
	{
		FVector RiverZOffset = CalcWaterVertOffset();
		const FColor& WaterColor = ConfigData.ColorsMap[EHexTerrainType::Water];

		float UVScale = CalcRiverUVScale(true, NumOfZSteps);
		bool bHasOutRiver = BorderDirection == InCellData.HexRiver.OutgoingDirection;
		FVector2D UV0s[4] = { FVector2D{ 0.0, 0.0 }, FVector2D{ 0.0, 1.0 }, FVector2D{ UVScale, 0.0 }, FVector2D{ UVScale, 1.0 } };

		FHexVertexData FromEdgeL = FromVerts[RiverIndex - 1].ApplyOverride(RiverZOffset, &WaterColor, &UV0s[bHasOutRiver ? 0 : 3]);
		FHexVertexData FromEdgeR = FromVerts[RiverIndex + 1].ApplyOverride(RiverZOffset, &WaterColor, &UV0s[bHasOutRiver ? 1 : 2]);
		FHexVertexData ToEdgeL = ToVerts[RiverIndex - 1].ApplyOverride(RiverZOffset, &WaterColor, &UV0s[bHasOutRiver ? 2 : 1]);
		FHexVertexData ToEdgeR = ToVerts[RiverIndex + 1].ApplyOverride(RiverZOffset, &WaterColor, &UV0s[bHasOutRiver ? 3 : 0]);

		bool bShouldSkip = false;
		if (FromWaterDepth > 0)
		{
			double TargetWaterZ = CalcWaterVertOffset(FromWaterDepth).Z - RiverZOffset.Z;
			double Ratio = TargetWaterZ / (ToEdgeL.Position.Z - FromEdgeL.Position.Z);

			if (Ratio > 0.0 && Ratio < 1.0)
			{
				Ratio += RiverZOffset.Z / (ToEdgeL.Position.Z - FromEdgeL.Position.Z);
				FromEdgeL = FHexVertexData::LerpVertex(FromEdgeL, ToEdgeL, FVector::OneVector * Ratio, Ratio);
				FromEdgeR = FHexVertexData::LerpVertex(FromEdgeR, ToEdgeR, FVector::OneVector * Ratio, Ratio);
			}
			else if (Ratio >= 1.0)
				bShouldSkip = true;
		}
		else if (ToWaterDepth > 0)
		{
			double TargetWaterZ = CalcWaterVertOffset(ToWaterDepth).Z - RiverZOffset.Z;
			double Ratio = TargetWaterZ / (FromEdgeL.Position.Z - ToEdgeL.Position.Z);

			if (Ratio > 0.0 && Ratio < 1.0)
			{
				Ratio += RiverZOffset.Z / (FromEdgeL.Position.Z - ToEdgeL.Position.Z);
				ToEdgeL = FHexVertexData::LerpVertex(ToEdgeL, FromEdgeL, FVector::OneVector * Ratio, Ratio);
				ToEdgeR = FHexVertexData::LerpVertex(ToEdgeR, FromEdgeR, FVector::OneVector * Ratio, Ratio);
			}
			else if (Ratio >= 1.0)
				bShouldSkip = true;
		}

		if (!bShouldSkip)
			FillQuad(FromEdgeL, FromEdgeR, ToEdgeL, ToEdgeR, OutTerrainMesh.RiverSection);
	}
	
	// Road
	if (RoadIndex >= 0)
	{
		FVector RoadZOffset = CalcRoadVertOffset();
		const FColor& RoadColor = ConfigData.ColorsMap[EHexTerrainType::Road];

		FVector2D UV0s[4] = { FVector2D{ 0.0, 0.0 }, FVector2D{ 1.0, 0.0 }, FVector2D{ 0.0, 1.0 }, FVector2D{ 1.0, 1.0 } };

		FHexVertexData RoadFromEdgeL = FHexVertexData::LerpVertex(FromVerts[RoadIndex], FromVerts[RoadIndex - 2], FVector::OneVector * RoadWidthRatio, RoadWidthRatio);
		FHexVertexData RoadFromEdgeM = FromVerts[RoadIndex].ApplyOverride(RoadZOffset, &RoadColor, &UV0s[0]);
		FHexVertexData RoadFromEdgeR = FHexVertexData::LerpVertex(FromVerts[RoadIndex], FromVerts[RoadIndex + 2], FVector::OneVector * RoadWidthRatio, RoadWidthRatio);
		FHexVertexData RoadToEdgeL = FHexVertexData::LerpVertex(ToVerts[RoadIndex], ToVerts[RoadIndex - 2], FVector::OneVector * RoadWidthRatio, RoadWidthRatio);
		FHexVertexData RoadToEdgeM = ToVerts[RoadIndex].ApplyOverride(RoadZOffset, &RoadColor, &UV0s[2]);
		FHexVertexData RoadToEdgeR = FHexVertexData::LerpVertex(ToVerts[RoadIndex], ToVerts[RoadIndex + 2], FVector::OneVector * RoadWidthRatio, RoadWidthRatio);

		RoadFromEdgeL.ApplyOverrideInline(RoadZOffset, &RoadColor, &UV0s[1]);
		RoadFromEdgeR.ApplyOverrideInline(RoadZOffset, &RoadColor, &UV0s[1]);
		RoadToEdgeL.ApplyOverrideInline(RoadZOffset, &RoadColor, &UV0s[3]);
		RoadToEdgeR.ApplyOverrideInline(RoadZOffset, &RoadColor, &UV0s[3]);

		if (HexBorder.LinkState == EHexBorderState::Terrace)
		{
			FillStrip(RoadFromEdgeL, RoadFromEdgeM, RoadToEdgeL, RoadToEdgeM, OutTerrainMesh.RoadSection, NumOfZSteps, true, !bLowToHigh);
			FillStrip(RoadFromEdgeM, RoadFromEdgeR, RoadToEdgeM, RoadToEdgeR, OutTerrainMesh.RoadSection, NumOfZSteps, true, bLowToHigh);
		}
		else
		{
			FillQuad(RoadFromEdgeL, RoadFromEdgeM, RoadToEdgeL, RoadToEdgeM, OutTerrainMesh.RoadSection, !bLowToHigh);
			FillQuad(RoadFromEdgeM, RoadFromEdgeR, RoadToEdgeM, RoadToEdgeR, OutTerrainMesh.RoadSection, bLowToHigh);
		}
	}
}

void AHexTerrainGenerator::GenerateHexCorner(const FHexCellData& InCellData, EHexDirection CornerDirection, FCachedChunkData& OutTerrainMesh)
{
	int32 CIndex = static_cast<uint8>(CornerDirection) - 4;
	const FHexCellCorner& CornerData = InCellData.HexCorners[CIndex];

	int32 NumOfTerraces = 0;	
	if (CornerData.LinkState[0] == EHexBorderState::Terrace)
		++NumOfTerraces;
	if (CornerData.LinkState[1] == EHexBorderState::Terrace)
		++NumOfTerraces;
	if (CornerData.LinkState[2] == EHexBorderState::Terrace)
		++NumOfTerraces;

	const FHexCellData& Cell1 = HexGrids[CornerData.LinkedCellsIndex.X];
	const FHexCellData& Cell2 = HexGrids[CornerData.LinkedCellsIndex.Y];
	const FHexCellData& Cell3 = HexGrids[CornerData.LinkedCellsIndex.Z];
	
	if (NumOfTerraces == 0)
		GenerateNoTerraceCorner(Cell1, Cell2, Cell3, CornerData, OutTerrainMesh);
	else
		GenerateCornerWithTerrace(Cell1, Cell2, Cell3, CornerData, OutTerrainMesh);	
}

void AHexTerrainGenerator::GenerateNoRiverCenter(const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh)
{
	TArray<FHexVertexData> EdgesV;
	for (int32 EdgeIndex = 0; EdgeIndex < CORNER_NUM; ++EdgeIndex)
	{
		for (int32 SubIndex = 0; SubIndex <= HexCellSubdivision; ++SubIndex)
		{
			if (SubIndex == 0)
				EdgesV.Add(CalcHexCellVertex(InCellData, EdgeIndex, false, 0u, true));
			else
				EdgesV.Add(CalcHexCellVertex(InCellData, EdgeIndex * HexCellSubdivision + SubIndex - 1, true, 0u, true));
		}
	}

	FHexVertexData CenterV{ InCellData.CellCenter, InCellData.SRGBColor, FVector::UpVector };

	TArray<bool> Dummy;
	FillFan(CenterV, EdgesV, Dummy, OutTerrainMesh.GroundSection, true);

	// Road
	GenerateRoadCenter(CenterV, EdgesV, OutTerrainMesh);

	// Features
	for (int32 EdgeIndex = 0; EdgeIndex < CORNER_NUM; ++EdgeIndex)
	{
		CalcFeatureTransform(InCellData, InCellData.CellCenter, EdgeIndex, OutTerrainMesh.FeatureTransforms);
	}
	CalcFeatureTransform(InCellData, InCellData.CellCenter, -1, OutTerrainMesh.FeatureTransforms);
}

void AHexTerrainGenerator::GenerateCenterWithRiverEnd(const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh)
{
	TArray<FHexVertexData> EdgesV;
	for (int32 EdgeIndex = 0; EdgeIndex < CORNER_NUM; ++EdgeIndex)
	{
		for (int32 SubIndex = 0; SubIndex <= HexCellSubdivision; ++SubIndex)
		{
			if (SubIndex == 0)
				EdgesV.Add(CalcHexCellVertex(InCellData, EdgeIndex, false));
			else
				EdgesV.Add(CalcHexCellVertex(InCellData, EdgeIndex * HexCellSubdivision + SubIndex - 1, true));
		}
	}
	
	int32 NumOfEdges = EdgesV.Num();
	FHexVertexData CenterV{ InCellData.CellCenter, InCellData.SRGBColor };

	TArray<FHexVertexData> CentersV;
	CentersV.Init(CenterV, NumOfEdges);

	FillGrid(CentersV, EdgesV, OutTerrainMesh.GroundSection, RiverSubdivision, false, true);
	
	int32 RiverIndex = -1;
	for (int32 Index = 0; Index < NumOfEdges; ++Index)
	{
		if (EdgesV[Index].VertexState == 1u)
		{
			RiverIndex = Index;
			break;
		}
	}

	// River
	if (RiverIndex >= 0 && InCellData.GetWaterDepth() <= 0)
	{
		FVector WaterZOffset = CalcWaterVertOffset();
		const FColor& WaterColor = ConfigData.ColorsMap[EHexTerrainType::Water];

		FHexVertexData CopiedEdgeL = EdgesV[RiverIndex - 1].ApplyOverride(WaterZOffset, &WaterColor);
		FHexVertexData CopiedEdgeR = EdgesV[RiverIndex + 1].ApplyOverride(WaterZOffset, &WaterColor);

		float UVScale = CalcRiverUVScale();

		if (InCellData.HexRiver.RiverState == EHexRiverState::StartPoint)
		{
			CenterV.SetUV0(FVector2D{ 0.0, 0.5 });
			CopiedEdgeL.SetUV0(FVector2D{ UVScale, 0.0 });
			CopiedEdgeR.SetUV0(FVector2D{ UVScale, 1.0 });
		}
		else
		{
			CenterV.SetUV0(FVector2D{ UVScale, 0.5 });
			CopiedEdgeL.SetUV0(FVector2D{ 0.0, 1.0 });
			CopiedEdgeR.SetUV0(FVector2D{ 0.0, 0.0 });
		}

		FillQuad(CenterV, CenterV, CopiedEdgeL, CopiedEdgeR, OutTerrainMesh.RiverSection);
	}

	// Road
	bool bHasRoadVert = GenerateRoadCenter(CenterV, EdgesV, OutTerrainMesh);
	
	if (bHasRoadVert && RiverIndex >= 0)
	{
		TArray<FHexVertexData> EdgesLV = { EdgesV[RiverIndex - 2], EdgesV[RiverIndex - 1] };
		TArray<FHexVertexData> EdgesRV = { EdgesV[RiverIndex + 1], EdgesV[(RiverIndex + 2) % NumOfEdges] };
		GenerateRoadCenterWithRiver(InCellData, CenterV, EdgesLV, OutTerrainMesh);
		GenerateRoadCenterWithRiver(InCellData, CenterV, EdgesRV, OutTerrainMesh);
	}

	// Features
	for (int32 EdgeIndex = 0; EdgeIndex < CORNER_NUM; ++EdgeIndex)
	{
		CalcFeatureTransform(InCellData, InCellData.CellCenter, EdgeIndex, OutTerrainMesh.FeatureTransforms);
	}
}

void AHexTerrainGenerator::GenerateCenterWithRiverThrough(const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh)
{
	EHexDirection InDirection = InCellData.HexRiver.IncomingDirection;
	EHexDirection OutDirection = InCellData.HexRiver.OutgoingDirection;

	uint8 InMVertId = FHexCellData::GetVertIdFromDirection(InDirection, true, 1u);
	uint8 OutMVertId = FHexCellData::GetVertIdFromDirection(OutDirection, true, 1u);

	FVector InDir = FHexCellData::HexSubVertices[InMVertId];
	FVector OutDir = FHexCellData::HexSubVertices[OutMVertId];

	FVector LeftDir;
	FVector RightDir;
	
	if (FHexCellData::CalcOppositeDirection(InDirection) == OutDirection)
	{
		OutDir.Normalize();

		LeftDir = FVector{ OutDir.Y, -OutDir.X , OutDir.Z }; // L90
		RightDir = FVector{ -OutDir.Y, OutDir.X , OutDir.Z }; // R90
	}
	else
	{
		FVector MidDir = (InDir + OutDir).GetSafeNormal();

		bool bRSign = FVector::CrossProduct(OutDir, MidDir).Z > 0.0;

		LeftDir = bRSign ? FVector::ZeroVector : MidDir * 2.0;
		RightDir = bRSign ? MidDir * 2.0 : FVector::ZeroVector;
	}

	FColor WaterColor = ConfigData.ColorsMap[EHexTerrainType::Water];
	float MoveDist = HexCellRadius / float(HexCellSubdivision + 1);
	FVector RiverOffset = InCellData.GetWaterDepth() > 0 ? FVector::ZeroVector : CalcRiverVertOffset();

	FHexVertexData CenterL = FHexVertexData{ InCellData.CellCenter + LeftDir * MoveDist, InCellData.SRGBColor };
	FHexVertexData Center = FHexVertexData{ InCellData.CellCenter + (LeftDir + RightDir) * 0.5 * MoveDist + RiverOffset, WaterColor };
	FHexVertexData CenterR = FHexVertexData{ InCellData.CellCenter + RightDir * MoveDist, InCellData.SRGBColor };
	
	auto GenerateFansWithoutRiver = [this](const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh,
		EHexDirection FromDirection, EHexDirection ToDirection, const FHexVertexData& InCenter) -> bool
		{
			TArray<FHexVertexData> EdgesV;

			EHexDirection CurDirection = FHexCellData::CalcNextDirection(FromDirection);
			while (CurDirection != ToDirection)
			{
				uint8 EdgeIndex = FHexCellData::GetVertIdFromDirection(CurDirection, false);
				for (int32 SubIndex = 0; SubIndex <= HexCellSubdivision; ++SubIndex)
				{
					if (SubIndex == 0)
						EdgesV.Add(CalcHexCellVertex(InCellData, EdgeIndex, false));
					else
						EdgesV.Add(CalcHexCellVertex(InCellData, EdgeIndex * HexCellSubdivision + SubIndex - 1, true));
				}

				CurDirection = FHexCellData::CalcNextDirection(CurDirection);
			}
			{
				uint8 EdgeIndex = FHexCellData::GetVertIdFromDirection(CurDirection, false);
				EdgesV.Add(CalcHexCellVertex(InCellData, EdgeIndex, false));
			}

			TArray<FHexVertexData> CentersV;
			CentersV.Init(InCenter, EdgesV.Num());

			FillGrid(CentersV, EdgesV, OutTerrainMesh.GroundSection, RiverSubdivision, false, false);
			
			// Road
			bool bHasRoadVert = GenerateRoadCenter(InCenter, EdgesV, OutTerrainMesh);

			// Features
			CurDirection = FHexCellData::CalcNextDirection(FromDirection);
			while (CurDirection != ToDirection)
			{
				CalcFeatureTransform(InCellData, InCenter.Position, static_cast<uint8>(CurDirection), OutTerrainMesh.FeatureTransforms);

				CurDirection = FHexCellData::CalcNextDirection(CurDirection);
			}

			return bHasRoadVert;
		};

	bool bHasRoadVertL = GenerateFansWithoutRiver(InCellData, OutTerrainMesh, OutDirection, InDirection, CenterR);
	bool bHasRoadVertR = GenerateFansWithoutRiver(InCellData, OutTerrainMesh, InDirection, OutDirection, CenterL);

	bool bSharpLeftTurn = OutDirection == FHexCellData::CalcNextDirection(InDirection);
	bool bSharpRightTurn = OutDirection == FHexCellData::CalcPreviousDirection(InDirection);

	auto GenerateRiverFan = [this, WaterColor](const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh,
		const FHexVertexData& InCenterL, const FHexVertexData& InCenter, const FHexVertexData& InCenterR, EHexDirection Direction, 
		bool bHasRoadVertL, bool bHasRoadVertR) -> void
		{
			uint8 SVertId = FHexCellData::GetVertIdFromDirection(Direction, true, 0u);
			uint8 MVertId = FHexCellData::GetVertIdFromDirection(Direction, true, 1u);
			uint8 EVertId = FHexCellData::GetVertIdFromDirection(Direction, true, 2u);

			// Left Fan
			TArray<FHexVertexData> EdgesLV;
			uint8 MainVertL = FHexCellData::GetVertIdFromDirection(Direction, false);
			EdgesLV.Add(CalcHexCellVertex(InCellData, MainVertL, false));
			for (uint8 Index = SVertId; Index <= MVertId - 1u; ++Index)
			{
				EdgesLV.Add(CalcHexCellVertex(InCellData, Index, true));
			}
			
			TArray<FHexVertexData> CentersLV;
			CentersLV.Init(InCenterL, EdgesLV.Num());
			
			FillGrid(CentersLV, EdgesLV, OutTerrainMesh.GroundSection, RiverSubdivision);

			// Center Two Quads
			FHexVertexData EdgeL = CalcHexCellVertex(InCellData, MVertId - 1u, true);
			FHexVertexData EdgeC = CalcHexCellVertex(InCellData, MVertId, true);
			FHexVertexData EdgeR = CalcHexCellVertex(InCellData, MVertId + 1u, true);

			FillStrip(InCenterL, InCenter, EdgeL, EdgeC, OutTerrainMesh.GroundSection, RiverSubdivision);
			FillStrip(InCenter, InCenterR, EdgeC, EdgeR, OutTerrainMesh.GroundSection, RiverSubdivision);

			// Right Fan
			TArray<FHexVertexData> EdgesRV;
			for (uint8 Index = MVertId + 1u; Index <= EVertId; ++Index)
			{
				EdgesRV.Add(CalcHexCellVertex(InCellData, Index, true));
			}
			uint8 MainVertR = FHexCellData::GetVertIdFromDirection(FHexCellData::CalcNextDirection(Direction), false);
			EdgesRV.Add(CalcHexCellVertex(InCellData, MainVertR, false));

			TArray<FHexVertexData> CentersRV;
			CentersRV.Init(InCenterR, EdgesRV.Num());

			FillGrid(CentersRV, EdgesRV, OutTerrainMesh.GroundSection, RiverSubdivision);

			// River
			if (InCellData.GetWaterDepth() <= 0)
			{
				FVector WaterZOffset = CalcWaterVertOffset();

				float UVScale = CalcRiverUVScale();
				bool bHasOutRiver = Direction == InCellData.HexRiver.OutgoingDirection;
				FVector2D UV0s[4] = { FVector2D{ 0.0, 0.0 }, FVector2D{ 0.0, 1.0 }, FVector2D{ UVScale, 0.0 }, FVector2D{ UVScale, 1.0 } };

				FHexVertexData CopiedCenterL = InCenterL.ApplyOverride(WaterZOffset, &WaterColor, &UV0s[bHasOutRiver ? 0 : 3]);
				FHexVertexData CopiedCenterR = InCenterR.ApplyOverride(WaterZOffset, &WaterColor, &UV0s[bHasOutRiver ? 1 : 2]);
				EdgeL.ApplyOverrideInline(WaterZOffset, &WaterColor, &UV0s[bHasOutRiver ? 2 : 1]);
				EdgeR.ApplyOverrideInline(WaterZOffset, &WaterColor, &UV0s[bHasOutRiver ? 3 : 0]);

				FillStrip(CopiedCenterL, CopiedCenterR, EdgeL, EdgeR, OutTerrainMesh.RiverSection, RiverSubdivision);
			}

			// Road
			if (bHasRoadVertL)
				GenerateRoadCenterWithRiver(InCellData, InCenterL, EdgesLV, OutTerrainMesh);
			if (bHasRoadVertR)
				GenerateRoadCenterWithRiver(InCellData, InCenterR, EdgesRV, OutTerrainMesh);
		};
	
	auto GenerateSharpRiverFan = [this, WaterColor](const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh,
		const FHexVertexData& InCenterL, const FHexVertexData& InCenter, const FHexVertexData& InCenterR, EHexDirection Direction, bool bMoveLeft) -> void
		{
			uint8 SVertId = FHexCellData::GetVertIdFromDirection(Direction, true, 0u);
			uint8 MVertId = FHexCellData::GetVertIdFromDirection(Direction, true, 1u);
			uint8 EVertId = FHexCellData::GetVertIdFromDirection(Direction, true, 2u);

			// Left Fan
			TArray<FHexVertexData> EgdesLV;
			uint8 MainVertL = FHexCellData::GetVertIdFromDirection(Direction, false);
			EgdesLV.Add(CalcHexCellVertex(InCellData, MainVertL, false));
			for (uint8 Index = SVertId; Index <= MVertId - 2u; ++Index)
			{
				EgdesLV.Add(CalcHexCellVertex(InCellData, Index, true));
			}

			TArray<FHexVertexData> CentersLV;
			CentersLV.Init(InCenterL, EgdesLV.Num());

			FillGrid(CentersLV, EgdesLV, OutTerrainMesh.GroundSection, RiverSubdivision);

			// Center Two Quads
			uint8 MainVertIdL = FHexCellData::GetVertIdFromDirection(Direction, false);
			uint8 MainVertIdR = FHexCellData::GetVertIdFromDirection(FHexCellData::CalcNextDirection(Direction), false);

			FHexVertexData EdgeL2 = MVertId - 2u < SVertId ? CalcHexCellVertex(InCellData, MainVertIdL, false) : CalcHexCellVertex(InCellData, MVertId - 2u, true);
			FHexVertexData EdgeL1 = CalcHexCellVertex(InCellData, MVertId - 1u, true);
			FHexVertexData EdgeC = CalcHexCellVertex(InCellData, MVertId, true);
			FHexVertexData EdgeR1 = CalcHexCellVertex(InCellData, MVertId + 1u, true);
			FHexVertexData EdgeR2 = MVertId + 2u > EVertId ? CalcHexCellVertex(InCellData, MainVertIdR, false) : CalcHexCellVertex(InCellData, MVertId + 2u, true);

			FVector EdgeCNoOffset = EdgeC.Position - CalcRiverVertOffset();
			FVector EdgeL1WithOffset = EdgeL1.Position + CalcRiverVertOffset();
			FVector EdgeR1WithOffset = EdgeR1.Position + CalcRiverVertOffset();
			FHexVertexData MidL{ bMoveLeft ? (InCenterL.Position + EdgeL2.Position) * 0.5 : (InCenterL.Position + EdgeCNoOffset) * 0.5, InCellData.SRGBColor };
			FHexVertexData MidC{ bMoveLeft ? (InCenter.Position + EdgeL1WithOffset) * 0.5 : (InCenter.Position + EdgeR1WithOffset) * 0.5, WaterColor };
			FHexVertexData MidR{ bMoveLeft ? (InCenterR.Position + EdgeCNoOffset) * 0.5 : (InCenterR.Position + EdgeR2.Position) * 0.5, InCellData.SRGBColor };
			FHexVertexData MidE{ bMoveLeft ? (InCenterR.Position + EdgeR2.Position) * 0.5 : (InCenterL.Position + EdgeL2.Position) * 0.5, InCellData.SRGBColor };

			FHexVertexData CenterL = InCenterL;
			FHexVertexData Center = InCenter;
			FHexVertexData CenterR = InCenterR;

			if (bMoveLeft)
			{
				FillQuad(MidL, MidL, EdgeL2, EdgeL1, OutTerrainMesh.GroundSection);
			}
			else
			{
				FillQuad(CenterL, CenterL, MidE, MidL, OutTerrainMesh.GroundSection);
				FillQuad(MidE, MidL, EdgeL2, EdgeL1, OutTerrainMesh.GroundSection);
			}

			FillQuad(CenterL, Center, MidL, MidC, OutTerrainMesh.GroundSection);
			FillQuad(MidL, MidC, EdgeL1, EdgeC, OutTerrainMesh.GroundSection);

			FillQuad(Center, CenterR, MidC, MidR, OutTerrainMesh.GroundSection);
			FillQuad(MidC, MidR, EdgeC, EdgeR1, OutTerrainMesh.GroundSection);

			if (bMoveLeft)
			{
				FillQuad(CenterR, CenterR, MidR, MidE, OutTerrainMesh.GroundSection);
				FillQuad(MidR, MidE, EdgeR1, EdgeR2, OutTerrainMesh.GroundSection);
			}
			else
			{
				FillQuad(MidR, MidR, EdgeR1, EdgeR2, OutTerrainMesh.GroundSection);
			}

			// Right Fan
			TArray<FHexVertexData> EgdesRV;
			for (uint8 Index = MVertId + 2u; Index <= EVertId; ++Index)
			{
				EgdesRV.Add(CalcHexCellVertex(InCellData, Index, true));
			}
			uint8 MainVertR = FHexCellData::GetVertIdFromDirection(FHexCellData::CalcNextDirection(Direction), false);
			EgdesRV.Add(CalcHexCellVertex(InCellData, MainVertR, false));

			TArray<FHexVertexData> CentersRV;
			CentersRV.Init(InCenterR, EgdesRV.Num());

			FillGrid(CentersRV, EgdesRV, OutTerrainMesh.GroundSection, RiverSubdivision);

			// River
			if (InCellData.GetWaterDepth() <= 0)
			{
				FVector WaterZOffset = CalcWaterVertOffset();
				bool bHasOutRiver = Direction == InCellData.HexRiver.OutgoingDirection;
				float UVScale = CalcRiverUVScale();
				FVector2D UV0s[6] = { FVector2D{ 0.0, 0.0 }, FVector2D{ 0.0, 1.0 }, FVector2D{ 0.5 * UVScale, 0.0 }, FVector2D{ 0.5 * UVScale, 1.0 },FVector2D{ UVScale, 0.0 },FVector2D{ UVScale, 1.0 } };

				CenterL.ApplyOverrideInline(WaterZOffset, &WaterColor, &UV0s[bHasOutRiver ? 0 : 5]);
				CenterR.ApplyOverrideInline(WaterZOffset, &WaterColor, &UV0s[bHasOutRiver ? 1 : 4]);
				MidL.ApplyOverrideInline(WaterZOffset, &WaterColor, &UV0s[bHasOutRiver ? 2 : 3]);
				MidR.ApplyOverrideInline(WaterZOffset, &WaterColor, &UV0s[bHasOutRiver ? 3 : 2]);
				EdgeL1.ApplyOverrideInline(WaterZOffset, &WaterColor, &UV0s[bHasOutRiver ? 4 : 1]);
				EdgeR1.ApplyOverrideInline(WaterZOffset, &WaterColor, &UV0s[bHasOutRiver ? 5 : 0]);

				FillQuad(CenterL, CenterR, MidL, MidR, OutTerrainMesh.RiverSection);
				FillQuad(MidL, MidR, EdgeL1, EdgeR1, OutTerrainMesh.RiverSection);
			}
		};

	if (bSharpLeftTurn || bSharpRightTurn)
	{
		GenerateSharpRiverFan(InCellData, OutTerrainMesh, CenterL, Center, CenterR, OutDirection, bSharpRightTurn);
		GenerateSharpRiverFan(InCellData, OutTerrainMesh, CenterR, Center, CenterL, InDirection, !bSharpRightTurn);
	}
	else
	{
		GenerateRiverFan(InCellData, OutTerrainMesh, CenterL, Center, CenterR, OutDirection, bHasRoadVertR, bHasRoadVertL);
		GenerateRiverFan(InCellData, OutTerrainMesh, CenterR, Center, CenterL, InDirection, bHasRoadVertL, bHasRoadVertR);
	}
}

bool AHexTerrainGenerator::GenerateRoadCenter(const FHexVertexData& CenterV, const TArray<FHexVertexData>& EdgesV, FCachedChunkData& OutTerrainMesh)
{
	FVector RoadZOffset = CalcRoadVertOffset();
	const FColor& RoadColor = ConfigData.ColorsMap[EHexTerrainType::Road];
	FVector2D UV0s[5] = { FVector2D{ 0.0, 0.0 }, FVector2D{ 1.0, 0.5 }, FVector2D{ 0.0, 1.0 }, FVector2D{ 1.0, 1.0 }, FVector2D{ 1.0, 0.0 } };

	FHexVertexData RoadCenterV = CenterV.ApplyOverride(RoadZOffset, &RoadColor, &UV0s[0]);
	TArray<FHexVertexData> RoadCentersV;
	RoadCentersV.Init(RoadCenterV, 3);

	TArray<int32> RoadVertIndices;

	int32 NumOfVerts = EdgesV.Num();
	for (int32 Index = 0; Index < NumOfVerts; ++Index)
	{
		if (EdgesV[Index].VertexState == 2u)
		{
			const FHexVertexData& EdgeL2Vert = EdgesV[Index - 2];
			const FHexVertexData& EdgeR2Vert = EdgesV[(Index + 2) % NumOfVerts];

			FHexVertexData RoadEdgeLVert = FHexVertexData::LerpVertex(EdgesV[Index], EdgeL2Vert, FVector::OneVector * RoadWidthRatio, RoadWidthRatio);
			FHexVertexData RoadEdgeRVert = FHexVertexData::LerpVertex(EdgesV[Index], EdgeR2Vert, FVector::OneVector * RoadWidthRatio, RoadWidthRatio);
			FHexVertexData RoadEdgeMVert = EdgesV[Index].ApplyOverride(RoadZOffset, &RoadColor, &UV0s[2]);

			RoadEdgeLVert.ApplyOverrideInline(RoadZOffset, &RoadColor, &UV0s[3]);
			RoadEdgeRVert.ApplyOverrideInline(RoadZOffset, &RoadColor, &UV0s[3]);

			FHexVertexData RoadMidLVert = FHexVertexData::LerpVertex(CenterV, EdgeL2Vert, FVector::OneVector * RoadWidthRatio, RoadWidthRatio);
			FHexVertexData RoadMidMVert = FHexVertexData::LerpVertex(RoadCenterV, RoadEdgeMVert, FVector::OneVector * RoadWidthRatio, RoadWidthRatio);
			FHexVertexData RoadMidRVert = FHexVertexData::LerpVertex(CenterV, EdgeR2Vert, FVector::OneVector * RoadWidthRatio, RoadWidthRatio);

			RoadMidLVert.ApplyOverrideInline(RoadZOffset, &RoadColor, &UV0s[1]);
			RoadMidRVert.ApplyOverrideInline(RoadZOffset, &RoadColor, &UV0s[1]);

			TArray<FHexVertexData> RoadEdgeV = { RoadEdgeLVert, RoadEdgeMVert, RoadEdgeRVert };
			TArray<FHexVertexData> RoadMidV = { RoadMidLVert, RoadMidMVert, RoadMidRVert };

			FillGrid(RoadCentersV, RoadMidV, OutTerrainMesh.RoadSection, 1);
			FillGrid(RoadMidV, RoadEdgeV, OutTerrainMesh.RoadSection, 1);

			RoadVertIndices.Add(EdgesV[Index].VertexIndex);
		}
	}

	bool bHasRoadVert = RoadVertIndices.Num() > 0;
	if (bHasRoadVert)
	{
		RoadCentersV.Init(RoadCenterV, HexCellSubdivision + 2);

		int32 NumOfEdges = NumOfVerts / (HexCellSubdivision + 1);
		for (int32 Index = 0; Index < NumOfEdges; ++Index)
		{
			bool bGroundEdge = true;
			for (int32 SubIndex = 1; SubIndex <= HexCellSubdivision; ++SubIndex)
			{
				int32 VertIndex = Index * (HexCellSubdivision + 1) + SubIndex;
				bGroundEdge = bGroundEdge && (EdgesV[VertIndex].VertexState == 0u);
			}

			if (bGroundEdge)
			{
				TArray<FHexVertexData> OneEdgeV;
				for (int32 SubIndex = 0; SubIndex <= (HexCellSubdivision + 1); ++SubIndex)
				{
					int32 VertIndex = (Index * (HexCellSubdivision + 1) + SubIndex) % NumOfVerts;
					//float RoadWidthScale = 1.0f - FMath::Clamp(CalcDiffToRoadVert(RoadVertIndices, EdgesV[VertIndex].VertexIndex) - 2, 0, 4) * 0.125f;
					float RoadWidthScale = CalcRoadWidthScale(CalcDiffToRoadVert(RoadVertIndices, EdgesV[VertIndex].VertexIndex));
					float ScaledRoadWidthRatio = RoadWidthRatio * RoadWidthScale;

					FHexVertexData RoadMidVert = FHexVertexData::LerpVertex(CenterV, EdgesV[VertIndex], FVector::OneVector * ScaledRoadWidthRatio, ScaledRoadWidthRatio);
					RoadMidVert.ApplyOverrideInline(RoadZOffset, &RoadColor, &UV0s[1]);

					OneEdgeV.Add(RoadMidVert);
				}

				FillGrid(RoadCentersV, OneEdgeV, OutTerrainMesh.RoadSection, 1);
			}
		}
	}
	return bHasRoadVert;
}

void AHexTerrainGenerator::GenerateRoadCenterWithRiver(const FHexCellData& InCellData, const FHexVertexData& CenterV, const TArray<FHexVertexData>& EdgesV, FCachedChunkData& OutTerrainMesh)
{
	FVector RoadZOffset = CalcRoadVertOffset();
	const FColor& RoadColor = ConfigData.ColorsMap[EHexTerrainType::Road];
	FVector2D UV0s[2] = { FVector2D{ 0.0, 0.0 }, FVector2D{ 1.0, 0.5 } };

	TArray<int32> RoadVertIndices;
	for (int32 VertIndex = 0; VertIndex < CORNER_NUM; ++VertIndex)
	{
		int32 EdgeIndex = FHexCellData::CalcNextDirection(VertIndex);
		if (InCellData.HexRoad.RoadState[EdgeIndex])
		{
			uint8 SubVertIndex = FHexCellData::GetVertIdFromDirection(static_cast<EHexDirection>(EdgeIndex));
			int32 RoadMidVert = SubVertIndex + VertIndex + 1;
			RoadVertIndices.Add(RoadMidVert);
		}
	}
	
	int32 NumOfVerts = EdgesV.Num();
	FHexVertexData RoadCenterV = CenterV.ApplyOverride(RoadZOffset, &RoadColor, &UV0s[0]);
	TArray<FHexVertexData> RoadCentersV;
	RoadCentersV.Init(RoadCenterV, NumOfVerts);

	TArray<FHexVertexData> RoadEgdesLV;
	for (int32 Index = 0; Index < NumOfVerts; ++Index)
	{
		//float RoadWidthScale = 1.0f - FMath::Clamp(CalcDiffToRoadVert(RoadVertIndices, EdgesV[Index].VertexIndex) - 2, 0, 4) * 0.125f;
		float RoadWidthScale = CalcRoadWidthScale(CalcDiffToRoadVert(RoadVertIndices, EdgesV[Index].VertexIndex));
		float ScaledRoadWidthRatio = RoadWidthScale * RoadWidthRatio;

		FHexVertexData RoadEdge = FHexVertexData::LerpVertex(CenterV, EdgesV[Index], FVector::OneVector * ScaledRoadWidthRatio, ScaledRoadWidthRatio);
		RoadEdge.ApplyOverrideInline(RoadZOffset, &RoadColor, &UV0s[1]);
		RoadEgdesLV.Add(RoadEdge);
	}
	FillGrid(RoadCentersV, RoadEgdesLV, OutTerrainMesh.RoadSection, 1);
}

void AHexTerrainGenerator::GenerateNoTerraceCorner(const FHexCellData& Cell1, const FHexCellData& Cell2, const FHexCellData& Cell3,
	const FHexCellCorner& CornerData, FCachedChunkData& OutTerrainMesh)
{
	FHexVertexData V0 = CalcHexCellVertex(Cell1, CornerData.VertsId.X, false);
	FHexVertexData V1 = CalcHexCellVertex(Cell2, CornerData.VertsId.Y, false);
	FHexVertexData V2 = CalcHexCellVertex(Cell3, CornerData.VertsId.Z, false);

	PerturbingVertexInline(V0);
	PerturbingVertexInline(V1);
	PerturbingVertexInline(V2);

	OutTerrainMesh.GroundSection.AddTriangle(V0, V1, V2);
}

void AHexTerrainGenerator::GenerateCornerWithTerrace(const FHexCellData& Cell1, const FHexCellData& Cell2, const FHexCellData& Cell3, const FHexCellCorner& CornerData, FCachedChunkData& OutTerrainMesh)
{
	const FHexCellData* CellsList[3];
	EHexBorderState LinkState[3];
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

	FHexVertexData Vert0 = CalcHexCellVertex(*CellsList[0], VertsList[0], false);
	FHexVertexData Vert1 = CalcHexCellVertex(*CellsList[1], VertsList[1], false);
	FHexVertexData Vert2 = CalcHexCellVertex(*CellsList[2], VertsList[2], false);

	auto CalcTerraceVerts = [this](TArray<TArray<FHexVertexData>>& OutVerts, 
		const FHexVertexData& ToVert, const FHexVertexData& FromVert, int32 ToElevation, int32 FromElevation)
		{
			int32 NumOfZSteps = ToElevation - FromElevation;
			int32 NumOfSteps = NumOfZSteps * 2 - 1;

			int32 BaseIndex = OutVerts.Num();
			OutVerts.AddDefaulted(NumOfZSteps);
			
			for (int32 StepIndex = 1; StepIndex <= NumOfSteps; ++StepIndex)
			{
				float RatioXY = float(StepIndex) / float(NumOfSteps);
				int32 StepZIndex = (StepIndex - 1) / 2 + 1;
				float RatioZ = float(StepZIndex) / float(NumOfZSteps);
				
				FHexVertexData CurStepVert = FHexVertexData::LerpVertex(FromVert, ToVert, FVector{ RatioXY, RatioXY, RatioZ }, RatioXY);

				OutVerts[BaseIndex + StepZIndex - 1].Add(CurStepVert);
			}
		};

	auto CalcLinearVerts = [](TArray<TArray<FHexVertexData>>& OutVerts,
		const FHexVertexData& ToVert, const FHexVertexData& FromVert, int32 ToElevation, int32 FromElevation)
		{
			int32 NumOfSteps = ToElevation - FromElevation;
			if (NumOfSteps == 0)
			{
				OutVerts.Last()[0] = ToVert;
			}
			else
			{
				int32 BaseIndex = OutVerts.Num();
				OutVerts.AddDefaulted(NumOfSteps);

				for (int32 StepIndex = 1; StepIndex <= NumOfSteps; ++StepIndex)
				{
					float Ratio = float(StepIndex) / float(NumOfSteps);

					FHexVertexData CurStepVert = FHexVertexData::LerpVertex(FromVert, ToVert, FVector{ Ratio, Ratio, Ratio }, Ratio);
					OutVerts[BaseIndex + StepIndex - 1].Add(CurStepVert);
				}
			}
		};

	TArray<TArray<FHexVertexData>> Verts01;
	TArray<TArray<FHexVertexData>> Verts02;

	Verts01.AddDefaulted();
	Verts01[0].Add(Vert0);
	Verts02.AddDefaulted();
	Verts02[0].Add(Vert0);

	if (LinkState[0] == EHexBorderState::Terrace)
	{
		CalcTerraceVerts(Verts01, Vert1, Vert0, CellsList[1]->Elevation, CellsList[0]->Elevation);
	}
	else
	{
		CalcLinearVerts(Verts01, Vert1, Vert0, CellsList[1]->Elevation, CellsList[0]->Elevation);
	}

	if (LinkState[2] == EHexBorderState::Terrace)
	{
		CalcTerraceVerts(Verts02, Vert2, Vert0, CellsList[2]->Elevation, CellsList[0]->Elevation);
	}
	else
	{
		CalcLinearVerts(Verts02, Vert2, Vert0, CellsList[2]->Elevation, CellsList[0]->Elevation);
	}
	
	if (LinkState[1] == EHexBorderState::Terrace)
	{
		if (CellsList[1]->Elevation < CellsList[2]->Elevation) // 1 -> 2
		{
			CalcTerraceVerts(Verts01, Vert2, Vert1, CellsList[2]->Elevation, CellsList[1]->Elevation);
		}
		else // 2 -> 1
		{
			CalcTerraceVerts(Verts02, Vert1, Vert2, CellsList[1]->Elevation, CellsList[2]->Elevation);
		}
	}
	else
	{
		if (CellsList[1]->Elevation < CellsList[2]->Elevation) // 1 -> 2
		{
			CalcLinearVerts(Verts01, Vert2, Vert1, CellsList[2]->Elevation, CellsList[1]->Elevation);
		}
		else if (CellsList[1]->Elevation > CellsList[2]->Elevation)// 2 -> 1
		{
			CalcLinearVerts(Verts02, Vert1, Vert2, CellsList[1]->Elevation, CellsList[2]->Elevation);
		}
	}
	
	int32 NumOfLayers01 = Verts01.Num();
	int32 NumOfLayers02 = Verts02.Num();
	check(NumOfLayers01 == NumOfLayers02);

	for (int32 Index = 1; Index < NumOfLayers01; ++Index)
	{
		// Cross Elevation
		FillQuad(Verts02[Index - 1].Last(), Verts01[Index - 1].Last(), Verts02[Index][0], Verts01[Index][0], OutTerrainMesh.GroundSection);

		// Current Elevation
		FillQuad(Verts02[Index][0], Verts01[Index][0], Verts02[Index].Last(), Verts01[Index].Last(), OutTerrainMesh.GroundSection);
	}
}

void AHexTerrainGenerator::GenerateHexWaterCell(const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh)
{
	// Inner HexCell
	GenerateHexWaterCenter(InCellData, OutTerrainMesh);

	// Border
	int32 WIndex = InCellData.HexNeighbors[static_cast<uint8>(EHexDirection::W)].LinkedCellIndex;
	int32 NWIndex = InCellData.HexNeighbors[static_cast<uint8>(EHexDirection::NW)].LinkedCellIndex;
	int32 NEIndex = InCellData.HexNeighbors[static_cast<uint8>(EHexDirection::NE)].LinkedCellIndex;
	if (WIndex >= 0) // W Edge
	{
		GenerateHexWaterBorder(InCellData, EHexDirection::W, OutTerrainMesh);
	}

	if (NWIndex >= 0) // NW Edge
	{
		GenerateHexWaterBorder(InCellData, EHexDirection::NW, OutTerrainMesh);
	}

	if (NEIndex >= 0) // NE Edge
	{
		GenerateHexWaterBorder(InCellData, EHexDirection::NE, OutTerrainMesh);
	}

	if (WIndex >= 0 && NWIndex >= 0) // NW Corner
	{
		GenerateHexWaterCorner(InCellData, EHexDirection::NW, OutTerrainMesh);
	}

	if (NWIndex >= 0 && NEIndex >= 0) // N Corner
	{
		GenerateHexWaterCorner(InCellData, EHexDirection::NE, OutTerrainMesh);
	}
}

void AHexTerrainGenerator::GenerateHexWaterCenter(const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh)
{
	if (InCellData.GetWaterDepth() <= 0)
		return;

	FVector WaterZOffset = CalcWaterVertOffset(InCellData.GetWaterDepth());
	const FColor& WaterColor = ConfigData.ColorsMap[EHexTerrainType::Water];
	FVector2D WaterUV0{ 0.0, 1.0 };

	TArray<FHexVertexData> EdgesV;
	for (int32 VertIndex = 0; VertIndex < CORNER_NUM; ++VertIndex)
	{
		int32 EdgeIndex = FHexCellData::CalcNextDirection(VertIndex);
		int32 NeighborId = InCellData.HexNeighbors[EdgeIndex].LinkedCellIndex;
		if (NeighborId >= 0 && HexGrids[NeighborId].GetWaterDepth() <= 0)
		{
			for (int32 SubIndex = 0; SubIndex <= HexCellSubdivision; ++SubIndex)
			{
				int32 SubVertIndex = SubIndex == 0 ? VertIndex : (VertIndex * HexCellSubdivision + SubIndex - 1);
				EdgesV.Add(CalcHexCellVertex(InCellData, SubVertIndex, SubIndex > 0, 1u, true));
			}
		}
		else
		{
			EdgesV.Add(CalcHexCellVertex(InCellData, VertIndex, false, 1u, true));
		}
	}

	FHexVertexData CenterV{ InCellData.CellCenter + WaterZOffset, WaterColor, FVector::UpVector, WaterUV0 };
	CenterV.VertexState = 1u;

	TArray<bool> Dummy;
	FillFan(CenterV, EdgesV, Dummy, OutTerrainMesh.WaterSection, true);
}

void AHexTerrainGenerator::GenerateHexWaterBorder(const FHexCellData& InCellData, EHexDirection BorderDirection, FCachedChunkData& OutTerrainMesh)
{
	uint8 BorderDirectionId = static_cast<uint8>(BorderDirection);
	const FHexCellBorder& HexBorder = InCellData.HexNeighbors[BorderDirectionId];
	const FHexCellData& OppositeCell = HexGrids[HexBorder.LinkedCellIndex];

	bool bCurrentUnderWater = InCellData.GetWaterDepth() > 0;
	bool bOppositeUnderWater = OppositeCell.GetWaterDepth() > 0;

	if (bCurrentUnderWater && bOppositeUnderWater)
	{
		FHexVertexData FromVert0 = CalcHexCellVertex(InCellData, HexBorder.FromVert.X, false, 1u, true);
		FHexVertexData FromVert1 = CalcHexCellVertex(InCellData, HexBorder.FromVert.Y, false, 1u, true);
		FHexVertexData ToVert0 = CalcHexCellVertex(OppositeCell, HexBorder.ToVert.X, false, 1u, true);
		FHexVertexData ToVert1 = CalcHexCellVertex(OppositeCell, HexBorder.ToVert.Y, false, 1u, true);

		FillQuad(FromVert0, FromVert1, ToVert0, ToVert1, OutTerrainMesh.WaterSection);
	}
	else if (bCurrentUnderWater || bOppositeUnderWater)
	{
		auto CheckEstuary = [this](const FHexCellData& FromCellData, const FHexCellData& ToCellData, EHexDirection BorderDirection) -> int32
			{
				if (FromCellData.HexRiver.RiverState == EHexRiverState::None ||
					ToCellData.HexRiver.RiverState == EHexRiverState::None)
					return 0;

				bool bFromCellUnderwater = FromCellData.GetWaterDepth() > 0;
				bool bToCellUnderwater = ToCellData.GetWaterDepth() > 0;
				double FromCellWaterOffZ = bFromCellUnderwater ? CalcWaterVertOffset(FromCellData.GetWaterDepth()).Z : CalcWaterVertOffset().Z;
				double ToCellWaterOffZ = bToCellUnderwater ? CalcWaterVertOffset(ToCellData.GetWaterDepth()).Z : CalcWaterVertOffset().Z;
				double RiverWaterLevel = FromCellData.CellCenter.Z + FromCellWaterOffZ;
				double OpenWaterLevel = ToCellData.CellCenter.Z + ToCellWaterOffZ;

				if (!FMath::IsNearlyZero(RiverWaterLevel - OpenWaterLevel))
					return 0;

				if (FromCellData.HexRiver.RiverState == EHexRiverState::StartPoint ||
					FromCellData.HexRiver.RiverState == EHexRiverState::PassThrough)
				{
					if (BorderDirection == FromCellData.HexRiver.OutgoingDirection)
						return 1;
				}

				if (ToCellData.HexRiver.RiverState == EHexRiverState::StartPoint ||
					ToCellData.HexRiver.RiverState == EHexRiverState::PassThrough)
				{
					EHexDirection OppoBorderDirection = FHexCellData::CalcOppositeDirection(BorderDirection);
					if (OppoBorderDirection == ToCellData.HexRiver.OutgoingDirection)
						return -1;
				}

				return 0;
			};
		
		int32 EstuaryDirection = CheckEstuary(InCellData, OppositeCell, BorderDirection);

		TArray<FHexVertexData> FromVerts;
		TArray<FHexVertexData> ToVerts;

		FromVerts.Add(CalcHexCellVertex(InCellData, HexBorder.FromVert.X, false, 1u, true));
		ToVerts.Add(CalcHexCellVertex(OppositeCell, HexBorder.ToVert.X, false, 1u, true));

		for (int32 SubIndex = 0; SubIndex < HexCellSubdivision; ++SubIndex)
		{
			int32 SubVertIndex = HexBorder.FromVert.X * HexCellSubdivision + SubIndex;
			FromVerts.Add(CalcHexCellVertex(InCellData, SubVertIndex, true, 1u, true));

			SubVertIndex = HexBorder.ToVert.Y * HexCellSubdivision + (HexCellSubdivision - SubIndex - 1);
			ToVerts.Add(CalcHexCellVertex(OppositeCell, SubVertIndex, true, 1u, true));
		}

		FromVerts.Add(CalcHexCellVertex(InCellData, HexBorder.FromVert.Y, false, 1u, true));
		ToVerts.Add(CalcHexCellVertex(OppositeCell, HexBorder.ToVert.Y, false, 1u, true));
		
		if (EstuaryDirection != 0)
		{
			int32 LastIndex = FromVerts.Num() - 1;
			if (bCurrentUnderWater)
			{
				FillQuad(FromVerts[0], FromVerts[1], ToVerts[0], ToVerts[0], OutTerrainMesh.WaterSection);
				FillQuad(FromVerts[LastIndex - 1], FromVerts[LastIndex], ToVerts[LastIndex], ToVerts[LastIndex], OutTerrainMesh.WaterSection);
			}
			else
			{
				FillQuad(FromVerts[0], FromVerts[0], ToVerts[0], ToVerts[1], OutTerrainMesh.WaterSection);
				FillQuad(FromVerts[LastIndex], FromVerts[LastIndex], ToVerts[LastIndex - 1], ToVerts[LastIndex], OutTerrainMesh.WaterSection);
			}
			
			static double ShoreUV1Us[5] = { 1.0, 0.0, 0.0, 0.0, 1.0 };
			static double WaterUV1Us[5] = { 2.0, 1.4, 1.2, 1.4, 2.0 };
			double UV1UOffset = (bCurrentUnderWater == EstuaryDirection > 0) ? 10.0 : 0.0;

			int32 MidIndex = LastIndex / 2;
			for (int32 Index = 0; Index <= LastIndex; ++Index)
			{
				int32 DistToMid = Index - MidIndex;
				double ShoreUV1V = DistToMid * 0.5 * EstuaryDirection + 0.5;
				double WaterUV1V = DistToMid * 0.2 * EstuaryDirection + 0.5;
				double ShoreUV1U = ShoreUV1Us[Index] + UV1UOffset;
				double WaterUV1U = WaterUV1Us[Index] + UV1UOffset;

				if (bCurrentUnderWater)
				{
					FromVerts[Index].SetUV1(FVector2D{ WaterUV1U, WaterUV1V});
					ToVerts[Index].SetUV1(FVector2D{ ShoreUV1U, ShoreUV1V });
				}
				else
				{
					FromVerts[Index].SetUV1(FVector2D{ ShoreUV1U, ShoreUV1V });
					ToVerts[Index].SetUV1(FVector2D{ WaterUV1U, WaterUV1V });
				}
			}
			
			if (bCurrentUnderWater)
			{
				for (int32 Index = 1; Index < MidIndex; ++Index)
				{
					FillQuad(FromVerts[Index], FromVerts[Index + 1], ToVerts[Index - 1], ToVerts[Index], OutTerrainMesh.EstuarySection);
				}
				FillQuad(FromVerts[MidIndex], FromVerts[MidIndex], ToVerts[MidIndex - 1], ToVerts[MidIndex + 1], OutTerrainMesh.EstuarySection);
				for (int32 Index = MidIndex; Index < LastIndex - 1; ++Index)
				{
					FillQuad(FromVerts[Index], FromVerts[Index + 1], ToVerts[Index + 1], ToVerts[Index + 2], OutTerrainMesh.EstuarySection, true);
				}
			}
			else
			{
				for (int32 Index = 1; Index < MidIndex; ++Index)
				{
					FillQuad(FromVerts[Index - 1], FromVerts[Index], ToVerts[Index], ToVerts[Index + 1], OutTerrainMesh.EstuarySection, true);
				}
				FillQuad(FromVerts[MidIndex - 1], FromVerts[MidIndex + 1], ToVerts[MidIndex], ToVerts[MidIndex], OutTerrainMesh.EstuarySection);
				for (int32 Index = MidIndex; Index < LastIndex - 1; ++Index)
				{
					FillQuad(FromVerts[Index + 1], FromVerts[Index + 2], ToVerts[Index], ToVerts[Index + 1], OutTerrainMesh.EstuarySection);
				}
			}
		}
		else
		{
			int32 NumOfSegments = FromVerts.Num() - 1;
			for (int32 Index = 0; Index < NumOfSegments; ++Index)
			{
				FillQuad(FromVerts[Index], FromVerts[Index + 1], ToVerts[Index], ToVerts[Index + 1], OutTerrainMesh.WaterSection);
			}
		}
	}
}

void AHexTerrainGenerator::GenerateHexWaterCorner(const FHexCellData& InCellData, EHexDirection CornerDirection, FCachedChunkData& OutTerrainMesh)
{
	int32 CIndex = static_cast<uint8>(CornerDirection) - 4;
	const FHexCellCorner& CornerData = InCellData.HexCorners[CIndex];

	const FHexCellData& Cell1 = HexGrids[CornerData.LinkedCellsIndex.X];
	const FHexCellData& Cell2 = HexGrids[CornerData.LinkedCellsIndex.Y];
	const FHexCellData& Cell3 = HexGrids[CornerData.LinkedCellsIndex.Z];

	bool bCell1UnderWater = Cell1.GetWaterDepth() > 0;
	bool bCell2UnderWater = Cell2.GetWaterDepth() > 0;
	bool bCell3UnderWater = Cell3.GetWaterDepth() > 0;

	if (bCell1UnderWater || bCell2UnderWater || bCell3UnderWater)
	{
		FHexVertexData V0 = CalcHexCellVertex(Cell1, CornerData.VertsId.X, false, 1u, true);
		FHexVertexData V1 = CalcHexCellVertex(Cell2, CornerData.VertsId.Y, false, 1u, true);
		FHexVertexData V2 = CalcHexCellVertex(Cell3, CornerData.VertsId.Z, false, 1u, true);

		PerturbingVertexInline(V0);
		PerturbingVertexInline(V1);
		PerturbingVertexInline(V2);

		OutTerrainMesh.WaterSection.AddTriangle(V0, V1, V2);
	}
}

FVector AHexTerrainGenerator::CalcHexCellCenter(const FIntPoint& GridId, int32 Elevation) const
{
	static FVector2D VertOffsetScale{ 1.732050807568877, 1.5 };
	float CellOuterRadius = HexCellRadius + HexCellBorderWidth;

	FVector VertOffset;
	VertOffset.X = (GridId.X + (GridId.Y % 2) * 0.5) * CellOuterRadius * VertOffsetScale.X;
	VertOffset.Y = GridId.Y * CellOuterRadius * VertOffsetScale.Y;
	VertOffset.Z = Elevation * HexElevationStep;
	
	return VertOffset;
}

FHexVertexData AHexTerrainGenerator::CalcHexCellVertex(const FHexCellData& InCellData, int32 VertIndex, bool bSubVert, uint8 VertState, bool bFillDefaultNormal) const
{
	FHexVertexData OutVertex{ InCellData.CellCenter, InCellData.SRGBColor };
	const FColor& WaterColor = ConfigData.ColorsMap[EHexTerrainType::Water];
	TArray<int32> BorderDirections;

	if (bSubVert)
	{
		int32 VertDirectionId = VertIndex / int32(HexCellSubdivision);
		OutVertex.VertexIndex = VertIndex + VertDirectionId + 1;
		int32 BorderDirectionId = FHexCellData::CalcNextDirection(uint8(VertDirectionId));

		if (VertState == 0u)
		{
			TArray<int32> RiverVerts;
			switch (InCellData.HexRiver.RiverState)
			{
			case EHexRiverState::StartPoint:
			{
				RiverVerts.Add(FHexCellData::GetVertIdFromDirection(InCellData.HexRiver.OutgoingDirection));
				break;
			}
			case EHexRiverState::EndPoint:
			{
				RiverVerts.Add(FHexCellData::GetVertIdFromDirection(InCellData.HexRiver.IncomingDirection));
				break;
			}
			case EHexRiverState::PassThrough:
			{
				RiverVerts.Add(FHexCellData::GetVertIdFromDirection(InCellData.HexRiver.IncomingDirection));
				RiverVerts.Add(FHexCellData::GetVertIdFromDirection(InCellData.HexRiver.OutgoingDirection));
				break;
			}

			default:
				break;
			}

			TArray<int32> RoadVerts;
			if (InCellData.HexRoad.RoadState[BorderDirectionId])
			{
				uint8 RoadMidVert = FHexCellData::GetVertIdFromDirection(static_cast<EHexDirection>(BorderDirectionId));
				RoadVerts.Add(RoadMidVert);
			}

			if (RiverVerts.Contains(VertIndex))
			{
				OutVertex.VertexState = 1u;
				if (InCellData.GetWaterDepth() <= 0)
					OutVertex.ApplyOverrideInline(CalcRiverVertOffset(), &WaterColor);
			}
			else if (RoadVerts.Contains(VertIndex))
			{
				OutVertex.VertexState = 2u;
			}
		}
		else if (VertState == 1u)
		{
			BorderDirections.Add(BorderDirectionId);
		}
	}
	else
	{
		OutVertex.VertexIndex = VertIndex * (HexCellSubdivision + 1);

		if (VertState == 1u)
		{
			BorderDirections.Add(VertIndex);
			BorderDirections.Add(FHexCellData::CalcNextDirection(uint8(VertIndex)));
		}
	}

	double WaterVertRadiusScale = 1.0;
	if (VertState == 1u)
	{
		int32 WaterDepth = InCellData.GetWaterDepth();

		bool bShore = WaterDepth <= 0;
		bool bNearShore = bShore;
		bool bFullNeighbor = true;
		int32 NeighborWaterLevel = FHexCellConfigData::DefaultWaterLevel;
		for (int32 OneDir : BorderDirections)
		{
			int32 NeighborId = InCellData.HexNeighbors[OneDir].LinkedCellIndex;
			if (NeighborId >= 0)
			{
				int32 NeighborWaterDepth = HexGrids[NeighborId].GetWaterDepth();
				bNearShore = bNearShore || NeighborWaterDepth <= 0;
				if (NeighborWaterDepth > 0)
					NeighborWaterLevel = HexGrids[NeighborId].WaterLevel;
			}
			
			bFullNeighbor = bFullNeighbor && NeighborId >= 0;
		}

		OutVertex.VertexState = 1u;
		FVector2D WaterUV0{ bShore ? 1.0 : 0.0, bNearShore ? 0.0 : 1.0 };
		OutVertex.ApplyOverrideInline(CalcWaterVertOffset(bShore ? (NeighborWaterLevel - InCellData.Elevation) : WaterDepth), &WaterColor, &WaterUV0);
		
		WaterVertRadiusScale = (bFullNeighbor && WaterDepth > 0) ? 0.8 : 1.0;
	}

	if (bSubVert)
		OutVertex.ApplyOverrideInline(FHexCellData::HexSubVertices[VertIndex] * WaterVertRadiusScale);
	else
		OutVertex.ApplyOverrideInline(FHexCellData::HexVertices[VertIndex] * WaterVertRadiusScale);

	if (bFillDefaultNormal)
		OutVertex.SetNormal(FVector::UpVector);

	return OutVertex;
}

FIntPoint AHexTerrainGenerator::CalcHexCellGridId(const FVector& WorldPos) const
{
	static FVector2D VertOffsetScale{ 1.732050807568877, 1.5 };
	float CellOuterRadius = HexCellRadius + HexCellBorderWidth;

	FIntPoint GridId;
	GridId.Y = FMath::RoundToInt(WorldPos.Y / (CellOuterRadius * VertOffsetScale.Y));
	GridId.X = FMath::RoundToInt(WorldPos.X / (CellOuterRadius * VertOffsetScale.X) - (GridId.Y % 2) * 0.5);
	return GridId;
}

int32 AHexTerrainGenerator::CalcDiffToRoadVert(const TArray<int32>& RoadVertIndices, int32 CurIndex) const
{
	check(CurIndex >= 0);
	int32 MaxRoadIndex = (HexCellSubdivision + 1) * CORNER_NUM;
	if (RoadVertIndices.IsEmpty())
		return MaxRoadIndex;

	TArray<int32> CopyiedRoadVertIndices{ RoadVertIndices };
	int32 FirstRoadIndex = RoadVertIndices[0];
	int32 LastRoadIndex = RoadVertIndices.Last();
	CopyiedRoadVertIndices.Add(FirstRoadIndex + MaxRoadIndex);
	CopyiedRoadVertIndices.Insert(LastRoadIndex - MaxRoadIndex, 0);

	int32 NumOfRoadVerts = CopyiedRoadVertIndices.Num();
	for (int32 Index = 0; Index < NumOfRoadVerts; ++Index)
	{
		if (CurIndex > CopyiedRoadVertIndices[Index])
			continue;

		if (Index == 0)
			return CopyiedRoadVertIndices[Index] - CurIndex;
		else
			return FMath::Min(CopyiedRoadVertIndices[Index] - CurIndex, CurIndex - CopyiedRoadVertIndices[Index - 1]);
	}
	return CurIndex - CopyiedRoadVertIndices[NumOfRoadVerts - 1];
}

float AHexTerrainGenerator::CalcRoadWidthScale(int32 DiffToRoad) const
{	
	static float DiffToScale[5] = { 1.0f, 0.8f, 2.0f / 3.0f, 4.0f / 7.0f, 0.5f };
	DiffToRoad = FMath::Clamp(DiffToRoad - 2, 0, 4);
	//return 1.0f - DiffToRoad*0.125f;
	return DiffToScale[DiffToRoad];
}

void AHexTerrainGenerator::CalcFeatureTransform(const FHexCellData& InCellData, const FVector& InCenter, int32 LocDirectionId, TArray<FTransform>& OutFeatureLocations)
{
	if (InCellData.GetWaterDepth() > 0)
		return;

	const FHexCellRiver& RiverData = InCellData.HexRiver;
	const FHexCellRoad& RoadData = InCellData.HexRoad;
	FVector FeatureLocation;
	if (LocDirectionId >= 0)
	{
		EHexDirection LocDirection = static_cast<EHexDirection>(LocDirectionId);
		if (RiverData.CheckRiver(LocDirection) || RoadData.RoadState[LocDirectionId])
			return;

		int32 SubVertIndex = FHexCellData::GetVertIdFromDirection(LocDirection, true);
		FVector HexEdgeCenter = InCellData.CellCenter + FHexCellData::HexSubVertices[SubVertIndex];
		FeatureLocation = FMath::Lerp(InCenter, HexEdgeCenter, 0.4);
	}
	else
	{
		if (RiverData.RiverState != EHexRiverState::None || RoadData.GetPackedState() > 0u)
			return;

		FeatureLocation = InCellData.CellCenter;
	}
	
	FVector2D FeatureRandom = GetRandomValueByPosition(FeatureLocation);
	if (FeatureRandom.Y < 0.5)
		return;

	PerturbingVertexInline(FeatureLocation, PerturbingStrengthHV, true);
	
	FQuat RandRot{ FVector::UpVector, FeatureRandom.X * UE_TWO_PI };
	FTransform FeatureTransform{ RandRot, FeatureLocation, FVector{20.0, 20.0, 10.0} };

	OutFeatureLocations.Add(FeatureTransform);
}

FVector AHexTerrainGenerator::CalcFaceNormal(const FVector& V0, const FVector& V1, const FVector& V2)
{
	FVector Edge1 = (V1 - V0);
	FVector Edge2 = (V2 - V0);
	FVector NormalVector = FVector::CrossProduct(Edge1, Edge2);
	return NormalVector.GetSafeNormal();
}

void AHexTerrainGenerator::FillGrid(const TArray<FHexVertexData>& FromV, const TArray<FHexVertexData>& ToV, FCachedSectionData& OutTerrainMesh,
	int32 NumOfSteps, bool bTerrace, bool bClosed, bool bRotTriangle)
{
	check(FromV.Num() == ToV.Num());

	int32 NumOfStrips = FromV.Num() - 1;
	for (int32 Index = 0; Index < NumOfStrips; ++Index)
	{
		FillStrip(
			FromV[Index], FromV[Index + 1], ToV[Index], ToV[Index + 1],
			OutTerrainMesh, NumOfSteps, bTerrace, bRotTriangle
		);
	}

	if (bClosed)
	{
		FillStrip(
			FromV[NumOfStrips], FromV[0], ToV[NumOfStrips], ToV[0],
			OutTerrainMesh, NumOfSteps, bTerrace, bRotTriangle
		);
	}
}

void AHexTerrainGenerator::FillStrip(const FHexVertexData& FromV0, const FHexVertexData& FromV1, const FHexVertexData& ToV0, const FHexVertexData& ToV1,
	FCachedSectionData& OutTerrainMesh, int32 NumOfSteps, bool bTerrace, bool bRotTriangle)
{
	FHexVertexData LastStepV0 = FromV0;
	FHexVertexData LastStepV1 = FromV1;
	int32 NumOfFinalSteps = bTerrace ? NumOfSteps * 2 - 1 : NumOfSteps;

	for (int32 StepIndex = 1; StepIndex <= NumOfFinalSteps; ++StepIndex)
	{
		float RatioXY = 0.0f;
		float RatioZ = 0.0f;
		if (bTerrace)
		{
			RatioXY = float(StepIndex) / float(NumOfFinalSteps);
			int32 StepZIndex = (StepIndex - 1) / 2 + 1;
			RatioZ = float(StepZIndex) / float(NumOfSteps);
		}
		else
		{
			RatioXY = float(StepIndex) / float(NumOfFinalSteps);
			RatioZ = RatioXY;
		}

		FHexVertexData CurStepV0 = FHexVertexData::LerpVertex(FromV0, ToV0, FVector{ RatioXY ,RatioXY ,RatioZ }, RatioXY);
		FHexVertexData CurStepV1 = FHexVertexData::LerpVertex(FromV1, ToV1, FVector{ RatioXY ,RatioXY ,RatioZ }, RatioXY);

		FillQuad(LastStepV0, LastStepV1, CurStepV0, CurStepV1, OutTerrainMesh, bRotTriangle);

		LastStepV0 = CurStepV0;
		LastStepV1 = CurStepV1;
	}
}

void AHexTerrainGenerator::FillQuad(const FHexVertexData& FromV0, const FHexVertexData& FromV1, const FHexVertexData& ToV0, const FHexVertexData& ToV1, FCachedSectionData& OutTerrainMesh, bool bRotTriangle)
{
	FHexVertexData FromV0_C = PerturbingVertex(FromV0);
	FHexVertexData FromV1_C = PerturbingVertex(FromV1);
	FHexVertexData ToV0_C = PerturbingVertex(ToV0);
	FHexVertexData ToV1_C = PerturbingVertex(ToV1);

	if ((FromV0.Position - FromV1.Position).IsNearlyZero())
	{
		OutTerrainMesh.AddTriangle(ToV0_C, FromV0_C, ToV1_C);
	}
	else if ((ToV0.Position - ToV1.Position).IsNearlyZero())
	{
		OutTerrainMesh.AddTriangle(ToV0_C, FromV0_C, FromV1_C);
	}
	else if ((ToV0.Position - FromV0.Position).IsNearlyZero())
	{
		OutTerrainMesh.AddTriangle(ToV0_C, FromV1_C, ToV1_C);
	}
	else if ((ToV1.Position - FromV1.Position).IsNearlyZero())
	{
		OutTerrainMesh.AddTriangle(ToV0_C, FromV0_C, ToV1_C);
	}
	else
	{
		FVector TestNormal = CalcFaceNormal(FromV0_C.Position, FromV1_C.Position, ToV0_C.Position);
		FVector TestVec = ToV1_C.Position - FromV0_C.Position;
		if (FMath::IsNearlyZero(FVector::DotProduct(TestNormal, TestVec), 1e-4)) // coplane
		{
			if (bRotTriangle)
				OutTerrainMesh.AddQuad(FromV0_C, ToV0_C, FromV1_C, ToV1_C);
			else
				OutTerrainMesh.AddQuad(ToV0_C, ToV1_C, FromV0_C, FromV1_C);
		}
		else
		{
			if (bRotTriangle)
			{
				OutTerrainMesh.AddTriangle(FromV0_C, FromV1_C, ToV0_C);
				OutTerrainMesh.AddTriangle(ToV0_C, FromV1_C, ToV1_C);
			}
			else
			{
				OutTerrainMesh.AddTriangle(ToV0_C, FromV0_C, ToV1_C);
				OutTerrainMesh.AddTriangle(ToV1_C, FromV0_C, FromV1_C);
			}
		}
	}
};

void AHexTerrainGenerator::FillFan(const FHexVertexData& CenterV, const TArray<FHexVertexData>& EdgesV,
	const TArray<bool>& bRecalcNormal, FCachedSectionData& OutTerrainMesh, bool bClosed)
{
	FHexVertexData DisturbedCenter = PerturbingVertex(CenterV);
	int32 NumOfEdges = EdgesV.Num();
	TArray<FHexVertexData> DisturbedEdges;
	DisturbedEdges.Reserve(NumOfEdges);
	for (const FHexVertexData& OneEdge : EdgesV)
		DisturbedEdges.Add(PerturbingVertex(OneEdge));

	int32 BaseIndex = OutTerrainMesh.AddVertex(DisturbedCenter);

	bool bShouldRecalcNormal = bRecalcNormal.Num() > 0;
	TArray<int32> IndicesList;
	
	for (int32 EdgeIndex = 0; EdgeIndex < NumOfEdges; ++EdgeIndex)
	{
		if (bShouldRecalcNormal)
		{
			IndicesList.Add(bRecalcNormal[EdgeIndex] ? -EdgeIndex : EdgeIndex);
		}

		if (!bShouldRecalcNormal || !bRecalcNormal[EdgeIndex])
		{
			OutTerrainMesh.AddVertex(DisturbedEdges[EdgeIndex]);
		}
	}
	
	int32 NumOfTriangles = bClosed ? NumOfEdges: NumOfEdges - 1;
	for (int32 Index = 0; Index < NumOfTriangles; ++Index)
	{
		int32 CurIndex = Index;
		int32 NextIndex = (Index + 1) % NumOfEdges;
		if (bShouldRecalcNormal)
		{
			CurIndex = IndicesList[CurIndex];
			NextIndex = IndicesList[NextIndex];

			if (CurIndex < 0 || NextIndex < 0)
			{
				CurIndex = FMath::Abs(CurIndex);
				NextIndex = FMath::Abs(NextIndex);

				OutTerrainMesh.AddTriangle(DisturbedCenter, DisturbedEdges[NextIndex], DisturbedEdges[CurIndex]);
				continue;
			}
		}
		else
		{
			++CurIndex;
			++NextIndex;
		}

		OutTerrainMesh.AddFace(
			BaseIndex,
			BaseIndex + NextIndex,
			BaseIndex + CurIndex);
	}
}

void AHexTerrainGenerator::PerturbingVertexInline(FVector& Vertex, const FVector2D& Strength, bool bPerturbZ)
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
	Vertex.X += (NoiseVector.R * 2.0f - 1.0f) * Strength.X;
	Vertex.Y += (NoiseVector.G * 2.0f - 1.0f) * Strength.X;
	
	if (bPerturbZ)
	{
		int32 Elevation = FMath::RoundToInt(Vertex.Z / HexElevationStep);
		if (!CachedNoiseZ.Contains(Elevation))
		{
			FLinearColor NoiseVectorZ = SampleTextureBilinear(NoiseTexture, FMath::RoundToInt(Elevation * PerturbingScalingHV.Y), 0);
			CachedNoiseZ.Add(Elevation, NoiseVectorZ.B * 2.0f - 1.0f);
		}

		Vertex.Z += CachedNoiseZ[Elevation] * Strength.Y;
	}
}

FVector AHexTerrainGenerator::PerturbingVertex(const FVector& Vertex, const FVector2D& Strength, bool bPerturbZ)
{
	FVector NewVec = Vertex;
	PerturbingVertexInline(NewVec, Strength, bPerturbZ);
	return NewVec;
}

void AHexTerrainGenerator::PerturbingVertexInline(FHexVertexData& Vertex)
{
	PerturbingVertexInline(Vertex.Position, PerturbingStrengthHV, true);
}

FHexVertexData AHexTerrainGenerator::PerturbingVertex(const FHexVertexData& Vertex)
{
	FHexVertexData NewVert = Vertex;
	PerturbingVertexInline(NewVert.Position, PerturbingStrengthHV, true);
	return NewVert;
}

FVector2D AHexTerrainGenerator::GetRandomValueByPosition(const FVector& InVertex) const
{
	if (RandomCache.IsEmpty())
		return FVector2D::ZeroVector;

	int32 X = FMath::RoundToInt(InVertex.X * 0.1);
	int32 Y = FMath::RoundToInt(InVertex.Y * 0.1);

	int32 CacheSizeX = RandomCache[0].Num();
	int32 CacheSizeY = RandomCache.Num();

	int32 CacheTillingStartX = FMath::FloorToInt(float(X) / CacheSizeX) * CacheSizeX;
	int32 CacheTillingStartY = FMath::FloorToInt(float(Y) / CacheSizeY) * CacheSizeY;

	X = X - CacheTillingStartX;
	Y = Y - CacheTillingStartY;
	
	return RandomCache[Y][X];
}

FLinearColor AHexTerrainGenerator::SampleTextureBilinear(const TArray<TArray<FColor>>& InTexture, const FVector& SamplePos) const
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

	FLinearColor TColor = FMath::Lerp(LTColor.ReinterpretAsLinear(), RTColor.ReinterpretAsLinear(), RatioX);
	FLinearColor DColor = FMath::Lerp(LDColor.ReinterpretAsLinear(), RDColor.ReinterpretAsLinear(), RatioX);

	return FMath::Lerp(TColor, DColor, RatioY);
}

FLinearColor AHexTerrainGenerator::SampleTextureBilinear(const TArray<TArray<FColor>>& InTexture, int32 SampleX, int32 SampleY) const
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

	if (!PlayerController)
		PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
		return;

	FHitResult HitResult;
	FIntPoint GridId = FIntPoint::ZeroValue;
	bool bHit = PlayerController->GetHitResultUnderCursor(ECC_Visibility, true, HitResult);
	if (bHit)
	{
		GridId = CalcHexCellGridId(HitResult.Location);
	}
	
	switch (HexEditMode)
	{
	case EHexEditMode::Ground:
		HexEditGround(bHit, GridId);
		break;

	case EHexEditMode::Road:
		HexEditRoad(bHit, GridId);
		break;

	case EHexEditMode::River:
		HexEditRiver(bHit, GridId);
		break;

	default:
		break;
	}
}

void AHexTerrainGenerator::OnReleased(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	//UE_LOG(LogTemp, Display, TEXT("OnRelease!!"));
}

void AHexTerrainGenerator::PostLoad()
{
	Super::PostLoad();

	TArray<uint8> TextureBinData;
	FFileHelper::LoadFileToArray(TextureBinData, *(FPaths::ProjectDir() / NoiseTexturePath));
	CreateTextureFromData(NoiseTexture, TextureBinData, EImageFormat::PNG);

	static int32 RandomCacheSize = 512;
	RandomCache.Empty(RandomCacheSize);
	RandomCache.AddDefaulted(RandomCacheSize);
	for (int32 Y = 0; Y < RandomCacheSize; ++Y)
	{
		RandomCache[Y].AddDefaulted(RandomCacheSize);
		for (int32 X = 0; X < RandomCacheSize; ++X)
		{
			RandomCache[Y][X].X = FMath::FRand();
			RandomCache[Y][X].Y = FMath::FRand();
		}
	}

	LoadTerrain();
}

#if WITH_EDITOR

void AHexTerrainGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FProperty* MemberPropertyThatChanged = PropertyChangedEvent.MemberProperty;
	const FName MemberPropertyName = MemberPropertyThatChanged != NULL ? MemberPropertyThatChanged->GetFName() : NAME_None;

	static FName Name_HexEditMode = GET_MEMBER_NAME_CHECKED(AHexTerrainGenerator, HexEditMode);
	if (MemberPropertyName == Name_HexEditMode)
	{
		ClearEditParameters(EHexEditMode::Ground);
		ClearEditParameters(EHexEditMode::River);
		ClearEditParameters(EHexEditMode::Road);
	}

	static FName Name_HexEditElevation = GET_MEMBER_NAME_CHECKED(AHexTerrainGenerator, HexEditElevation);
	static FName Name_HexEditWaterLevel = GET_MEMBER_NAME_CHECKED(AHexTerrainGenerator, HexEditWaterLevel);
	static FName Name_HexEditTerrainType = GET_MEMBER_NAME_CHECKED(AHexTerrainGenerator, HexEditTerrainType);
	if (MemberPropertyName == Name_HexEditElevation || 
		MemberPropertyName == Name_HexEditTerrainType)
	{
		if (HexEditGridId.X >= 0 && HexEditGridId.Y >= 0)
		{
			ConfigData.ElevationsList[HexEditGridId.Y][HexEditGridId.X] = HexEditElevation;
			ConfigData.TerrainTypesList[HexEditGridId.Y][HexEditGridId.X] = HexEditTerrainType;

			UpdateHexGridsData();
			GenerateTerrain();
		}
	}
	else if (MemberPropertyName == Name_HexEditWaterLevel)
	{
		if (HexEditGridId.X >= 0 && HexEditGridId.Y >= 0)
		{
			TSet<FIntPoint> ProcessedGrids;
			HexEditWater(ProcessedGrids, HexEditGridId, HexEditWaterLevel);

			UpdateHexGridsData();
			GenerateTerrain();
		}
	}

	static FName Name_ConfigFileName = GET_MEMBER_NAME_CHECKED(AHexTerrainGenerator, ConfigFileName);
	if (MemberPropertyName == Name_ConfigFileName)
	{
		LoadTerrain();
	}

	static FName Name_HexCellRadius = GET_MEMBER_NAME_CHECKED(AHexTerrainGenerator, HexCellRadius);
	static FName Name_HexCellBorderWidth = GET_MEMBER_NAME_CHECKED(AHexTerrainGenerator, HexCellBorderWidth);
	if (MemberPropertyName == Name_HexCellRadius || MemberPropertyName == Name_HexCellBorderWidth)
	{
		UpdateHexGridsData();
	}
}

#endif

void AHexTerrainGenerator::HexEditGround(bool bHit, const FIntPoint& HitGridId)
{
	if (bHit)
	{
		HexEditGridId.X = HitGridId.X;
		HexEditGridId.Y = HitGridId.Y;

		HexEditElevation = ConfigData.ElevationsList[HitGridId.Y][HitGridId.X];
		HexEditWaterLevel = ConfigData.WaterLevelsList[HitGridId.Y][HitGridId.X];
		HexEditTerrainType = ConfigData.TerrainTypesList[HitGridId.Y][HitGridId.X];
	}
	else
		ClearEditParameters(EHexEditMode::Ground);
}

void AHexTerrainGenerator::HexEditRoad(bool bHit, const FIntPoint& HitGridId)
{
	if (!bHit)
		return;

	if (HexEditRoadFirstPoint.X < 0 || HexEditRoadFirstPoint.Y < 0) // road's first node
	{
		HexEditRoadFirstPoint = HitGridId;
	}
	else
	{
		int32 FirstGridIndex = HexEditRoadFirstPoint.Y * HexChunkCount.X * HexChunkSize.X + HexEditRoadFirstPoint.X;
		int32 SecondGridIndex = HitGridId.Y * HexChunkCount.X * HexChunkSize.X + HitGridId.X;
		const FHexCellData& FirstGrid = HexGrids[FirstGridIndex];
		const FHexCellData& SecondGrid = HexGrids[SecondGridIndex];

		int32 RoadDirection = -1;
		for (int32 Index = 0; Index < CORNER_NUM; ++Index)
		{
			EHexDirection EdgeDirection = static_cast<EHexDirection>(Index);

			if (FirstGrid.HexRiver.CheckRiver(EdgeDirection))
				continue;

			if (FirstGrid.HexNeighbors[Index].LinkedCellIndex == SecondGridIndex)
			{
				RoadDirection = Index;
				break;
			}
		}

		if (RoadDirection >= 0)
		{
			int32 OppoDirection = FHexCellData::CalcOppositeDirection(RoadDirection);
			if (FirstGrid.HexRoad.RoadState[RoadDirection] || SecondGrid.HexRoad.RoadState[OppoDirection])
			{
				int32 RoadIndex = FirstGrid.HexRoad.RoadIndex[RoadDirection];
				FHexRiverRoadConfigData& RoadConfig = ConfigData.RoadsList[RoadIndex];

				if (RoadConfig.StartPoint == HexEditRoadFirstPoint)
					RoadConfig.ExtensionDirections.Remove(static_cast<EHexDirection>(RoadDirection));
				else
					RoadConfig.ExtensionDirections.Remove(static_cast<EHexDirection>(OppoDirection));

				if (RoadConfig.ExtensionDirections.IsEmpty())
				{
					ConfigData.RoadsList.RemoveAt(RoadIndex);
				}
			}
			else
			{
				int32 RoadIndex = -1;
				int32 NumOfRoads = ConfigData.RoadsList.Num();
				for (int32 Index = 0; Index < NumOfRoads; ++Index)
				{
					FHexRiverRoadConfigData& RoadConfig = ConfigData.RoadsList[Index];
					if (RoadConfig.StartPoint == HexEditRoadFirstPoint ||
						RoadConfig.StartPoint == HitGridId)
					{
						RoadIndex = Index;
						break;
					}
				}

				if (RoadIndex >= 0)
				{
					FHexRiverRoadConfigData& RoadConfig = ConfigData.RoadsList[RoadIndex];
					if (RoadConfig.StartPoint == HexEditRoadFirstPoint)
						RoadConfig.ExtensionDirections.Add(static_cast<EHexDirection>(RoadDirection));
					else
						RoadConfig.ExtensionDirections.Add(static_cast<EHexDirection>(OppoDirection));
				}
				else
				{
					RoadIndex = ConfigData.RoadsList.AddDefaulted();
					FHexRiverRoadConfigData& RoadConfig = ConfigData.RoadsList[RoadIndex];
					RoadConfig.StartPoint = HexEditRoadFirstPoint;
					RoadConfig.ExtensionDirections.Add(static_cast<EHexDirection>(RoadDirection));
				}
			}

			UpdateHexGridsData();
			GenerateTerrain();
		}

		ClearEditParameters(EHexEditMode::Road);
	}
}

void AHexTerrainGenerator::HexEditRiver(bool bHit, const FIntPoint& HitGridId)
{
	if (!bHit)
		return;

	int32 GridIndex = HitGridId.Y * HexChunkCount.X * HexChunkSize.X + HitGridId.X;
	const FHexCellData& CurGrid = HexGrids[GridIndex];
	bool bNeedUpdateConfig = true;
	if (CurGrid.HexRiver.RiverState == EHexRiverState::None)
	{
		if (HexEditRiverStartPoint.X < 0 || HexEditRiverStartPoint.Y < 0) // river's first node
		{
			HexEditRiverStartPoint = HitGridId;
			HexEditRiverLastPoint = HitGridId;
			HexEditRiverPoints.Add(HitGridId);
		}
		else // river's other nodes
		{
			int32 LastGridIndex = HexEditRiverLastPoint.Y * HexChunkCount.X * HexChunkSize.X + HexEditRiverLastPoint.X;
			const FHexCellData& LastGrid = HexGrids[LastGridIndex];

			bNeedUpdateConfig = false;
			for (int32 Index = 0; Index < CORNER_NUM; ++Index)
			{
				if (FHexCellData::IsValidRiverDirection(LastGrid, CurGrid) &&
					LastGrid.HexNeighbors[Index].LinkedCellIndex == GridIndex)
				{
					HexEditRiverFlowDirections.Add(static_cast<EHexDirection>(Index));
					HexEditRiverLastPoint = HitGridId;
					HexEditRiverPoints.Add(HitGridId);
					bNeedUpdateConfig = true;
					break;
				}
			}
		}
	}
	else
	{
		if (HexEditRiverPoints.Contains(HitGridId)) // remove river's nodes
		{
			int32 FoundIndex = -1;
			HexEditRiverPoints.Find(HitGridId, FoundIndex);

			if (FoundIndex == 0)
			{
				int32 TempRiverId = HexEditRiverId;
				ClearEditParameters(EHexEditMode::River);
				HexEditRiverId = TempRiverId;
			}
			else
			{
				HexEditRiverLastPoint = HexEditRiverPoints[FoundIndex - 1];
				for (int32 Index = HexEditRiverPoints.Num() - 1; Index >= FoundIndex; --Index)
				{
					HexEditRiverPoints.RemoveAt(Index);
					HexEditRiverFlowDirections.RemoveAt(Index - 1);
				}
			}
		}
		else // select another river
		{
			bNeedUpdateConfig = false;
			ClearEditParameters(EHexEditMode::River);

			int32 RiverIndex = CurGrid.HexRiver.RiverIndex;
			const FHexRiverRoadConfigData& RiverConfig = ConfigData.RiversList[RiverIndex];

			HexEditRiverId = RiverIndex;
			HexEditRiverStartPoint = RiverConfig.StartPoint;
			HexEditRiverFlowDirections = RiverConfig.ExtensionDirections;
			HexEditRiverPoints.Add(HexEditRiverStartPoint);

			int32 FirstIndex = HexEditRiverStartPoint.Y * HexChunkCount.X * HexChunkSize.X + HexEditRiverStartPoint.X;
			const FHexCellData* LastRiverNode = &HexGrids[FirstIndex];
			for (EHexDirection FlowDir : HexEditRiverFlowDirections)
			{
				int32 CurGridIndex = LastRiverNode->HexNeighbors[static_cast<uint8>(FlowDir)].LinkedCellIndex;
				const FHexCellData& CurRiverNode = HexGrids[CurGridIndex];
				FIntPoint CurGridId{
					CurRiverNode.GridId.X * HexChunkSize.X + CurRiverNode.GridId.Z,
					CurRiverNode.GridId.Y * HexChunkSize.Y + CurRiverNode.GridId.W
				};

				HexEditRiverPoints.Add(CurGridId);
				HexEditRiverLastPoint = CurGridId;

				LastRiverNode = &CurRiverNode;
			}
		}
	}

	if (bNeedUpdateConfig)
	{
		if (HexEditRiverPoints.Num() >= 2)
		{
			if (HexEditRiverId < 0)
				HexEditRiverId = ConfigData.RiversList.AddDefaulted();

			FHexRiverRoadConfigData& RiverConfig = ConfigData.RiversList[HexEditRiverId];
			RiverConfig.StartPoint = HexEditRiverStartPoint;
			RiverConfig.ExtensionDirections = HexEditRiverFlowDirections;

			UpdateHexGridsData();
			GenerateTerrain();
		}
		else if (HexEditRiverId >= 0)
		{
			ConfigData.RiversList.RemoveAt(HexEditRiverId);
			ClearEditParameters(EHexEditMode::River);

			UpdateHexGridsData();
			GenerateTerrain();
		}
	}
}

void AHexTerrainGenerator::HexEditWater(TSet<FIntPoint>& ProcessedGrids, const FIntPoint& CurGridId, int32 NewWaterLevel)
{
	if (ProcessedGrids.Contains(CurGridId))
		return;

	int32 GridIndex = CurGridId.Y * HexChunkCount.X * HexChunkSize.X + CurGridId.X;
	const FHexCellData& CurGrid = HexGrids[GridIndex];
	if (CurGrid.Elevation >= NewWaterLevel)
	{
		ConfigData.WaterLevelsList[CurGridId.Y][CurGridId.X] = FHexCellConfigData::DefaultWaterLevel;
		return;
	}
	
	ConfigData.WaterLevelsList[CurGridId.Y][CurGridId.X] = NewWaterLevel;
	ProcessedGrids.Add(CurGridId);
	
	for (int32 Index = 0; Index < CORNER_NUM; ++Index)
	{
		int32 NeighborGridIndex = CurGrid.HexNeighbors[Index].LinkedCellIndex;
		if (NeighborGridIndex < 0)
			continue;

		const FHexCellData& NeighborGrid = HexGrids[NeighborGridIndex];
		FIntPoint NeighborGridId{
			NeighborGrid.GridId.X * HexChunkSize.X + NeighborGrid.GridId.Z,
			NeighborGrid.GridId.Y * HexChunkSize.Y + NeighborGrid.GridId.W
		};
		HexEditWater(ProcessedGrids, NeighborGridId, NewWaterLevel);
	}
}

void AHexTerrainGenerator::ClearEditParameters(EHexEditMode ModeToClear)
{
	if (ModeToClear == EHexEditMode::Ground)
	{
		HexEditGridId = FIntPoint(-1, -1);
		HexEditElevation = 0;
		HexEditTerrainType = EHexTerrainType::None;
	}
	else if (ModeToClear == EHexEditMode::River)
	{
		HexEditRiverId = -1;
		HexEditRiverStartPoint = FIntPoint(-1, -1);
		HexEditRiverLastPoint = FIntPoint(-1, -1);
		HexEditRiverFlowDirections.Empty();
		HexEditRiverPoints.Empty();
	}
	else if (ModeToClear == EHexEditMode::Road)
	{
		HexEditRoadFirstPoint = FIntPoint(-1, -1);
	}
}

//#pragma optimize("", on)