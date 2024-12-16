#include "HexTerrainGenerator.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Kismet/GameplayStatics.h"

#define	CORNER_NUM 6
#define	CORNER_UNUM 6u
#define	CORNER_HALF_UNUM 3u

#pragma optimize("", off)

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
	DirectionId = (DirectionId - 1u + CORNER_UNUM) % CORNER_UNUM;

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
	:Position(InPos), bHasNormal(false), bHasUV0(false), bHasVertexColor(false)
{}

FHexVertexData::FHexVertexData(const FVector& InPos, const FColor& InColor)
	:Position(InPos), VertexColor(InColor)
	, bHasNormal(false), bHasUV0(false), bHasVertexColor(true)
{}

FHexVertexData::FHexVertexData(const FVector& InPos, const FVector2D& InUV0)
	:Position(InPos), UV0(InUV0)
	, bHasNormal(false), bHasUV0(true), bHasVertexColor(false)
{}

FHexVertexData::FHexVertexData(const FVector& InPos, const FColor& InColor, const FVector& InNormal)
	:Position(InPos), Normal(InNormal), VertexColor(InColor)
	, bHasNormal(true), bHasUV0(false), bHasVertexColor(true)
{}

FHexVertexData::FHexVertexData(const FVector& InPos, const FColor& InColor, const FVector2D& InUV0)
	:Position(InPos), UV0(InUV0), VertexColor(InColor)
	, bHasNormal(false), bHasUV0(true), bHasVertexColor(true)
{}

FHexVertexData::FHexVertexData(const FVector& InPos, const FColor& InColor, const FVector& InNormal, const FVector2D& InUV0)
	:Position(InPos), Normal(InNormal), UV0(InUV0), VertexColor(InColor)
	, bHasNormal(true), bHasUV0(true), bHasVertexColor(true)
{}

FHexVertexData FHexVertexData::LerpVertex(const FHexVertexData& FromV, const FHexVertexData& ToV, FVector PosRatio, float AttrRatio)
{
	FVector NewVertex;
	NewVertex.X = FMath::Lerp(FromV.Position.X, ToV.Position.X, PosRatio.X);
	NewVertex.Y = FMath::Lerp(FromV.Position.Y, ToV.Position.Y, PosRatio.Y);
	NewVertex.Z = FMath::Lerp(FromV.Position.Z, ToV.Position.Z, PosRatio.Z);

	FColor NewColor;
	bool bAllHasVertexColor = FromV.bHasVertexColor && ToV.bHasVertexColor;
	if (bAllHasVertexColor)
	{
		NewColor.R = FMath::Lerp(FromV.VertexColor.R, ToV.VertexColor.R, PosRatio.Z);
		NewColor.G = FMath::Lerp(FromV.VertexColor.G, ToV.VertexColor.G, PosRatio.Z);
		NewColor.B = FMath::Lerp(FromV.VertexColor.B, ToV.VertexColor.B, PosRatio.Z);
		NewColor.A = FMath::Lerp(FromV.VertexColor.A, ToV.VertexColor.A, PosRatio.Z);
	}

	FVector2D NewUV0;
	bool bAllHasUV0 = FromV.bHasUV0 && ToV.bHasUV0;
	if (bAllHasUV0)
	{
		NewUV0.X = FMath::Lerp(FromV.UV0.X, ToV.UV0.X, AttrRatio);
		NewUV0.Y = FMath::Lerp(FromV.UV0.Y, ToV.UV0.Y, AttrRatio);
	}

	if (bAllHasVertexColor && bAllHasUV0)
		return FHexVertexData{ NewVertex, NewColor, NewUV0 };
	else if (bAllHasVertexColor)
		return FHexVertexData{ NewVertex, NewColor };
	else if (bAllHasUV0)
		return FHexVertexData{ NewVertex, NewUV0 };
	else
		return FHexVertexData{ NewVertex };
}

void FHexVertexData::SetUV0(const FVector2D& InUV0)
{
	UV0 = InUV0;
	bHasUV0 = true;
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

int32 FHexCellConfigData::DefaultElevation = 0;
EHexTerrainType FHexCellConfigData::DefaultTerrainType = EHexTerrainType::Water;

//////////////////////////////////////////////////////////////////////////

// Sets default values
AHexTerrainGenerator::AHexTerrainGenerator()
	: NoiseTexturePath(TEXT("Content/Noise.png"))
	, HexChunkCount(4, 3)
	, HexChunkSize(5, 5)
	, HexCellRadius(100.0f)
	, HexCellBorderWidth(10.0f)
	, HexCellSubdivision(3u)
	, HexElevationStep(5.0f)
	, MaxElevationForTerrace(4)
	, RiverElevationOffset(-1)
	, RiverSubdivision(2u)
	, PerturbingStrengthHV(1.0f, 1.0f)
	, PerturbingScalingHV(0.25f, 1.0f)

	, HexEditMode(EHexEditMode::Cell)
	, HexEditGridId(-1, -1)
	, HexEditElevation(0)
	, HexEditTerrainType(EHexTerrainType::None)
	, HexEditRiverId(-1)
	, HexEditRiverStartPoint(-1, -1)
	, HexEditRiverLastPoint(-1, -1)
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
	FCachedSectionData RiverSection;
	FCachedSectionData CollisionSection;

	ProceduralMeshComponent->ClearAllMeshSections();
	for (int32 CY = 0; CY < HexChunkCount.Y; ++CY)
	{
		for (int32 CX = 0; CX < HexChunkCount.X; ++CX)
		{
			int32 ChunkIndex = CY * HexChunkCount.X + CX;
			FCachedSectionData TerrainChunkSection;
			for (int32 GY = 0; GY < HexChunkSize.Y; ++GY)
			{
				for (int32 GX = 0; GX < HexChunkSize.X; ++GX)
				{
					int32 GridIndex = (CY * HexChunkSize.Y + GY) * HexGridSizeX + (CX * HexChunkSize.X + GX);
					const FHexCellData& CellData = HexGrids[GridIndex];

					FCachedSectionData TerrainMesh, RiverMesh, CollisionMesh;
					GenerateHexCell(CellData, TerrainMesh, RiverMesh, CollisionMesh);

					TerrainChunkSection.MeshSection(TerrainMesh);
					RiverSection.MeshSection(RiverMesh);
					CollisionSection.MeshSection(CollisionMesh);
				}
			}

			// Submit Mesh
			ProceduralMeshComponent->CreateMeshSection(ChunkIndex, TerrainChunkSection.GetVertices(), TerrainChunkSection.GetTriangles(), TerrainChunkSection.GetNormals(),
				TerrainChunkSection.GetUV0s(), TerrainChunkSection.GetVertexColors(), TerrainChunkSection.GetTangents(), false);

			// Set Material
			if (!!HexTerrainMaterial)
			{
				ProceduralMeshComponent->SetMaterial(ChunkIndex, HexTerrainMaterial);
			}
		}
	}

	// Create River
	if (!RiverSection.IsEmpty())
	{
		int32 RiverSectionId = ProceduralMeshComponent->GetNumSections();
		ProceduralMeshComponent->CreateMeshSection(RiverSectionId, RiverSection.GetVertices(), RiverSection.GetTriangles(),
			RiverSection.GetNormals(), RiverSection.GetUV0s(), RiverSection.GetVertexColors(), RiverSection.GetTangents(), false);
		//ProceduralMeshComponent->SetMeshSectionVisible(RiverSectionId, false);

		if (!!RiverMaterial)
		{
			ProceduralMeshComponent->SetMaterial(RiverSectionId, RiverMaterial);
		}
	}

	// Create Collision
	if (!CollisionSection.IsEmpty())
	{
		int32 CollisionSectionId = ProceduralMeshComponent->GetNumSections();
		ProceduralMeshComponent->CreateMeshSection(CollisionSectionId, CollisionSection.GetVertices(), CollisionSection.GetTriangles(),
			CollisionSection.GetNormals(), CollisionSection.GetUV0s(), CollisionSection.GetVertexColors(), CollisionSection.GetTangents(), false);
		ProceduralMeshComponent->SetMeshSectionVisible(CollisionSectionId, false);
	}

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

	TArray<TSharedPtr<FJsonValue>> ChunkSizeData = JsonRoot->GetArrayField(TEXT("ChunkSize"));
	ChunkSizeData[0]->TryGetNumber(HexChunkSize.X);
	ChunkSizeData[1]->TryGetNumber(HexChunkSize.Y);
	ChunkSizeData[2]->TryGetNumber(HexChunkCount.X);
	ChunkSizeData[3]->TryGetNumber(HexChunkCount.Y);

	TArray<TSharedPtr<FJsonValue>> ElevationsList = JsonRoot->GetArrayField(TEXT("Elevations"));
	TSharedPtr<FJsonObject> ColorsMap = JsonRoot->GetObjectField(TEXT("Colors"));
	TArray<TSharedPtr<FJsonValue>> TypesList = JsonRoot->GetArrayField(TEXT("HexTypes"));
	TArray<TSharedPtr<FJsonValue>> RiversList = JsonRoot->GetArrayField(TEXT("Rivers"));

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
	
	int32 NumOfRivers = RiversList.Num();
	ConfigData.RiversList.Empty(NumOfRivers);
	ConfigData.RiversList.AddDefaulted(NumOfRivers);
	for (int32 Index = 0; Index < NumOfRivers; ++Index)
	{
		FHexRiverConfigData& RiverConfig = ConfigData.RiversList[Index];
		TSharedPtr<FJsonObject> RiverData = RiversList[Index]->AsObject();

		TArray<TSharedPtr<FJsonValue>> StartPoint = RiverData->GetArrayField(TEXT("StartPoint"));
		StartPoint[0]->TryGetNumber(RiverConfig.RiverStartPoint.X);
		StartPoint[1]->TryGetNumber(RiverConfig.RiverStartPoint.Y);

		TArray<TSharedPtr<FJsonValue>> FlowDirections = RiverData->GetArrayField(TEXT("FlowDirections"));
		for (TSharedPtr<FJsonValue> Direction : FlowDirections)
		{
			uint8 DirectionId = 0u;
			Direction->TryGetNumber(DirectionId);
			RiverConfig.RiverFlowDirections.Add(static_cast<EHexDirection>(DirectionId));
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

	TArray<TSharedPtr<FJsonValue>> ChunkSizeData;
	ChunkSizeData.Add(MakeShared<FJsonValueNumber>(HexChunkSize.X));
	ChunkSizeData.Add(MakeShared<FJsonValueNumber>(HexChunkSize.Y));
	ChunkSizeData.Add(MakeShared<FJsonValueNumber>(HexChunkCount.X));
	ChunkSizeData.Add(MakeShared<FJsonValueNumber>(HexChunkCount.Y));

	TArray<TSharedPtr<FJsonValue>> ElevationsList;
	TArray<TSharedPtr<FJsonValue>> TypesList;

	for (int32 Y = 0; Y < HexGridSizeY; ++Y)
	{
		TArray<TSharedPtr<FJsonValue>> ElevationRow;
		TArray<TSharedPtr<FJsonValue>> TypeRow;

		for (int32 X = 0; X < HexGridSizeX; ++X)
		{
			ElevationRow.Add(MakeShared<FJsonValueNumber>(ConfigData.ElevationsList[Y][X]));
			TypeRow.Add(MakeShared<FJsonValueNumber>(uint8(ConfigData.TerrainTypesList[Y][X])));
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
	
	TArray<TSharedPtr<FJsonValue>> RiversList;
	int32 NumOfRivers = ConfigData.RiversList.Num();
	for (int32 Index = 0; Index < NumOfRivers; ++Index)
	{
		FHexRiverConfigData& RiverConfig = ConfigData.RiversList[Index];
		TSharedRef<FJsonObject> RiverData = MakeShareable(new FJsonObject());
		
		TArray<TSharedPtr<FJsonValue>> StartPoint;
		StartPoint.Add(MakeShared<FJsonValueNumber>(RiverConfig.RiverStartPoint.X));
		StartPoint.Add(MakeShared<FJsonValueNumber>(RiverConfig.RiverStartPoint.Y));
		RiverData->SetArrayField(TEXT("StartPoint"), StartPoint);
		
		TArray<TSharedPtr<FJsonValue>> FlowDirections;
		for (EHexDirection Direction : RiverConfig.RiverFlowDirections)
		{
			FlowDirections.Add(MakeShared<FJsonValueNumber>(static_cast<uint8>(Direction)));
		}
		RiverData->SetArrayField(TEXT("FlowDirections"), FlowDirections);

		RiversList.Add(MakeShared<FJsonValueObject>(RiverData));
	}

	JsonObject->SetArrayField(TEXT("ChunkSize"), ChunkSizeData);
	JsonObject->SetArrayField(TEXT("Elevations"), ElevationsList);
	JsonObject->SetObjectField(TEXT("Colors"), ColorsMap);
	JsonObject->SetArrayField(TEXT("HexTypes"), TypesList);
	JsonObject->SetArrayField(TEXT("Rivers"), RiversList);

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

			//OneCell.LinearColor = FLinearColor::MakeRandomColor();
			//OneCell.SRGBColor = OneCell.LinearColor.ToFColorSRGB();
			ConfigData.GetHexCellTerrainData(GridId, OneCell.SRGBColor, OneCell.Elevation);
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
		const FHexRiverConfigData& OneRiver = ConfigData.RiversList[Index];
		int32 LenOfRiver = OneRiver.RiverFlowDirections.Num();

		int32 FirstIndex = OneRiver.RiverStartPoint.Y * HexGridSizeX + OneRiver.RiverStartPoint.X;
		if (FirstIndex >= NumOfGrids)
			continue;

		FHexCellData& FirstRiverNode = HexGrids[FirstIndex];
		FirstRiverNode.HexRiver.RiverIndex = Index;
		FirstRiverNode.HexRiver.RiverState = EHexRiverState::StartPoint;
		//FirstRiverNode.HexRiver.RiverColor = (Index == HexEditRiverId ? FColor{ 55u, 110u, 225u, 255u } : FColor{ 0u, 55u, 225u, 255u });

		FHexCellData* LastRiverNode = &FirstRiverNode;
		for (int32 Step = 0; Step < LenOfRiver; ++Step)
		{
			EHexDirection StepDirection = OneRiver.RiverFlowDirections[Step];

			int32 CurGridIndex = LastRiverNode->HexNeighbors[static_cast<uint8>(StepDirection)].LinkedCellIndex;
			if (CurGridIndex < 0)
			{
				LastRiverNode->HexRiver.RiverState = EHexRiverState::EndPoint;
				break;
			}

			FHexCellData& CurRiverNode = HexGrids[CurGridIndex];
			CurRiverNode.HexRiver.RiverIndex = Index;
			CurRiverNode.HexRiver.RiverState = (Step == LenOfRiver - 1) ? EHexRiverState::EndPoint : EHexRiverState::PassThrough;
			//CurRiverNode.HexRiver.RiverColor = FirstRiverNode.HexRiver.RiverColor;

			LastRiverNode->HexRiver.OutgoingDirection = StepDirection;
			CurRiverNode.HexRiver.IncomingDirection = FHexCellData::CalcOppositeDirection(StepDirection);

			LastRiverNode = &CurRiverNode;
		}
	}
}

void AHexTerrainGenerator::GenerateHexCell(const FHexCellData& InCellData, FCachedSectionData& OutTerrainMesh, FCachedSectionData& OutRiverMesh, FCachedSectionData& OutCellCollisionMesh)
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
	OutTerrainMesh.Reset(NumOfVerts);

	// Inner HexCell
	GenerateHexCenter(InCellData, OutTerrainMesh, OutRiverMesh);

	//OutCellCollisionMesh = OutTerrainMesh;

	// Border
	int32 WIndex = InCellData.HexNeighbors[static_cast<uint8>(EHexDirection::W)].LinkedCellIndex;
	int32 NWIndex = InCellData.HexNeighbors[static_cast<uint8>(EHexDirection::NW)].LinkedCellIndex;
	int32 NEIndex = InCellData.HexNeighbors[static_cast<uint8>(EHexDirection::NE)].LinkedCellIndex;
	if (WIndex >= 0) // W Edge
	{
		GenerateHexBorder(InCellData, EHexDirection::W, OutTerrainMesh, OutRiverMesh);
	}

	if (NWIndex >= 0) // NW Edge
	{
		GenerateHexBorder(InCellData, EHexDirection::NW, OutTerrainMesh, OutRiverMesh);
	}

	if (NEIndex >= 0) // NE Edge
	{
		GenerateHexBorder(InCellData, EHexDirection::NE, OutTerrainMesh, OutRiverMesh);
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

void AHexTerrainGenerator::GenerateHexCenter(const FHexCellData& InCellData, FCachedSectionData& OutTerrainMesh, FCachedSectionData& OutRiverMesh)
{
	switch (InCellData.HexRiver.RiverState)
	{
	case EHexRiverState::None:
		GenerateNoRiverCenter(InCellData, OutTerrainMesh);
		break;

	case EHexRiverState::StartPoint:
	case EHexRiverState::EndPoint:
		GenerateCenterWithRiverEnd(InCellData, OutTerrainMesh, OutRiverMesh);
		break;

	case EHexRiverState::PassThrough:
		GenerateCenterWithRiverThrough(InCellData, OutTerrainMesh, OutRiverMesh);
		break;
	}
}

void AHexTerrainGenerator::GenerateHexBorder(const FHexCellData& InCellData, EHexDirection BorderDirection, FCachedSectionData& OutTerrainMesh, FCachedSectionData& OutRiverMesh)
{
	uint8 BorderDirectionId = static_cast<uint8>(BorderDirection);
	const FHexCellBorder& HexBorder = InCellData.HexNeighbors[BorderDirectionId];
	const FHexCellData& OppositeCell = HexGrids[HexBorder.LinkedCellIndex];
	
	TArray<FHexVertexData> FromVerts;
	TArray<FHexVertexData> ToVerts;
	TArray<bool> bRiverVerts;

	FromVerts.Add(FHexVertexData{ CalcHexCellVertex(InCellData, HexBorder.FromVert.X, false), InCellData.SRGBColor });
	ToVerts.Add(FHexVertexData{ CalcHexCellVertex(OppositeCell, HexBorder.ToVert.X, false), OppositeCell.SRGBColor });
	bRiverVerts.Add(false);

	const FColor& WaterColor = ConfigData.ColorsMap[EHexTerrainType::Water];
	for (int32 SubIndex = 0; SubIndex < HexCellSubdivision; ++SubIndex)
	{
		bool bRiverVertex = false;
		int32 SubVertIndex = HexBorder.FromVert.X * HexCellSubdivision + SubIndex;
		FVector FromVertex = CalcHexCellVertex(InCellData, SubVertIndex, true, bRiverVertex);
		FromVerts.Add(FHexVertexData{ FromVertex, bRiverVertex ? WaterColor : InCellData.SRGBColor });
		bRiverVerts.Add(bRiverVertex);

		SubVertIndex = HexBorder.ToVert.Y * HexCellSubdivision + (HexCellSubdivision - SubIndex - 1);
		ToVerts.Add(FHexVertexData{ CalcHexCellVertex(OppositeCell, SubVertIndex, true), bRiverVertex ? WaterColor : OppositeCell.SRGBColor });
	}

	FromVerts.Add(FHexVertexData{ CalcHexCellVertex(InCellData, HexBorder.FromVert.Y, false), InCellData.SRGBColor });
	ToVerts.Add(FHexVertexData{ CalcHexCellVertex(OppositeCell, HexBorder.ToVert.Y, false), OppositeCell.SRGBColor });
	bRiverVerts.Add(false);
	
	int32 NumOfVerts = FromVerts.Num();
	int32 NumOfZSteps = FMath::Abs(OppositeCell.Elevation - InCellData.Elevation);
	if (HexBorder.LinkState == EHexBorderState::Terrace)
	{
		int32 NumOfSegments = NumOfVerts - 1;
		for (int32 Index = 0; Index < NumOfSegments; ++Index)
		{
			FillStrip(FromVerts[Index], FromVerts[Index + 1], ToVerts[Index], ToVerts[Index + 1], OutTerrainMesh, NumOfZSteps, true);
		}
	}
	else
	{
		for (int32 Index = 0; Index < NumOfVerts; ++Index)
		{
			PerturbingVertexInline(FromVerts[Index].Position);
			PerturbingVertexInline(ToVerts[Index].Position);
		}

		int32 NumOfSegments = NumOfVerts - 1;
		for (int32 Index = 0; Index < NumOfSegments; ++Index)
		{
			FillQuad(FromVerts[Index], FromVerts[Index + 1], ToVerts[Index], ToVerts[Index + 1], OutTerrainMesh);
		}
	}

	// River
	int32 RiverIndex = -1;
	for (int32 Index = 0; Index < NumOfVerts; ++Index)
	{
		if (bRiverVerts[Index])
		{
			RiverIndex = Index;
			break;
		}
	}

	if (RiverIndex >= 0)
	{
		FHexVertexData FromEdgeL = FromVerts[RiverIndex - 1];
		FHexVertexData FromEdgeR = FromVerts[RiverIndex + 1];
		FHexVertexData ToEdgeL = ToVerts[RiverIndex - 1];
		FHexVertexData ToEdgeR = ToVerts[RiverIndex + 1];

		FVector WaterZOffset = CalcRiverVertOffset(true);
		FromEdgeL.Position += WaterZOffset;
		FromEdgeR.Position += WaterZOffset;
		ToEdgeL.Position += WaterZOffset;
		ToEdgeR.Position += WaterZOffset;

		float UVScale = CalcRiverUVScale(true, NumOfZSteps);

		bool bHasOutRiver = BorderDirection == InCellData.HexRiver.OutgoingDirection;
		if (bHasOutRiver)
		{
			FromEdgeL.SetUV0(FVector2D{ 0.0, 0.0 });
			FromEdgeR.SetUV0(FVector2D{ 0.0, 1.0 });
			ToEdgeL.SetUV0(FVector2D{ UVScale, 0.0 });
			ToEdgeR.SetUV0(FVector2D{ UVScale, 1.0 });
		}
		else
		{
			FromEdgeL.SetUV0(FVector2D{ UVScale, 1.0 });
			FromEdgeR.SetUV0(FVector2D{ UVScale, 0.0 });
			ToEdgeL.SetUV0(FVector2D{ 0.0, 1.0 });
			ToEdgeR.SetUV0(FVector2D{ 0.0, 0.0 });
		}

		FillQuad(FromEdgeL, FromEdgeR, ToEdgeL, ToEdgeR, OutRiverMesh);
	}
}

void AHexTerrainGenerator::GenerateHexCorner(const FHexCellData& InCellData, EHexDirection CornerDirection, FCachedSectionData& OutTerrainMesh)
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

void AHexTerrainGenerator::GenerateNoRiverCenter(const FHexCellData& InCellData, FCachedSectionData& OutTerrainMesh)
{
	TArray<FHexVertexData> EgdesV;
	for (int32 EdgeIndex = 0; EdgeIndex < CORNER_NUM; ++EdgeIndex)
	{
		for (int32 SubIndex = 0; SubIndex <= HexCellSubdivision; ++SubIndex)
		{
			bool bRiverVert = false;
			FVector CurVertex;
			if (SubIndex == 0)
				CurVertex = CalcHexCellVertex(InCellData, EdgeIndex, false);
			else
				CurVertex = CalcHexCellVertex(InCellData, EdgeIndex * HexCellSubdivision + SubIndex - 1, true, bRiverVert);

			PerturbingVertexInline(CurVertex);

			EgdesV.Add(FHexVertexData{ 
				CurVertex, 
				bRiverVert ? ConfigData.ColorsMap[EHexTerrainType::Water] : InCellData.SRGBColor,
				FVector::UpVector
			});
		}
	}

	FHexVertexData CenterV{ InCellData.CellCenter, InCellData.SRGBColor, FVector::UpVector };
	PerturbingVertexInline(CenterV.Position);

	TArray<bool> Dummy;
	FillFan(CenterV, EgdesV, Dummy, OutTerrainMesh, true);
}

void AHexTerrainGenerator::GenerateCenterWithRiverEnd(const FHexCellData& InCellData, FCachedSectionData& OutTerrainMesh, FCachedSectionData& OutRiverMesh)
{
	TArray<FHexVertexData> EgdesV;
	TArray<bool> ShouldRecalcNormal;
	for (int32 EdgeIndex = 0; EdgeIndex < CORNER_NUM; ++EdgeIndex)
	{
		for (int32 SubIndex = 0; SubIndex <= HexCellSubdivision; ++SubIndex)
		{
			bool bRiverVert = false;
			FVector CurVertex;
			if (SubIndex == 0)
				CurVertex = CalcHexCellVertex(InCellData, EdgeIndex, false);
			else
				CurVertex = CalcHexCellVertex(InCellData, EdgeIndex * HexCellSubdivision + SubIndex - 1, true, bRiverVert);
			
			EgdesV.Add(FHexVertexData{
				CurVertex,
				bRiverVert ? ConfigData.ColorsMap[EHexTerrainType::Water] : InCellData.SRGBColor 
			});
			ShouldRecalcNormal.Add(bRiverVert);
		}
	}
	
	int32 NumOfEdges = EgdesV.Num();
	FHexVertexData CenterV{ InCellData.CellCenter, InCellData.SRGBColor };

	TArray<FHexVertexData> CentersV;
	CentersV.Init(CenterV, NumOfEdges);

	FillGrid(CentersV, EgdesV, OutTerrainMesh, RiverSubdivision, false, true);

	// River
	int32 RiverIndex = -1;
	for (int32 Index = 0; Index < NumOfEdges; ++Index)
	{
		if (ShouldRecalcNormal[Index])
		{
			RiverIndex = Index;
			break;
		}
	}

	if (RiverIndex >= 0)
	{
		FHexVertexData CopiedEdgeL = EgdesV[RiverIndex - 1];
		FHexVertexData CopiedEdgeR = EgdesV[RiverIndex + 1];

		FVector WaterZOffset = CalcRiverVertOffset(true);
		CopiedEdgeL.Position += WaterZOffset;
		CopiedEdgeR.Position += WaterZOffset;

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

		FillQuad(CenterV, CenterV, CopiedEdgeL, CopiedEdgeR, OutRiverMesh);
	}
}

void AHexTerrainGenerator::GenerateCenterWithRiverThrough(const FHexCellData& InCellData, FCachedSectionData& OutTerrainMesh, FCachedSectionData& OutRiverMesh)
{
	EHexDirection InDirection = InCellData.HexRiver.IncomingDirection;
	EHexDirection OutDirection = InCellData.HexRiver.OutgoingDirection;

	uint8 InMVertId = FHexCellData::GetVertIdFromDirection(InDirection, true, 1u);
	uint8 OutMVertId = FHexCellData::GetVertIdFromDirection(OutDirection, true, 1u);

	FVector InDir = FHexCellData::HexSubVertices[InMVertId];
	FVector OutDir = FHexCellData::HexSubVertices[OutMVertId];

	FVector LeftDir;
	FVector RightDir;

	float MoveDist = HexCellRadius / float(HexCellSubdivision + 1);
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

	FHexVertexData CenterL = FHexVertexData{ InCellData.CellCenter + LeftDir * MoveDist, InCellData.SRGBColor };
	FHexVertexData Center = FHexVertexData{ InCellData.CellCenter + (LeftDir + RightDir) * 0.5 * MoveDist + CalcRiverVertOffset(), WaterColor };
	FHexVertexData CenterR = FHexVertexData{ InCellData.CellCenter + RightDir * MoveDist, InCellData.SRGBColor };
	
	bool bSharpLeftTurn = OutDirection == FHexCellData::CalcNextDirection(InDirection);
	bool bSharpRightTurn = OutDirection == FHexCellData::CalcPreviousDirection(InDirection);

	auto GenerateRiverFan = [this, WaterColor](const FHexCellData& InCellData, FCachedSectionData& OutTerrainMesh, FCachedSectionData& OutRiverMesh,
		const FHexVertexData& InCenterL, const FHexVertexData& InCenter, const FHexVertexData& InCenterR, EHexDirection Direction) -> void
		{
			uint8 SVertId = FHexCellData::GetVertIdFromDirection(Direction, true, 0u);
			uint8 MVertId = FHexCellData::GetVertIdFromDirection(Direction, true, 1u);
			uint8 EVertId = FHexCellData::GetVertIdFromDirection(Direction, true, 2u);

			TArray<bool> Dummy;

			// Left Fan
			TArray<FHexVertexData> EgdesLV;
			uint8 MainVertL = FHexCellData::GetVertIdFromDirection(Direction, false);
			EgdesLV.Add(FHexVertexData{ CalcHexCellVertex(InCellData, MainVertL, false), InCellData.SRGBColor });
			for (uint8 Index = SVertId; Index <= MVertId - 1u; ++Index)
			{
				EgdesLV.Add(FHexVertexData{ CalcHexCellVertex(InCellData, Index, true), InCellData.SRGBColor });
			}

			TArray<FHexVertexData> CentersLV;
			CentersLV.Init(InCenterL, EgdesLV.Num());
			
			FillGrid(CentersLV, EgdesLV, OutTerrainMesh, RiverSubdivision);

			// Center Two Quads
			FHexVertexData EdgeL = FHexVertexData{CalcHexCellVertex(InCellData, MVertId - 1u, true), InCellData.SRGBColor };
			FHexVertexData EdgeC = FHexVertexData{CalcHexCellVertex(InCellData, MVertId, true), WaterColor };
			FHexVertexData EdgeR = FHexVertexData{CalcHexCellVertex(InCellData, MVertId + 1u, true), InCellData.SRGBColor };

			FillStrip(InCenterL, InCenter, EdgeL, EdgeC, OutTerrainMesh, RiverSubdivision);
			FillStrip(InCenter, InCenterR, EdgeC, EdgeR, OutTerrainMesh, RiverSubdivision);

			// Right Fan
			TArray<FHexVertexData> EgdesRV;
			for (uint8 Index = MVertId + 1u; Index <= EVertId; ++Index)
			{
				EgdesRV.Add(FHexVertexData{ CalcHexCellVertex(InCellData, Index, true),InCellData.SRGBColor });
			}
			uint8 MainVertR = FHexCellData::GetVertIdFromDirection(FHexCellData::CalcNextDirection(Direction), false);
			EgdesRV.Add(FHexVertexData{ CalcHexCellVertex(InCellData, MainVertR, false), InCellData.SRGBColor });

			TArray<FHexVertexData> CentersRV;
			CentersRV.Init(InCenterR, EgdesRV.Num());

			FillGrid(CentersRV, EgdesRV, OutTerrainMesh, RiverSubdivision);

			// River
			FHexVertexData CopiedCenterL = InCenterL;
			FHexVertexData CopiedCenterR = InCenterR;

			FVector WaterZOffset = CalcRiverVertOffset(true);
			CopiedCenterL.Position += WaterZOffset;
			CopiedCenterR.Position += WaterZOffset;
			EdgeL.Position += WaterZOffset;
			EdgeR.Position += WaterZOffset;

			float UVScale = CalcRiverUVScale();

			if (Direction == InCellData.HexRiver.OutgoingDirection)
			{
				CopiedCenterL.SetUV0(FVector2D{ 0.0, 0.0 });
				CopiedCenterR.SetUV0(FVector2D{ 0.0, 1.0 });
				EdgeL.SetUV0(FVector2D{ UVScale, 0.0 });
				EdgeR.SetUV0(FVector2D{ UVScale, 1.0 });
			}
			else
			{
				CopiedCenterL.SetUV0(FVector2D{ UVScale, 1.0 });
				CopiedCenterR.SetUV0(FVector2D{ UVScale, 0.0 });
				EdgeL.SetUV0(FVector2D{ 0.0, 1.0 });
				EdgeR.SetUV0(FVector2D{ 0.0, 0.0 });
			}

			FillStrip(CopiedCenterL, CopiedCenterR, EdgeL, EdgeR, OutRiverMesh, RiverSubdivision);
		};
	
	auto GenerateSharpRiverFan = [this, WaterColor](const FHexCellData& InCellData, FCachedSectionData& OutTerrainMesh, FCachedSectionData& OutRiverMesh,
		const FHexVertexData& InCenterL, const FHexVertexData& InCenter, const FHexVertexData& InCenterR, EHexDirection Direction, bool bMoveLeft) -> void
		{
			uint8 SVertId = FHexCellData::GetVertIdFromDirection(Direction, true, 0u);
			uint8 MVertId = FHexCellData::GetVertIdFromDirection(Direction, true, 1u);
			uint8 EVertId = FHexCellData::GetVertIdFromDirection(Direction, true, 2u);

			// Left Fan
			TArray<FHexVertexData> EgdesLV;
			uint8 MainVertL = FHexCellData::GetVertIdFromDirection(Direction, false);
			EgdesLV.Add(FHexVertexData{ CalcHexCellVertex(InCellData, MainVertL, false), InCellData.SRGBColor });
			for (uint8 Index = SVertId; Index <= MVertId - 2u; ++Index)
			{
				EgdesLV.Add(FHexVertexData{ CalcHexCellVertex(InCellData, Index, true), InCellData.SRGBColor });
			}

			TArray<FHexVertexData> CentersLV;
			CentersLV.Init(InCenterL, EgdesLV.Num());

			FillGrid(CentersLV, EgdesLV, OutTerrainMesh, RiverSubdivision);

			// Center Two Quads
			uint8 MainVertIdL = FHexCellData::GetVertIdFromDirection(Direction, false);
			uint8 MainVertIdR = FHexCellData::GetVertIdFromDirection(FHexCellData::CalcNextDirection(Direction), false);

			FHexVertexData EdgeL2{ MVertId - 2u < SVertId ? CalcHexCellVertex(InCellData, MainVertIdL, false) : CalcHexCellVertex(InCellData, MVertId - 2u, true), InCellData.SRGBColor };
			FHexVertexData EdgeL1{ CalcHexCellVertex(InCellData, MVertId - 1u, true), InCellData.SRGBColor };
			FHexVertexData EdgeC{ CalcHexCellVertex(InCellData, MVertId, true), WaterColor };
			FHexVertexData EdgeR1{ CalcHexCellVertex(InCellData, MVertId + 1u, true), InCellData.SRGBColor };
			FHexVertexData EdgeR2{ MVertId + 2u > EVertId ? CalcHexCellVertex(InCellData, MainVertIdR, false) : CalcHexCellVertex(InCellData, MVertId + 2u, true), InCellData.SRGBColor };

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

			PerturbingVertexInline(CenterL.Position);
			PerturbingVertexInline(Center.Position);
			PerturbingVertexInline(CenterR.Position);

			PerturbingVertexInline(EdgeL2.Position);
			PerturbingVertexInline(EdgeL1.Position);
			PerturbingVertexInline(EdgeC.Position);
			PerturbingVertexInline(EdgeR1.Position);
			PerturbingVertexInline(EdgeR2.Position);

			PerturbingVertexInline(MidL.Position);
			PerturbingVertexInline(MidC.Position);
			PerturbingVertexInline(MidR.Position);
			PerturbingVertexInline(MidE.Position);

			if (bMoveLeft)
			{
				FillQuad(MidL, MidL, EdgeL2, EdgeL1, OutTerrainMesh);
			}
			else
			{
				FillQuad(CenterL, CenterL, MidE, MidL, OutTerrainMesh);
				FillQuad(MidE, MidL, EdgeL2, EdgeL1, OutTerrainMesh);
			}

			FillQuad(CenterL, Center, MidL, MidC, OutTerrainMesh);
			FillQuad(MidL, MidC, EdgeL1, EdgeC, OutTerrainMesh);

			FillQuad(Center, CenterR, MidC, MidR, OutTerrainMesh);
			FillQuad(MidC, MidR, EdgeC, EdgeR1, OutTerrainMesh);

			if (bMoveLeft)
			{
				FillQuad(CenterR, CenterR, MidR, MidE, OutTerrainMesh);
				FillQuad(MidR, MidE, EdgeR1, EdgeR2, OutTerrainMesh);
			}
			else
			{
				FillQuad(MidR, MidR, EdgeR1, EdgeR2, OutTerrainMesh);
			}

			// Right Fan
			TArray<FHexVertexData> EgdesRV;
			for (uint8 Index = MVertId + 2u; Index <= EVertId; ++Index)
			{
				EgdesRV.Add(FHexVertexData{ CalcHexCellVertex(InCellData, Index, true),InCellData.SRGBColor });
			}
			uint8 MainVertR = FHexCellData::GetVertIdFromDirection(FHexCellData::CalcNextDirection(Direction), false);
			EgdesRV.Add(FHexVertexData{ CalcHexCellVertex(InCellData, MainVertR, false), InCellData.SRGBColor });

			TArray<FHexVertexData> CentersRV;
			CentersRV.Init(InCenterR, EgdesRV.Num());

			FillGrid(CentersRV, EgdesRV, OutTerrainMesh, RiverSubdivision);

			// River
			FVector WaterZOffset = CalcRiverVertOffset(true);
			CenterL.Position += WaterZOffset;
			CenterR.Position += WaterZOffset;
			MidL.Position += WaterZOffset;
			MidR.Position += WaterZOffset;
			EdgeL1.Position += WaterZOffset;
			EdgeR1.Position += WaterZOffset;

			float UVScale = CalcRiverUVScale();

			if (Direction == InCellData.HexRiver.OutgoingDirection)
			{
				CenterL.SetUV0(FVector2D{ 0.0, 0.0 });
				CenterR.SetUV0(FVector2D{ 0.0, 1.0 });
				MidL.SetUV0(FVector2D{ 0.5 * UVScale, 0.0 });
				MidR.SetUV0(FVector2D{ 0.5 * UVScale, 1.0 });
				EdgeL1.SetUV0(FVector2D{ UVScale, 0.0 });
				EdgeR1.SetUV0(FVector2D{ UVScale, 1.0 });
			}
			else
			{
				CenterL.SetUV0(FVector2D{ UVScale, 1.0 });
				CenterR.SetUV0(FVector2D{ UVScale, 0.0 });
				MidL.SetUV0(FVector2D{ 0.5 * UVScale, 1.0 });
				MidR.SetUV0(FVector2D{ 0.5 * UVScale, 0.0 });
				EdgeL1.SetUV0(FVector2D{ 0.0, 1.0 });
				EdgeR1.SetUV0(FVector2D{ 0.0, 0.0 });
			}

			FillQuad(CenterL, CenterR, MidL, MidR, OutRiverMesh);
			FillQuad(MidL, MidR, EdgeL1, EdgeR1, OutRiverMesh);
		};

	if (bSharpLeftTurn || bSharpRightTurn)
	{
		GenerateSharpRiverFan(InCellData, OutTerrainMesh, OutRiverMesh, CenterL, Center, CenterR, OutDirection, bSharpRightTurn);
		GenerateSharpRiverFan(InCellData, OutTerrainMesh, OutRiverMesh, CenterR, Center, CenterL, InDirection, !bSharpRightTurn);
	}
	else
	{
		GenerateRiverFan(InCellData, OutTerrainMesh, OutRiverMesh, CenterL, Center, CenterR, OutDirection);
		GenerateRiverFan(InCellData, OutTerrainMesh, OutRiverMesh, CenterR, Center, CenterL, InDirection);
	}

	auto GenerateFansWithoutRiver = [this](const FHexCellData& InCellData, FCachedSectionData& OutTerrainMesh,
		EHexDirection FromDirection, EHexDirection ToDirection, const FHexVertexData& InCenter) -> void
		{
			TArray<FHexVertexData> EdgesV;

			EHexDirection CurDirection = FHexCellData::CalcNextDirection(FromDirection);
			while (CurDirection != ToDirection)
			{
				uint8 EdgeIndex = FHexCellData::GetVertIdFromDirection(CurDirection, false);
				for (int32 SubIndex = 0; SubIndex <= HexCellSubdivision; ++SubIndex)
				{
					FVector CurVertex;
					if (SubIndex == 0)
						CurVertex = CalcHexCellVertex(InCellData, EdgeIndex, false);
					else
						CurVertex = CalcHexCellVertex(InCellData, EdgeIndex * HexCellSubdivision + SubIndex - 1, true);

					EdgesV.Add(FHexVertexData{ CurVertex, InCellData.SRGBColor });
				}

				CurDirection = FHexCellData::CalcNextDirection(CurDirection);
			}
			{
				uint8 EdgeIndex = FHexCellData::GetVertIdFromDirection(CurDirection, false);
				FVector CurVertex = CalcHexCellVertex(InCellData, EdgeIndex, false);
				EdgesV.Add(FHexVertexData{ CurVertex,InCellData.SRGBColor });
			}

			TArray<FHexVertexData> CentersV;
			CentersV.Init(InCenter, EdgesV.Num());

			FillGrid(CentersV, EdgesV, OutTerrainMesh, RiverSubdivision, false, false);
		};

	GenerateFansWithoutRiver(InCellData, OutTerrainMesh, OutDirection, InDirection, CenterR);
	GenerateFansWithoutRiver(InCellData, OutTerrainMesh, InDirection, OutDirection, CenterL);
}

void AHexTerrainGenerator::GenerateNoTerraceCorner(const FHexCellData& Cell1, const FHexCellData& Cell2, const FHexCellData& Cell3,
	const FHexCellCorner& CornerData, FCachedSectionData& OutTerrainMesh)
{
	FHexVertexData V0{ CalcHexCellVertex(Cell1, CornerData.VertsId.X, false), Cell1.SRGBColor };
	FHexVertexData V1{ CalcHexCellVertex(Cell2, CornerData.VertsId.Y, false), Cell2.SRGBColor };
	FHexVertexData V2{ CalcHexCellVertex(Cell3, CornerData.VertsId.Z, false), Cell3.SRGBColor };

	PerturbingVertexInline(V0.Position);
	PerturbingVertexInline(V1.Position);
	PerturbingVertexInline(V2.Position);

	OutTerrainMesh.AddTriangle(V0, V1, V2);
}

void AHexTerrainGenerator::GenerateCornerWithTerrace(const FHexCellData& Cell1, const FHexCellData& Cell2, const FHexCellData& Cell3, const FHexCellCorner& CornerData, FCachedSectionData& OutTerrainMesh)
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

	FHexVertexData Vert0{ CalcHexCellVertex(*CellsList[0], VertsList[0], false), CellsList[0]->SRGBColor };
	FHexVertexData Vert1{ CalcHexCellVertex(*CellsList[1], VertsList[1], false), CellsList[1]->SRGBColor };
	FHexVertexData Vert2{ CalcHexCellVertex(*CellsList[2], VertsList[2], false), CellsList[2]->SRGBColor };
	
	FHexVertexData DisturbedVert0 = Vert0;
	FHexVertexData DisturbedVert1 = Vert1;
	FHexVertexData DisturbedVert2 = Vert2;
	PerturbingVertexInline(DisturbedVert0.Position);
	PerturbingVertexInline(DisturbedVert1.Position);
	PerturbingVertexInline(DisturbedVert2.Position);

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
				PerturbingVertexInline(CurStepVert.Position);

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
	Verts01[0].Add(DisturbedVert0);
	Verts02.AddDefaulted();
	Verts02[0].Add(DisturbedVert0);

	if (LinkState[0] == EHexBorderState::Terrace)
	{
		CalcTerraceVerts(Verts01, Vert1, Vert0, CellsList[1]->Elevation, CellsList[0]->Elevation);
	}
	else
	{
		CalcLinearVerts(Verts01, DisturbedVert1, DisturbedVert0, CellsList[1]->Elevation, CellsList[0]->Elevation);
	}

	if (LinkState[2] == EHexBorderState::Terrace)
	{
		CalcTerraceVerts(Verts02, Vert2, Vert0, CellsList[2]->Elevation, CellsList[0]->Elevation);
	}
	else
	{
		CalcLinearVerts(Verts02, DisturbedVert2, DisturbedVert0, CellsList[2]->Elevation, CellsList[0]->Elevation);
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
			CalcLinearVerts(Verts01, DisturbedVert2, DisturbedVert1, CellsList[2]->Elevation, CellsList[1]->Elevation);
		}
		else if (CellsList[1]->Elevation > CellsList[2]->Elevation)// 2 -> 1
		{
			CalcLinearVerts(Verts02, DisturbedVert1, DisturbedVert2, CellsList[1]->Elevation, CellsList[2]->Elevation);
		}
	}
	
	int32 NumOfLayers01 = Verts01.Num();
	int32 NumOfLayers02 = Verts02.Num();
	check(NumOfLayers01 == NumOfLayers02);

	for (int32 Index = 1; Index < NumOfLayers01; ++Index)
	{
		// Cross Elevation
		FillQuad(Verts02[Index - 1].Last(), Verts01[Index - 1].Last(), Verts02[Index][0], Verts01[Index][0], OutTerrainMesh);

		// Current Elevation
		FillQuad(Verts02[Index][0], Verts01[Index][0], Verts02[Index].Last(), Verts01[Index].Last(), OutTerrainMesh);
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

FVector AHexTerrainGenerator::CalcHexCellVertex(const FHexCellData& InCellData, int32 VertIndex, bool bSubVert) const
{
	bool Dummy;
	return CalcHexCellVertex(InCellData, VertIndex, bSubVert, Dummy);
}

FVector AHexTerrainGenerator::CalcHexCellVertex(const FHexCellData& InCellData, int32 VertIndex, bool bSubVert, bool& bOutRiverVert) const
{
	bOutRiverVert = false;
	if (bSubVert)
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
		}

		if (RiverVerts.Contains(VertIndex))
		{
			bOutRiverVert = true;
			return InCellData.CellCenter + FHexCellData::HexSubVertices[VertIndex] + CalcRiverVertOffset();
		}
		return InCellData.CellCenter + FHexCellData::HexSubVertices[VertIndex];
	}
	else
		return InCellData.CellCenter + FHexCellData::HexVertices[VertIndex];
}

FIntPoint AHexTerrainGenerator::CalcHexCellGridId(const FVector& WorldPos)
{
	static FVector2D VertOffsetScale{ 1.732050807568877, 1.5 };
	float CellOuterRadius = HexCellRadius + HexCellBorderWidth;

	FIntPoint GridId;
	GridId.Y = FMath::RoundToInt(WorldPos.Y / (CellOuterRadius * VertOffsetScale.Y));
	GridId.X = FMath::RoundToInt(WorldPos.X / (CellOuterRadius * VertOffsetScale.X) - (GridId.Y % 2) * 0.5);
	return GridId;
}

FVector AHexTerrainGenerator::CalcFaceNormal(const FVector& V0, const FVector& V1, const FVector& V2)
{
	FVector Edge1 = (V1 - V0);
	FVector Edge2 = (V2 - V0);
	FVector NormalVector = FVector::CrossProduct(Edge1, Edge2);
	return NormalVector.GetSafeNormal();
}

void AHexTerrainGenerator::FillGrid(const TArray<FHexVertexData>& FromV, const TArray<FHexVertexData>& ToV, FCachedSectionData& OutTerrainMesh,
	int32 NumOfSteps, bool bTerrace, bool bClosed)
{
	int32 NumOfStrips = FromV.Num() - 1;
	for (int32 Index = 0; Index < NumOfStrips; ++Index)
	{
		FillStrip(
			FromV[Index], FromV[Index + 1], ToV[Index], ToV[Index + 1],
			OutTerrainMesh, NumOfSteps, bTerrace
		);
	}

	if (bClosed)
	{
		FillStrip(
			FromV[NumOfStrips], FromV[0], ToV[NumOfStrips], ToV[0],
			OutTerrainMesh, NumOfSteps, bTerrace
		);
	}
}

void AHexTerrainGenerator::FillStrip(const FHexVertexData& FromV0, const FHexVertexData& FromV1, const FHexVertexData& ToV0, const FHexVertexData& ToV1,
	FCachedSectionData& OutTerrainMesh, int32 NumOfSteps, bool bTerrace)
{
	FHexVertexData LastStepV0 = FromV0;
	FHexVertexData LastStepV1 = FromV1;
	int32 NumOfFinalSteps = bTerrace ? NumOfSteps * 2 - 1 : NumOfSteps;

	PerturbingVertexInline(LastStepV0.Position);
	PerturbingVertexInline(LastStepV1.Position);

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

		PerturbingVertexInline(CurStepV0.Position);
		PerturbingVertexInline(CurStepV1.Position);

		FillQuad(LastStepV0, LastStepV1, CurStepV0, CurStepV1, OutTerrainMesh);

		LastStepV0 = CurStepV0;
		LastStepV1 = CurStepV1;
	}
}

void AHexTerrainGenerator::FillQuad(const FHexVertexData& FromV0, const FHexVertexData& FromV1, const FHexVertexData& ToV0, const FHexVertexData& ToV1, FCachedSectionData& OutTerrainMesh)
{
	if ((FromV0.Position - FromV1.Position).IsNearlyZero())
	{
		OutTerrainMesh.AddTriangle(ToV0, FromV0, ToV1);
	}
	else if ((ToV0.Position - ToV1.Position).IsNearlyZero())
	{
		OutTerrainMesh.AddTriangle(ToV0, FromV0, FromV1);
	}
	else if ((ToV0.Position - FromV0.Position).IsNearlyZero())
	{
		OutTerrainMesh.AddTriangle(ToV0, FromV1, ToV1);
	}
	else if ((ToV1.Position - FromV1.Position).IsNearlyZero())
	{
		OutTerrainMesh.AddTriangle(ToV0, FromV0, ToV1);
	}
	else
	{
		OutTerrainMesh.AddQuad(ToV0, ToV1, FromV0, FromV1);
	}
};

void AHexTerrainGenerator::FillFan(const FHexVertexData& CenterV, const TArray<FHexVertexData>& EdgesV,
	const TArray<bool>& bRecalcNormal, FCachedSectionData& OutTerrainMesh, bool bClosed)
{
	int32 BaseIndex = OutTerrainMesh.AddVertex(CenterV);

	bool bShouldRecalcNormal = bRecalcNormal.Num() > 0;
	TArray<int32> IndicesList;

	int32 NumOfEdges = EdgesV.Num();
	for (int32 EdgeIndex = 0; EdgeIndex < NumOfEdges; ++EdgeIndex)
	{
		if (bShouldRecalcNormal)
		{
			IndicesList.Add(bRecalcNormal[EdgeIndex] ? -EdgeIndex : EdgeIndex);
		}

		if (!bShouldRecalcNormal || !bRecalcNormal[EdgeIndex])
		{
			OutTerrainMesh.AddVertex(EdgesV[EdgeIndex]);
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

				FHexVertexData V0{ CenterV.Position, CenterV.VertexColor };
				FHexVertexData V1{ EdgesV[NextIndex].Position, EdgesV[NextIndex].VertexColor };
				FHexVertexData V2{ EdgesV[CurIndex].Position, EdgesV[CurIndex].VertexColor };

				OutTerrainMesh.AddTriangle(V0, V1, V2);
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

void AHexTerrainGenerator::PerturbingVertexInline(FVector& Vertex)
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
	
	int32 Elevation = FMath::RoundToInt(Vertex.Z / HexElevationStep);
	if (!CachedNoiseZ.Contains(Elevation))
	{
		FLinearColor NoiseVectorZ = SampleTextureBilinear(NoiseTexture, FMath::RoundToInt(Elevation * PerturbingScalingHV.Y), 0);
		CachedNoiseZ.Add(Elevation, NoiseVectorZ.B * 2.0f - 1.0f);
	}

	Vertex.Z += CachedNoiseZ[Elevation] * PerturbingStrengthHV.Y;
}

void AHexTerrainGenerator::PerturbingVerticesInline(TArray<FVector>& Vertices)
{
	int32 NumOfVerts = Vertices.Num();
	for (int32 Index = 0; Index < NumOfVerts; ++Index)
	{
		PerturbingVertexInline(Vertices[Index]);
	}
}

FVector AHexTerrainGenerator::PerturbingVertex(const FVector& Vertex)
{
	FVector NewVec = Vertex;
	PerturbingVertexInline(NewVec);
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

	if (!PlayerController)
		PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
		return;

	FHitResult HitResult;
	if (PlayerController->GetHitResultUnderCursor(ECC_Visibility, true, HitResult))
	{
		FIntPoint GridId = CalcHexCellGridId(HitResult.Location);
		
		if (HexEditMode == EHexEditMode::Cell)
		{
			HexEditGridId.X = GridId.X;
			HexEditGridId.Y = GridId.Y;

			HexEditElevation = ConfigData.ElevationsList[GridId.Y][GridId.X];
			HexEditTerrainType = ConfigData.TerrainTypesList[GridId.Y][GridId.X];
		}
		else if (HexEditMode == EHexEditMode::River)
		{
			int32 GridIndex = GridId.Y * HexChunkCount.X * HexChunkSize.X + GridId.X;
			const FHexCellData& CurGrid = HexGrids[GridIndex];
			bool bNeedUpdateConfig = true;
			if (CurGrid.HexRiver.RiverState == EHexRiverState::None)
			{
				if (HexEditRiverStartPoint.X < 0 || HexEditRiverStartPoint.Y < 0) // river's first node
				{
					HexEditRiverStartPoint = GridId;
					HexEditRiverLastPoint = GridId;
					HexEditRiverPoints.Add(GridId);
				}
				else // river's other nodes
				{
					int32 LastGridIndex = HexEditRiverLastPoint.Y * HexChunkCount.X * HexChunkSize.X + HexEditRiverLastPoint.X;
					const FHexCellData& LastGrid = HexGrids[LastGridIndex];

					bNeedUpdateConfig = false;
					for (int32 Index = 0; Index < CORNER_NUM; ++Index)
					{
						if (CurGrid.Elevation <= LastGrid.Elevation &&
							LastGrid.HexNeighbors[Index].LinkedCellIndex == GridIndex)
						{
							HexEditRiverFlowDirections.Add(static_cast<EHexDirection>(Index));
							HexEditRiverLastPoint = GridId;
							HexEditRiverPoints.Add(GridId);
							bNeedUpdateConfig = true;
							break;
						}
					}
				}
			}
			else
			{
				if (HexEditRiverPoints.Contains(GridId)) // remove river's nodes
				{
					int32 FoundIndex = -1;
					HexEditRiverPoints.Find(GridId, FoundIndex);

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
					const FHexRiverConfigData& RiverConfig = ConfigData.RiversList[RiverIndex];

					HexEditRiverId = RiverIndex;
					HexEditRiverStartPoint = RiverConfig.RiverStartPoint;
					HexEditRiverFlowDirections = RiverConfig.RiverFlowDirections;
					HexEditRiverPoints.Add(HexEditRiverStartPoint);

					int32 FirstIndex = HexEditRiverStartPoint.Y * HexChunkCount.X * HexChunkSize.X + HexEditRiverStartPoint.X;
					FHexCellData* LastRiverNode = &HexGrids[FirstIndex];
					for (EHexDirection FlowDir : HexEditRiverFlowDirections)
					{
						int32 CurGridIndex = LastRiverNode->HexNeighbors[static_cast<uint8>(FlowDir)].LinkedCellIndex;
						FHexCellData& CurRiverNode = HexGrids[CurGridIndex];
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

					FHexRiverConfigData& RiverConfig = ConfigData.RiversList[HexEditRiverId];
					RiverConfig.RiverStartPoint = HexEditRiverStartPoint;
					RiverConfig.RiverFlowDirections = HexEditRiverFlowDirections;

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
	}
	else
	{
		if (HexEditMode == EHexEditMode::Cell)
		{
			ClearEditParameters(EHexEditMode::Cell);
		}
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
		if (HexEditMode == EHexEditMode::Cell)
		{
			ClearEditParameters(EHexEditMode::River);
		}
		else if (HexEditMode == EHexEditMode::River)
		{
			ClearEditParameters(EHexEditMode::Cell);
		}
	}

	static FName Name_HexEditElevation = GET_MEMBER_NAME_CHECKED(AHexTerrainGenerator, HexEditElevation);
	static FName Name_HexEditTerrainType = GET_MEMBER_NAME_CHECKED(AHexTerrainGenerator, HexEditTerrainType);
	if (MemberPropertyName == Name_HexEditElevation || MemberPropertyName == Name_HexEditTerrainType)
	{
		if (HexEditGridId.X >= 0 && HexEditGridId.Y >= 0)
		{
			ConfigData.ElevationsList[HexEditGridId.Y][HexEditGridId.X] = HexEditElevation;
			ConfigData.TerrainTypesList[HexEditGridId.Y][HexEditGridId.X] = HexEditTerrainType;

			UpdateHexGridsData();
			GenerateTerrain();
		}
	}
}

#endif

void AHexTerrainGenerator::ClearEditParameters(EHexEditMode ModeToClear)
{
	if (ModeToClear == EHexEditMode::Cell)
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
}

#pragma optimize("", on)