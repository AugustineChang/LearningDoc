#include "HexTerrainGenerator.h"
#include "HexTerrainDataGenerator.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RotationMatrix.h"
#include "Materials/MaterialInstanceDynamic.h"

//#pragma optimize("", off)

//////////////////////////////////////////////////////////////////////////
int32 FHexCellFeature::MaxFeatureValue = 12;
int32 FHexCellFeature::MaxDetailFeatureValue = 10;
void FHexCellFeature::SetupFeature(int32 InFeatureWallValue)
{
	bHasWall = InFeatureWallValue < 0;
	int32 InFeatureValue = FMath::Abs(InFeatureWallValue);
	if (FeatureValue == InFeatureValue)
		return;
	
	FeatureValue = InFeatureValue;
	switch (FeatureValue)
	{
	case 0:
		FeatureTypes.Empty();
		ProbabilityValues.Empty();
		break;
	
	case 1:
		FeatureTypes = { EHexFeatureType::Tree };
		ProbabilityValues = { 0.6f };
		break;

	case 2:
		FeatureTypes = { EHexFeatureType::Tree, EHexFeatureType::Farm };
		ProbabilityValues = { 0.4f, 0.6f };
		break;

	case 3:
		FeatureTypes = { EHexFeatureType::Tree, EHexFeatureType::Farm, EHexFeatureType::Hovel };
		ProbabilityValues = { 0.3f, 0.5f, 0.6f };
		break;

	case 4:
		FeatureTypes = { EHexFeatureType::Tree, EHexFeatureType::Farm, EHexFeatureType::Hovel };
		ProbabilityValues = { 0.2f, 0.4f, 0.6f };
		break;

	case 5:
		FeatureTypes = { EHexFeatureType::Tree, EHexFeatureType::Hovel };
		ProbabilityValues = { 0.2f, 0.6f };
		break;

	case 6:
		FeatureTypes = { EHexFeatureType::Hovel, EHexFeatureType::LowRise };
		ProbabilityValues = { 0.4f , 0.6f };
		break;

	case 7:
		FeatureTypes = { EHexFeatureType::Hovel, EHexFeatureType::LowRise };
		ProbabilityValues = { 0.3f , 0.65f };
		break;

	case 8:
		FeatureTypes = { EHexFeatureType::Hovel, EHexFeatureType::LowRise, EHexFeatureType::HighRise };
		ProbabilityValues = { 0.2f , 0.5f, 0.7f };
		break;

	case 9:
		FeatureTypes = { EHexFeatureType::Hovel, EHexFeatureType::LowRise, EHexFeatureType::HighRise };
		ProbabilityValues = { 0.1f, 0.35f, 0.75f };
		break;

	case 10:
		FeatureTypes = { EHexFeatureType::LowRise, EHexFeatureType::HighRise, EHexFeatureType::Tower };
		ProbabilityValues = { 0.2f, 0.6f, 0.8f };
		break;

	case 11:
		FeatureTypes = { EHexFeatureType::Castle };
		ProbabilityValues = { 1.0f };
		break;

	case 12:
		FeatureTypes = { EHexFeatureType::Temple };
		ProbabilityValues = { 1.0f };
		break;

	default:
		break;
	}
}

FIntPoint FHexCellData::ChunkSize{ 0, 0 };
FIntPoint FHexCellData::ChunkCount{ 0, 0 };
uint8 FHexCellData::CellSubdivision = 0u;
int32 FHexCellData::MaxTerranceElevation = 0;
TArray<FVector> FHexCellData::HexVertices;
TArray<FVector> FHexCellData::HexSubVertices;

FColor FHexCellData::RoadColor = FColor{ 111u,61u,20u };

FHexCellData::FHexCellData(const FIntPoint& InIndex)
	: GridIndex(InIndex.X + InIndex.Y * ChunkSize.X * ChunkCount.X)
	, GridCoord(CalcGridCoordinate(InIndex))
	, CellCenter(EForceInit::ForceInitToZero)
	, TerrainType(EHexTerrainType::MAX), Elevation(0), WaterLevel(0)
	, HexRiver() , HexRoad(), HexFeature()
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

uint8 FHexCellData::CalcDirectionsDistance(EHexDirection InDirectionA, EHexDirection InDirectionB)
{
	uint8 DirA = static_cast<uint8>(InDirectionA);
	uint8 DirB = static_cast<uint8>(InDirectionB);

	uint8 TestDirA1 = DirA, TestDirA2 = DirA;
	for (uint8 Index = 0u; Index < CORNER_UNUM; ++Index)
	{
		if (TestDirA1 == DirB || TestDirA2 == DirB)
			return Index;

		TestDirA1 = CalcPreviousDirection(TestDirA1);
		TestDirA2 = CalcNextDirection(TestDirA2);
	}

	check(0);
	return 255u;
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

	int32 RowSize = ChunkSize.X * ChunkCount.X;
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
	:Position(InPos), bHasNormal(false), bHasUV0(false), bHasUV1(false), bHasUV2(false), bHasVertexColor(false), bPerturbed(false), VertexState(0u), VertexIndex(-1)
{}

FHexVertexData::FHexVertexData(const FVector& InPos, const FColor& InColor)
	:Position(InPos), VertexColor(InColor)
	, bHasNormal(false), bHasUV0(false), bHasUV1(false), bHasUV2(false), bHasVertexColor(true), bPerturbed(false), VertexState(0u), VertexIndex(-1)
{}

FHexVertexData::FHexVertexData(const FVector& InPos, const FVector2D& InUV0)
	:Position(InPos), UV0(InUV0)
	, bHasNormal(false), bHasUV0(true), bHasUV1(false), bHasUV2(false), bHasVertexColor(false), bPerturbed(false), VertexState(0u), VertexIndex(-1)
{}

FHexVertexData::FHexVertexData(const FVector& InPos, const FVector& InNormal)
	:Position(InPos), Normal(InNormal)
	, bHasNormal(true), bHasUV0(false), bHasUV1(false), bHasUV2(false), bHasVertexColor(false), bPerturbed(false), VertexState(0u), VertexIndex(-1)
{}

FHexVertexData::FHexVertexData(const FVector& InPos, const FColor& InColor, const FVector& InNormal)
	:Position(InPos), Normal(InNormal), VertexColor(InColor)
	, bHasNormal(true), bHasUV0(false), bHasUV1(false), bHasUV2(false), bHasVertexColor(true), bPerturbed(false), VertexState(0u), VertexIndex(-1)
{}

FHexVertexData::FHexVertexData(const FVector& InPos, const FColor& InColor, const FVector2D& InUV0)
	:Position(InPos), UV0(InUV0), VertexColor(InColor)
	, bHasNormal(false), bHasUV0(true), bHasUV1(false), bHasUV2(false), bHasVertexColor(true), bPerturbed(false), VertexState(0u), VertexIndex(-1)
{}

FHexVertexData::FHexVertexData(const FVector& InPos, const FVector& InNormal, const FVector2D& InUV0)
	:Position(InPos), Normal(InNormal), UV0(InUV0)
	, bHasNormal(true), bHasUV0(true), bHasUV1(false), bHasUV2(false), bHasVertexColor(false), bPerturbed(false), VertexState(0u), VertexIndex(-1)
{}

FHexVertexData::FHexVertexData(const FVector& InPos, const FColor& InColor, const FVector& InNormal, const FVector2D& InUV0)
	:Position(InPos), Normal(InNormal), UV0(InUV0), VertexColor(InColor)
	, bHasNormal(true), bHasUV0(true), bHasUV1(false), bHasUV2(false), bHasVertexColor(true), bPerturbed(false), VertexState(0u), VertexIndex(-1)
{}

FHexVertexData FHexVertexData::LerpVertex(const FHexVertexData& FromV, const FHexVertexData& ToV, FVector PosRatio, float AttrRatio)
{
	FVector NewVertex;
	NewVertex.X = FMath::Lerp(FromV.Position.X, ToV.Position.X, PosRatio.X);
	NewVertex.Y = FMath::Lerp(FromV.Position.Y, ToV.Position.Y, PosRatio.Y);
	NewVertex.Z = FMath::Lerp(FromV.Position.Z, ToV.Position.Z, PosRatio.Z);

	FHexVertexData OutVertex{ NewVertex };
	OutVertex.bPerturbed = FromV.bPerturbed & ToV.bPerturbed;

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

	bool bAllHasUV2 = FromV.bHasUV2 && ToV.bHasUV2;
	if (bAllHasUV2)
	{
		FVector2D NewUV2;
		NewUV2.X = FMath::Lerp(FromV.UV2.X, ToV.UV2.X, AttrRatio);
		NewUV2.Y = FMath::Lerp(FromV.UV2.Y, ToV.UV2.Y, AttrRatio);

		OutVertex.SetUV2(NewUV2);
	}

	return OutVertex;
}

FHexVertexData FHexVertexData::ApplyOverride(const FVector& InPosOffset, const FColor* InOverrideColor, const FVector2D* InOverrideUV0, bool bClear) const
{
	FHexVertexData CopiedVertex = *this;
	CopiedVertex.VertexIndex = -1;
	CopiedVertex.ApplyOverrideInline(InPosOffset, InOverrideColor, InOverrideUV0, bClear);
	return CopiedVertex;
}

void FHexVertexData::ApplyOverrideInline(const FVector& InPosOffset, const FColor* InOverrideColor, const FVector2D* InOverrideUV0, bool bClear)
{
	Position += InPosOffset;

	if (bClear) ClearProperties();

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
	TArray<FVector2D> UV2s;
	TArray<FColor> VertexColors;
	TArray<int32> Triangles;
	TArray<FProcMeshTangent> Tangents;
	
	//FBox BoundingBox;
	
public:
	void CreateMesh(TObjectPtr<UProceduralMeshComponent> TerrainMeshComponent, TObjectPtr<UMaterialInterface> SectionMaterial, bool bCreateCollision = false) const
	{
		TArray<FVector2D> EmptyUVs;
		int32 SectionId = TerrainMeshComponent->GetNumSections();
		TerrainMeshComponent->CreateMeshSection(SectionId, GetVertices(), GetTriangles(), GetNormals(), GetUV0s(), GetUV1s(), GetUV2s(), EmptyUVs, GetVertexColors(), GetTangents(), bCreateCollision);
		if (!!SectionMaterial)
		{
			TerrainMeshComponent->SetMaterial(SectionId, SectionMaterial);
		}
		TerrainMeshComponent->SetMeshSectionVisible(SectionId, !!SectionMaterial);
	}

	void MeshSection(const FCachedSectionData &Other)
	{
		if (Other.IsEmpty())
			return;

		int32 BaseIndex = Vertices.Num();
		Vertices.Append(Other.Vertices);
		Normals.Append(Other.Normals);
		UV0s.Append(Other.UV0s);
		UV1s.Append(Other.UV1s);
		UV2s.Append(Other.UV2s);
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

		if (V0.bHasUV2 && V1.bHasUV2 && V2.bHasUV2)
		{
			UV2s.Add(V0.UV2);
			UV2s.Add(V1.UV2);
			UV2s.Add(V2.UV2);
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

		if (V0.bHasUV2 && V1.bHasUV2 && V2.bHasUV2 && V3.bHasUV2)
		{
			UV2s.Add(V0.UV2);
			UV2s.Add(V1.UV2);
			UV2s.Add(V2.UV2);
			UV2s.Add(V3.UV2);
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
		UV2s.Empty(NumOfVerts + 1);
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

		if (InVertex.bHasUV2)
			UV2s.Add(InVertex.UV2);

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
	const TArray<FVector2D>& GetUV2s() const { return UV2s; }
	const TArray<FColor>& GetVertexColors() const { return VertexColors; }
	const TArray<int32>& GetTriangles() const { return Triangles; }
	const TArray<FProcMeshTangent>& GetTangents() const { return Tangents; }
	bool IsEmpty() const { return Vertices.IsEmpty(); }
};

struct FCachedFeatureData 
{
	EHexFeatureType FeatureType;
	FTransform FeatureTransform;

	FCachedFeatureData(EHexFeatureType InType, const FTransform& InTransform)
		: FeatureType(InType), FeatureTransform(InTransform)
	{}
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
	FCachedSectionData WallSection;
	TArray<FCachedFeatureData> Features;
};

struct FCachedTerrainData 
{
	TArray<FCachedChunkData> TerrainChunksSection;
};

int32 FHexCellConfigData::DefaultElevation = 0;
int32 FHexCellConfigData::DefaultWaterLevel = 0;
EHexTerrainType FHexCellConfigData::DefaultTerrainType = EHexTerrainType::Water;
int32 FHexCellConfigData::DefaultFeatureValue = 0;

//////////////////////////////////////////////////////////////////////////

// Sets default values
AHexTerrainGenerator::AHexTerrainGenerator()
	: bGenerateRandomly(true)
	, ConfigFileName(TEXT("HexTerrainConfig.json"))
	, HexChunkCount(0, 0)
	, HexChunkSize(0, 0)
	, HexCellRadius(100.0f)
	, HexCellBorderWidth(10.0f)
	, HexCellSubdivision(3u)
	, HexElevationStep(5.0f)
	, MaxElevationForTerrace(4)
	, RiverElevationOffset(-1)
	, RiverSubdivision(2u)
	, RoadElevationOffset(0.5f)
	, RoadWidthRatio(0.5f)
	, NoiseTexturePath(TEXT("Content/Noise.png"))
	, PerturbingStrengthHV(1.0f, 1.0f)
	, PerturbingScalingHV(0.25f, 1.0f)

	, HexEditMode(EHexEditMode::Ground)
	, HexEditGridId(-1, -1)
	, HexEditElevation(0)
	, HexEditWaterLevel(0)
	, HexEditTerrainType(EHexTerrainType::None)
	, HexEditFeatureValue(0)
	, HexEditRiverId(-1)
	, HexEditRiverStartPoint(-1, -1)
	, HexEditRiverLastPoint(-1, -1)
	, HexEditRoadFirstPoint(-1, -1)
{
 	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;

	TerrainMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMeshComponent"));
	TerrainMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TerrainMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	TerrainMeshComponent->Mobility = EComponentMobility::Movable;
	TerrainMeshComponent->SetGenerateOverlapEvents(false);
	TerrainMeshComponent->SetRenderCustomDepth(true);
	TerrainMeshComponent->SetupAttachment(RootComponent);
	TerrainMeshComponent->OnClicked.AddDynamic(this, &AHexTerrainGenerator::OnClicked);
	TerrainMeshComponent->OnReleased.AddDynamic(this, &AHexTerrainGenerator::OnReleased);

	TArray<UInstancedStaticMeshComponent*> FeatureComponentsList;
	for (int32 Index = 0; Index < 8; ++Index)
	{
		FString FeatureCompNameStr = FString::Printf(TEXT("Feature%dMeshComponent"), Index);
		UInstancedStaticMeshComponent* FeatureMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(FName{ *FeatureCompNameStr });
		FeatureMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FeatureMeshComponent->Mobility = EComponentMobility::Movable;
		FeatureMeshComponent->SetGenerateOverlapEvents(false);
		FeatureMeshComponent->SetupAttachment(RootComponent);
		FeatureComponentsList.Add(FeatureMeshComponent);
	}
	FeatureMeshComponents.Add(EHexFeatureType::Tree, FeatureComponentsList[0]);
	FeatureMeshComponents.Add(EHexFeatureType::Farm, FeatureComponentsList[1]);
	FeatureMeshComponents.Add(EHexFeatureType::Hovel, FeatureComponentsList[2]);
	FeatureMeshComponents.Add(EHexFeatureType::LowRise, FeatureComponentsList[2]);
	FeatureMeshComponents.Add(EHexFeatureType::HighRise, FeatureComponentsList[2]);
	FeatureMeshComponents.Add(EHexFeatureType::Tower, FeatureComponentsList[3]);
	FeatureMeshComponents.Add(EHexFeatureType::Castle, FeatureComponentsList[4]);
	FeatureMeshComponents.Add(EHexFeatureType::Temple, FeatureComponentsList[5]);
	FeatureMeshComponents.Add(EHexFeatureType::Bridge, FeatureComponentsList[6]);
	FeatureMeshComponents.Add(EHexFeatureType::WallTower, FeatureComponentsList[7]);

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

void AHexTerrainGenerator::GenerateOrLoadTerrain()
{
	if (bGenerateRandomly)
	{
		FHexTerrainDataGenerator DataGenerator{ ConfigData };
		DataGenerator.GenerateData();
		FIntPoint MapSize = DataGenerator.GetMapSize();
		ConfigFileName = FString::Format(TEXT("HexTerrainConfig{0}x{1}.json"), { MapSize.X, MapSize.Y });
	}
	else
		ConfigData.bConfigValid = LoadHexTerrainConfig();

	UpdateHexGridsData();
	CreateTerrain();
}

void AHexTerrainGenerator::SaveTerrain()
{
	SaveHexTerrainConfig();
}

void AHexTerrainGenerator::CreateTerrain()
{
	if (HexGrids.IsEmpty())
		return;

	int32 HexGridSizeX = HexChunkCount.X * HexChunkSize.X;
	int32 HexGridSizeY = HexChunkCount.Y * HexChunkSize.Y;

	// Create HexCellMesh
	FCachedTerrainData CachedTerrain;

	TObjectPtr<UMaterialInterface> GroundMaterial = MaterialsLibrary.FindOrAdd(TEXT("Ground"), nullptr);
	TObjectPtr<UMaterialInterface> RoadMaterial = MaterialsLibrary.FindOrAdd(TEXT("Road"), nullptr);
	TObjectPtr<UMaterialInterface> WaterMaterial = MaterialsLibrary.FindOrAdd(TEXT("Water"), nullptr);
	TObjectPtr<UMaterialInterface> EstuaryMaterial = MaterialsLibrary.FindOrAdd(TEXT("Estuary"), nullptr);
	TObjectPtr<UMaterialInterface> RiverMaterial = MaterialsLibrary.FindOrAdd(TEXT("River"), nullptr);
	TObjectPtr<UMaterialInterface> WallMaterial = MaterialsLibrary.FindOrAdd(TEXT("Feature"), nullptr);

	{
		static FName WaterWaveSourcePos{ TEXT("ScrPos") };
		FVector TerrainCenter = (HexGrids[0].CellCenter + HexGrids[HexGrids.Num() - 1].CellCenter) / 2.0;

		static FName WaterWaveFreqence{ TEXT("Freqence") };
		float WaveFrequence = 0.07f - FMath::Loge(float(HexGridSizeX)) * 0.0125f;
		WaveFrequence = FMath::Clamp(WaveFrequence, 0.015f, 0.06f);

		UMaterialInstanceDynamic* WaterDynMaterial = UMaterialInstanceDynamic::Create(WaterMaterial, this);
		WaterDynMaterial->SetVectorParameterValue(WaterWaveSourcePos, FVector4{ TerrainCenter.X, TerrainCenter.Y, 0.0, 0.0 });
		WaterDynMaterial->SetScalarParameterValue(WaterWaveFreqence, WaveFrequence);
		WaterMaterial = WaterDynMaterial;

		UMaterialInstanceDynamic* EstuaryDynMaterial = UMaterialInstanceDynamic::Create(EstuaryMaterial, this);
		EstuaryDynMaterial->SetVectorParameterValue(WaterWaveSourcePos, FVector4{ TerrainCenter.X, TerrainCenter.Y, 0.0, 0.0 });
		EstuaryDynMaterial->SetScalarParameterValue(WaterWaveFreqence, WaveFrequence);
		EstuaryMaterial = EstuaryDynMaterial;
	}

	TerrainMeshComponent->ClearAllMeshSections();
	for (int32 CY = 0; CY < HexChunkCount.Y; ++CY)
	{
		for (int32 CX = 0; CX < HexChunkCount.X; ++CX)
		{
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
			CurrentChunkSection.CollisionSection = CurrentChunkSection.GroundSection;
		}
	}

	int32 NumOfChunks = CachedTerrain.TerrainChunksSection.Num();
	for (int32 C = 0; C < NumOfChunks; ++C)
	{
		const FCachedChunkData& CurrentChunkSection = CachedTerrain.TerrainChunksSection[C];

		// Create Sections
		CurrentChunkSection.GroundSection.CreateMesh(TerrainMeshComponent, GroundMaterial);
		CurrentChunkSection.RoadSection.CreateMesh(TerrainMeshComponent, RoadMaterial);
		CurrentChunkSection.WaterSection.CreateMesh(TerrainMeshComponent, WaterMaterial);
		CurrentChunkSection.EstuarySection.CreateMesh(TerrainMeshComponent, EstuaryMaterial);
		CurrentChunkSection.RiverSection.CreateMesh(TerrainMeshComponent, RiverMaterial);
		CurrentChunkSection.CollisionSection.CreateMesh(TerrainMeshComponent, nullptr, true);

		// Create Features
		CurrentChunkSection.WallSection.CreateMesh(TerrainMeshComponent, WallMaterial);
	}

	// Features
	AddTerrainFeatures(CachedTerrain);

	// Grid Coordinates
	AddGridCoordinates(HexGridSizeX, HexGridSizeY);
}

/*void AHexTerrainGenerator::Debug()
{
	for (int32 Index = 0; Index < 10; ++Index)
	{
		UE_LOG(LogTemp, Display, TEXT("SRand = %0.4f"), FMath::SRand());
	}
}*/

void AHexTerrainGenerator::AddTerrainFeatures(const FCachedTerrainData& CachedTerrain)
{
	TObjectPtr<UMaterialInterface> FeatureMaterial = MaterialsLibrary.FindOrAdd(TEXT("Feature"), nullptr);
	for (uint8 Index = 1u; Index < uint8(EHexFeatureType::MAX); ++Index)
	{
		EHexFeatureType FeatureType = EHexFeatureType(Index);
		TObjectPtr<UStaticMesh> FeatureModel = ModelsLibrary.FindOrAdd(FHexCellFeature::GetHexFeatureString(FeatureType), nullptr);
		
		TObjectPtr<UInstancedStaticMeshComponent> FeatureMeshComponent = FeatureMeshComponents[FeatureType];
		if (!FeatureMeshComponent->GetStaticMesh() && !!FeatureModel)
		{
			FeatureMeshComponent->SetStaticMesh(FeatureModel.Get());
		}

		if (!!FeatureMaterial)
		{
			FeatureMeshComponent->SetMaterial(0, FeatureMaterial);
		}

		if (FeatureMeshComponent->GetInstanceCount() > 0)
			FeatureMeshComponent->ClearInstances();
	}
	
	int32 NumOfChunks = CachedTerrain.TerrainChunksSection.Num();
	for (int32 ChunkId = 0; ChunkId < NumOfChunks; ++ChunkId)
	{
		const FCachedChunkData& ChunkData = CachedTerrain.TerrainChunksSection[ChunkId];
		int32 NumOfFeaturesPerChunk = ChunkData.Features.Num();
		
		for (int32 FeatureId = 0; FeatureId < NumOfFeaturesPerChunk; ++FeatureId)
		{
			const FCachedFeatureData& OneFeature = ChunkData.Features[FeatureId];
			FeatureMeshComponents[OneFeature.FeatureType]->AddInstance(OneFeature.FeatureTransform, false);
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
			FIntPoint CellGridId{
				CellData.GridId.X * HexChunkSize.X + CellData.GridId.Z,
				CellData.GridId.Y * HexChunkSize.Y + CellData.GridId.W
			};

			CoordTextComponent->AddInstance(Instance, false);
			CoordTextComponent->SetCustomDataValue(GridIndex, 0, CellGridId.X);
			CoordTextComponent->SetCustomDataValue(GridIndex, 1, CellGridId.Y);
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
	ChunkSizeData[0]->TryGetNumber(ConfigData.HexChunkSize.X);
	ChunkSizeData[1]->TryGetNumber(ConfigData.HexChunkSize.Y);
	ChunkSizeData[2]->TryGetNumber(ConfigData.HexChunkCount.X);
	ChunkSizeData[3]->TryGetNumber(ConfigData.HexChunkCount.Y);

	TArray<TSharedPtr<FJsonValue>> ElevationsList = JsonRoot->GetArrayField(TEXT("Elevations"));
	TArray<TSharedPtr<FJsonValue>> WaterLevelsList = JsonRoot->GetArrayField(TEXT("WaterLevels"));
	TArray<TSharedPtr<FJsonValue>> TypesList = JsonRoot->GetArrayField(TEXT("HexTypes"));
	TArray<TSharedPtr<FJsonValue>> FeatureValuesList = JsonRoot->GetArrayField(TEXT("FeatureValues"));

	int32 HexGridSizeX = ConfigData.HexChunkCount.X * ConfigData.HexChunkSize.X;
	int32 HexGridSizeY = ConfigData.HexChunkCount.Y * ConfigData.HexChunkSize.Y;
	
	auto LoadGridProperty = [HexGridSizeX, HexGridSizeY]<typename T>(const TArray<TSharedPtr<FJsonValue>>& InConfigList, T DefaultVal, T MinVal, T MaxVal, TArray<TArray<T>>& OutDataList)
		{
			OutDataList.Empty(HexGridSizeY);
			OutDataList.AddDefaulted(HexGridSizeY);
			for (int32 Y = 0; Y < HexGridSizeY; ++Y)
			{
				OutDataList[Y].Init(DefaultVal, HexGridSizeX);
				if (Y >= InConfigList.Num())
					continue;

				const TArray<TSharedPtr<FJsonValue>>& OneRow = InConfigList[Y]->AsArray();
				for (int32 X = 0; X < HexGridSizeX; ++X)
				{
					int32 TempVal = static_cast<int32>(DefaultVal);
					if (X < OneRow.Num())
					{
						OneRow[X]->TryGetNumber(TempVal);
						TempVal = FMath::Clamp(TempVal, static_cast<int32>(MinVal), static_cast<int32>(MaxVal));
					}
					OutDataList[Y][X] = static_cast<T>(TempVal);
				}
			}
		};
	
	LoadGridProperty(ElevationsList, FHexCellConfigData::DefaultElevation, -100, 100, ConfigData.ElevationsList);
	LoadGridProperty(WaterLevelsList, FHexCellConfigData::DefaultWaterLevel, -100, 100, ConfigData.WaterLevelsList);
	LoadGridProperty(TypesList, FHexCellConfigData::DefaultTerrainType, EHexTerrainType::None, EHexTerrainType::MAX, ConfigData.TerrainTypesList);
	LoadGridProperty(FeatureValuesList, FHexCellConfigData::DefaultFeatureValue, -FHexCellFeature::MaxFeatureValue, FHexCellFeature::MaxFeatureValue, ConfigData.FeatureValuesList);
	
	auto LoadRiverRoadConfig = [](const TArray<TSharedPtr<FJsonValue>>& InConfigList, TArray<FHexRiverRoadConfigData>& OutDataList)
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

	TArray<TSharedPtr<FJsonValue>> RiversList = JsonRoot->GetArrayField(TEXT("Rivers"));
	TArray<TSharedPtr<FJsonValue>> RoadsList = JsonRoot->GetArrayField(TEXT("Roads"));
	LoadRiverRoadConfig(RiversList, ConfigData.RiversList);
	LoadRiverRoadConfig(RoadsList, ConfigData.RoadsList);

	return true;
}

void AHexTerrainGenerator::SaveHexTerrainConfig()
{
	FString StructuredJson;
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	int32 HexGridSizeX = ConfigData.HexChunkCount.X * ConfigData.HexChunkSize.X;
	int32 HexGridSizeY = ConfigData.HexChunkCount.Y * ConfigData.HexChunkSize.Y;

	TArray<TSharedPtr<FJsonValue>> ChunkSizeData;
	ChunkSizeData.Add(MakeShared<FJsonValueNumber>(ConfigData.HexChunkSize.X));
	ChunkSizeData.Add(MakeShared<FJsonValueNumber>(ConfigData.HexChunkSize.Y));
	ChunkSizeData.Add(MakeShared<FJsonValueNumber>(ConfigData.HexChunkCount.X));
	ChunkSizeData.Add(MakeShared<FJsonValueNumber>(ConfigData.HexChunkCount.Y));

	TArray<TSharedPtr<FJsonValue>> ElevationsList;
	TArray<TSharedPtr<FJsonValue>> WaterLevelsList;
	TArray<TSharedPtr<FJsonValue>> TypesList;
	TArray<TSharedPtr<FJsonValue>> FeatureValuesList;

	for (int32 Y = 0; Y < HexGridSizeY; ++Y)
	{
		TArray<TSharedPtr<FJsonValue>> ElevationRow;
		TArray<TSharedPtr<FJsonValue>> WaterLevelRow;
		TArray<TSharedPtr<FJsonValue>> TypeRow;
		TArray<TSharedPtr<FJsonValue>> FeatureValueRow;

		for (int32 X = 0; X < HexGridSizeX; ++X)
		{
			ElevationRow.Add(MakeShared<FJsonValueNumber>(ConfigData.ElevationsList[Y][X]));
			WaterLevelRow.Add(MakeShared<FJsonValueNumber>(ConfigData.WaterLevelsList[Y][X]));
			TypeRow.Add(MakeShared<FJsonValueNumber>(uint8(ConfigData.TerrainTypesList[Y][X])));
			FeatureValueRow.Add(MakeShared<FJsonValueNumber>(ConfigData.FeatureValuesList[Y][X]));
		}

		ElevationsList.Add(MakeShared<FJsonValueArray>(ElevationRow));
		WaterLevelsList.Add(MakeShared<FJsonValueArray>(WaterLevelRow));
		TypesList.Add(MakeShared<FJsonValueArray>(TypeRow));
		FeatureValuesList.Add(MakeShared<FJsonValueArray>(FeatureValueRow));
	}
	
	auto SaveRiverRoadConfig = [](const TArray<FHexRiverRoadConfigData>& InDataList, TArray<TSharedPtr<FJsonValue>>& OutConfigList)
		{
			int32 NumOfData = InDataList.Num();
			for (int32 Index = 0; Index < NumOfData; ++Index)
			{
				const FHexRiverRoadConfigData& ConfigData = InDataList[Index];
				TSharedRef<FJsonObject> ConfigJson = MakeShareable(new FJsonObject());

				TArray<TSharedPtr<FJsonValue>> StartPoint;
				StartPoint.Add(MakeShared<FJsonValueNumber>(ConfigData.StartPoint.X));
				StartPoint.Add(MakeShared<FJsonValueNumber>(ConfigData.StartPoint.Y));
				ConfigJson->SetArrayField(TEXT("StartPoint"), StartPoint);

				TArray<TSharedPtr<FJsonValue>> Directions;
				for (EHexDirection Direction : ConfigData.ExtensionDirections)
				{
					Directions.Add(MakeShared<FJsonValueNumber>(static_cast<uint8>(Direction)));
				}
				ConfigJson->SetArrayField(TEXT("Directions"), Directions);

				OutConfigList.Add(MakeShared<FJsonValueObject>(ConfigJson));
			}
		};
	
	TArray<TSharedPtr<FJsonValue>> RiversList;
	TArray<TSharedPtr<FJsonValue>> RoadsList;
	SaveRiverRoadConfig(ConfigData.RiversList, RiversList);
	SaveRiverRoadConfig(ConfigData.RoadsList, RoadsList);

	JsonObject->SetArrayField(TEXT("ChunkSize"), ChunkSizeData);
	JsonObject->SetArrayField(TEXT("Elevations"), ElevationsList);
	JsonObject->SetArrayField(TEXT("WaterLevels"), WaterLevelsList);
	JsonObject->SetArrayField(TEXT("HexTypes"), TypesList);
	JsonObject->SetArrayField(TEXT("FeatureValues"), FeatureValuesList);
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

	HexChunkSize = ConfigData.HexChunkSize;
	HexChunkCount = ConfigData.HexChunkCount;
	FHexCellData::ChunkSize = ConfigData.HexChunkSize;
	FHexCellData::ChunkCount = ConfigData.HexChunkCount;
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
	switch (InCellData.GetTerrainRiverState())
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

	// texture
	for (int32 Index = 0; Index < NumOfVerts; ++Index)
	{
		FHexVertexData& FromVert = FromVerts[Index];
		FHexVertexData& ToVert = ToVerts[Index];

		if (FromVert.VertexState == 1u)
			FromVert.SetTextureData(InCellData.GetTerrainTextureType(), OppositeCell.GetTerrainTextureType(), InCellData.WaterTextureType, 0.0f, 0.0f, 1.0f);
		else
			FromVert.SetTextureData(InCellData.GetTerrainTextureType(), OppositeCell.GetTerrainTextureType(), InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);

		if (ToVert.VertexState == 1u)
			ToVert.SetTextureData(InCellData.GetTerrainTextureType(), OppositeCell.GetTerrainTextureType(), OppositeCell.WaterTextureType, 0.0f, 0.0f, 1.0f);
		else
			ToVert.SetTextureData(InCellData.GetTerrainTextureType(), OppositeCell.GetTerrainTextureType(), OppositeCell.WaterTextureType, 0.0f, 1.0f, 0.0f);
	}
	
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

		float UVScale = CalcRiverUVScale(true, NumOfZSteps);
		bool bHasOutRiver = BorderDirection == InCellData.HexRiver.OutgoingDirection;
		FVector2D UV0s[4] = { FVector2D{ 0.0, 0.0 }, FVector2D{ 0.0, 1.0 }, FVector2D{ UVScale, 0.0 }, FVector2D{ UVScale, 1.0 } };

		FHexVertexData FromEdgeL = FromVerts[RiverIndex - 1].ApplyOverride(RiverZOffset, nullptr, &UV0s[bHasOutRiver ? 0 : 3]);
		FHexVertexData FromEdgeR = FromVerts[RiverIndex + 1].ApplyOverride(RiverZOffset, nullptr, &UV0s[bHasOutRiver ? 1 : 2]);
		FHexVertexData ToEdgeL = ToVerts[RiverIndex - 1].ApplyOverride(RiverZOffset, nullptr, &UV0s[bHasOutRiver ? 2 : 1]);
		FHexVertexData ToEdgeR = ToVerts[RiverIndex + 1].ApplyOverride(RiverZOffset, nullptr, &UV0s[bHasOutRiver ? 3 : 0]);

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
		const FColor& RoadColor = FHexCellData::RoadColor;

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

	// Wall
	bool bHasFromWall = InCellData.HexFeature.bHasWall;
	bool bHasToWall = OppositeCell.HexFeature.bHasWall;
	if (bHasFromWall != bHasToWall &&  
		FromWaterDepth <= 0 && ToWaterDepth <= 0 &&
		HexBorder.LinkState < EHexBorderState::Cliff)
	{
		int32 NumOfZStepsForWall = HexBorder.LinkState == EHexBorderState::Terrace ? NumOfZSteps : 0;
		if (RoadIndex < 0 && RiverIndex < 0)
		{
			TArray<FIntPoint> AtrributesList;
			AtrributesList.Init(FIntPoint{ NumOfZStepsForWall, -1 }, NumOfVerts);

			GenerateWallFeature(FromVerts, ToVerts, AtrributesList, bHasToWall, false, OutTerrainMesh);
		}
		else
		{
			TArray<FHexVertexData> FromVertsHalf1, FromVertsHalf2;
			TArray<FHexVertexData> ToVertsHalf1, ToVertsHalf2;
			
			int32 SkipIndex = RoadIndex < 0 ? RiverIndex : RoadIndex;
			for (int32 Index = 0; Index < SkipIndex; ++Index)
			{
				FromVertsHalf1.Add(FromVerts[Index]);
				ToVertsHalf1.Add(ToVerts[Index]);
			}
			for (int32 Index = SkipIndex + 1; Index < NumOfVerts; ++Index)
			{
				FromVertsHalf2.Add(FromVerts[Index]);
				ToVertsHalf2.Add(ToVerts[Index]);
			}

			TArray<FIntPoint> AttriListHalf1, AttriListHalf2;
			AttriListHalf1.Init(FIntPoint{ NumOfZStepsForWall, -1 }, SkipIndex);
			AttriListHalf2.Init(FIntPoint{ NumOfZStepsForWall, -1 }, NumOfVerts - SkipIndex - 1);

			GenerateWallFeature(FromVertsHalf1, ToVertsHalf1, AttriListHalf1, bHasToWall, false, OutTerrainMesh);
			GenerateWallFeature(FromVertsHalf2, ToVertsHalf2, AttriListHalf2, bHasToWall, false, OutTerrainMesh);

			TArray<FHexVertexData> FromVertsMid1 = { FromVertsHalf1.Last() }, FromVertsMid2 = { FromVertsHalf2[0] };
			TArray<FHexVertexData> ToVertsMid1 = { ToVertsHalf1.Last() }, ToVertsMid2 = { ToVertsHalf2[0] };

			TArray<FIntPoint> AttriListMid;
			AttriListMid.Init(FIntPoint{ NumOfZStepsForWall, -1 }, 1);

			GenerateWallFeature(FromVertsMid1, ToVertsMid1, AttriListMid, bHasToWall, false, OutTerrainMesh);
			GenerateWallFeature(ToVertsMid2, FromVertsMid2, AttriListMid, bHasToWall, false, OutTerrainMesh);
		}
	}
}

void AHexTerrainGenerator::GenerateHexCorner(const FHexCellData& InCellData, EHexDirection CornerDirection, FCachedChunkData& OutTerrainMesh)
{
	int32 CIndex = static_cast<uint8>(CornerDirection) - 4;
	const FHexCellCorner& CornerData = InCellData.HexCorners[CIndex];

	bool bHasTerraceCell12 = CornerData.LinkState[0] == EHexBorderState::Terrace;
	bool bHasTerraceCell13 = CornerData.LinkState[2] == EHexBorderState::Terrace;
	bool bHasTerraceCell23 = CornerData.LinkState[1] == EHexBorderState::Terrace;

	int32 NumOfTerraces = 0;	
	if (bHasTerraceCell12) ++NumOfTerraces;
	if (bHasTerraceCell23) ++NumOfTerraces;
	if (bHasTerraceCell13) ++NumOfTerraces;

	const FHexCellData& Cell1 = HexGrids[CornerData.LinkedCellsIndex.X];
	const FHexCellData& Cell2 = HexGrids[CornerData.LinkedCellsIndex.Y];
	const FHexCellData& Cell3 = HexGrids[CornerData.LinkedCellsIndex.Z];
	
	if (NumOfTerraces == 0)
		GenerateNoTerraceCorner(Cell1, Cell2, Cell3, CornerData, OutTerrainMesh);
	else
		GenerateCornerWithTerrace(Cell1, Cell2, Cell3, CornerData, OutTerrainMesh);
	
	// Wall
	bool bCell1InWall = Cell1.HexFeature.bHasWall;
	bool bCell2InWall = Cell2.HexFeature.bHasWall;
	bool bCell3InWall = Cell3.HexFeature.bHasWall;

	bool bCell1UnderWater = Cell1.GetWaterDepth() > 0;
	bool bCell2UnderWater = Cell2.GetWaterDepth() > 0;
	bool bCell3UnderWater = Cell3.GetWaterDepth() > 0;

	bool bHasWallCell12 = bCell1InWall != bCell2InWall && !bCell1UnderWater && !bCell2UnderWater;
	bool bHasWallCell13 = bCell1InWall != bCell3InWall && !bCell1UnderWater && !bCell3UnderWater;
	bool bHasWallCell23 = bCell2InWall != bCell3InWall && !bCell2UnderWater && !bCell3UnderWater;

	bool bHasCliffCell12 = CornerData.LinkState[0] >= EHexBorderState::Cliff;
	bool bHasCliffCell13 = CornerData.LinkState[2] >= EHexBorderState::Cliff;
	bool bHasCliffCell23 = CornerData.LinkState[1] >= EHexBorderState::Cliff;
	
	int32 HighestElevation = FMath::Max3(Cell1.Elevation, Cell2.Elevation, Cell3.Elevation);
	bool bHasWalledCliff12 = bHasWallCell12 && bHasCliffCell12 && (bCell1InWall == bCell3InWall ? Cell2.Elevation < HighestElevation : Cell1.Elevation < HighestElevation);
	bool bHasWalledCliff13 = bHasWallCell13 && bHasCliffCell13 && (bCell1InWall == bCell2InWall ? Cell3.Elevation < HighestElevation : Cell1.Elevation < HighestElevation);
	bool bHasWalledCliff23 = bHasWallCell23 && bHasCliffCell23 && (bCell1InWall == bCell2InWall ? Cell3.Elevation < HighestElevation : Cell2.Elevation < HighestElevation);

	bHasWallCell12 = bHasWallCell12 && CornerData.LinkState[0] < EHexBorderState::Cliff;
	bHasWallCell13 = bHasWallCell13 && CornerData.LinkState[2] < EHexBorderState::Cliff;
	bHasWallCell23 = bHasWallCell23 && CornerData.LinkState[1] < EHexBorderState::Cliff;

	int32 NumOfWalledBorders = 0;
	if (bHasWallCell12) ++NumOfWalledBorders;
	if (bHasWallCell13) ++NumOfWalledBorders;
	if (bHasWallCell23) ++NumOfWalledBorders;

	int32 NumOfWalledCliffs = 0;
	if (bHasWalledCliff12) ++NumOfWalledCliffs;
	if (bHasWalledCliff13) ++NumOfWalledCliffs;
	if (bHasWalledCliff23) ++NumOfWalledCliffs;

	TArray<FHexVertexData> OuterVerts;
	TArray<FHexVertexData> InnerVerts;
	TArray<FIntPoint> AttributesList;

	int32 ZDiff[3] = {
		FMath::Abs(Cell1.Elevation - Cell2.Elevation),
		FMath::Abs(Cell2.Elevation - Cell3.Elevation),
		FMath::Abs(Cell3.Elevation - Cell1.Elevation)
	};

	FHexVertexData V0 = CalcHexCellVertex(Cell1, CornerData.VertsId.X, false);
	FHexVertexData V1 = CalcHexCellVertex(Cell2, CornerData.VertsId.Y, false);
	FHexVertexData V2 = CalcHexCellVertex(Cell3, CornerData.VertsId.Z, false);

	if (NumOfWalledBorders == 2 || (NumOfWalledBorders == 1 && NumOfWalledCliffs == 1))
	{
		if ((bHasWallCell13 || bHasWalledCliff13) && (bHasWallCell23 || bHasWalledCliff23))
		{
			if (bCell1InWall)
			{
				InnerVerts = { V1, V0 };
				OuterVerts = { V2, V2 };
				AttributesList = {
					FIntPoint{bHasTerraceCell23 ? ZDiff[1] : 0, bHasCliffCell23 ? 1 : -1},
					FIntPoint{bHasTerraceCell13 ? ZDiff[2] : 0, bHasCliffCell13 ? 1 : -1}
				};
			}
			else
			{
				OuterVerts = { V0, V1 };
				InnerVerts = { V2, V2 };
				AttributesList = {
					FIntPoint{bHasTerraceCell13 ? ZDiff[2] : 0, bHasCliffCell13 ? 1 : -1},
					FIntPoint{bHasTerraceCell23 ? ZDiff[1] : 0, bHasCliffCell23 ? 1 : -1}
				};
			}
		}
		else if ((bHasWallCell12 || bHasWalledCliff12) && (bHasWallCell23 || bHasWalledCliff23))
		{
			if (bCell1InWall)
			{
				InnerVerts = { V0, V2 };
				OuterVerts = { V1, V1 };
				AttributesList = {
					FIntPoint{bHasTerraceCell12 ? ZDiff[0] : 0, bHasCliffCell12 ? 1 : -1},
					FIntPoint{bHasTerraceCell23 ? ZDiff[1] : 0, bHasCliffCell23 ? 1 : -1}
				};
			}
			else
			{
				OuterVerts = { V2, V0 };
				InnerVerts = { V1, V1 };
				AttributesList = {
					FIntPoint{bHasTerraceCell23 ? ZDiff[1] : 0, bHasCliffCell23 ? 1 : -1},
					FIntPoint{bHasTerraceCell12 ? ZDiff[0] : 0, bHasCliffCell12 ? 1 : -1}
				};
			}
		}
		else if ((bHasWallCell12 || bHasWalledCliff12) && (bHasWallCell13 || bHasWalledCliff13))
		{
			if (bCell2InWall)
			{
				InnerVerts = { V2, V1 };
				OuterVerts = { V0, V0 };
				AttributesList = {
					FIntPoint{bHasTerraceCell13 ? ZDiff[2] : 0, bHasCliffCell13 ? 1 : -1},
					FIntPoint{bHasTerraceCell12 ? ZDiff[0] : 0, bHasCliffCell12 ? 1 : -1}
				};
			}
			else
			{
				OuterVerts = { V1, V2 };
				InnerVerts = { V0, V0 };
				AttributesList = {
					FIntPoint{bHasTerraceCell12 ? ZDiff[0] : 0, bHasCliffCell12 ? 1 : -1},
					FIntPoint{bHasTerraceCell13 ? ZDiff[2] : 0, bHasCliffCell13 ? 1 : -1}
				};
			}
		}

		GenerateWallFeature(OuterVerts, InnerVerts, AttributesList, true, true, OutTerrainMesh);
	}
	else if (NumOfWalledBorders == 1 && NumOfWalledCliffs == 0)
	{
		if (bHasWallCell12)
		{
			InnerVerts = { V0 };
			OuterVerts = { V1 };
			AttributesList = { FIntPoint{bHasTerraceCell12 ? ZDiff[0] : 0, -1} };
		}
		else if (bHasWallCell13)
		{
			InnerVerts = { V2 };
			OuterVerts = { V0 };
			AttributesList = { FIntPoint{bHasTerraceCell13 ? ZDiff[2] : 0, -1} };
		}
		else //if (bHasWallCell23)
		{
			InnerVerts = { V1 };
			OuterVerts = { V2 };
			AttributesList = { FIntPoint{bHasTerraceCell23 ? ZDiff[1] : 0, -1} };
		}

		GenerateWallFeature(OuterVerts, InnerVerts, AttributesList, true, false, OutTerrainMesh);
	}
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

	FHexVertexData CenterV{ InCellData.CellCenter, FVector::UpVector };

	// texture
	int32 NumOfVerts = EdgesV.Num();
	for (int32 Index = 0; Index < NumOfVerts; ++Index)
	{
		FHexVertexData& EdgeVert = EdgesV[Index];

		EdgeVert.SetTextureData(InCellData.GetTerrainTextureType(), EHexTerrainTextureType::None, EHexTerrainTextureType::None, 1.0f, 0.0f, 0.0f);
	}
	CenterV.SetTextureData(InCellData.GetTerrainTextureType(), EHexTerrainTextureType::None, EHexTerrainTextureType::None, 1.0f, 0.0f, 0.0f);

	TArray<bool> Dummy;
	FillFan(CenterV, EdgesV, Dummy, OutTerrainMesh.GroundSection, true);

	// Road
	GenerateRoadCenter(CenterV, EdgesV, OutTerrainMesh);

	// Features
	for (int32 EdgeIndex = 0; EdgeIndex < CORNER_NUM; ++EdgeIndex)
	{
		AddDetailFeature(InCellData, InCellData.CellCenter, EdgeIndex, OutTerrainMesh.Features);
	}
	AddDetailFeature(InCellData, InCellData.CellCenter, -1, OutTerrainMesh.Features);
	AddLargeFeature(InCellData, InCellData.CellCenter, OutTerrainMesh.Features);
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
	FHexVertexData CenterV{ InCellData.CellCenter };

	// texture
	for (int32 Index = 0; Index < NumOfEdges; ++Index)
	{
		FHexVertexData& EdgeVert = EdgesV[Index];

		if (EdgeVert.VertexState == 1u)
			EdgeVert.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 0.0f, 0.0f, 1.0f);
		else
			EdgeVert.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
	}
	CenterV.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);

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
	if (RiverIndex >= 0)
	{
		FVector WaterZOffset = CalcWaterVertOffset();

		FHexVertexData CopiedEdgeL = EdgesV[RiverIndex - 1].ApplyOverride(WaterZOffset);
		FHexVertexData CopiedEdgeR = EdgesV[RiverIndex + 1].ApplyOverride(WaterZOffset);
		FHexVertexData CopiedCenterV{ InCellData.CellCenter };

		float UVScale = CalcRiverUVScale();

		EHexRiverState RiverState = InCellData.GetTerrainRiverState();
		if (RiverState == EHexRiverState::StartPoint)
		{
			CopiedCenterV.SetUV0(FVector2D{ 0.0, 0.5 });
			CopiedEdgeL.SetUV0(FVector2D{ UVScale, 0.0 });
			CopiedEdgeR.SetUV0(FVector2D{ UVScale, 1.0 });
		}
		else if (RiverState == EHexRiverState::EndPoint)
		{
			CopiedCenterV.SetUV0(FVector2D{ UVScale, 0.5 });
			CopiedEdgeL.SetUV0(FVector2D{ 0.0, 1.0 });
			CopiedEdgeR.SetUV0(FVector2D{ 0.0, 0.0 });
		}
		else
			check(0);

		FillQuad(CopiedCenterV, CopiedCenterV, CopiedEdgeL, CopiedEdgeR, OutTerrainMesh.RiverSection);
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
		AddDetailFeature(InCellData, InCellData.CellCenter, EdgeIndex, OutTerrainMesh.Features);
	}
}

void AHexTerrainGenerator::GenerateCenterWithRiverThrough(const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh)
{
	EHexDirection InDirection = InCellData.HexRiver.IncomingDirection;
	EHexDirection OutDirection = InCellData.HexRiver.OutgoingDirection;

	int32 InMVertId = FHexCellData::GetVertIdFromDirection(InDirection, true, 1u);
	int32 OutMVertId = FHexCellData::GetVertIdFromDirection(OutDirection, true, 1u);

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
	
	float MoveDist = HexCellRadius / float(HexCellSubdivision + 1);

	FHexVertexData CenterL = FHexVertexData{ InCellData.CellCenter + LeftDir * MoveDist };
	FHexVertexData Center = FHexVertexData{ InCellData.CellCenter + (LeftDir + RightDir) * 0.5 * MoveDist + CalcRiverVertOffset()};
	FHexVertexData CenterR = FHexVertexData{ InCellData.CellCenter + RightDir * MoveDist};
	
	// texture
	CenterL.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
	Center.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 0.0f, 0.0f, 1.0f);
	CenterR.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);

	auto GenerateFansWithoutRiver = [this](const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh,
		EHexDirection FromDirection, EHexDirection ToDirection, const FHexVertexData& InCenter) -> bool
		{
			TArray<FHexVertexData> EdgesV;

			EHexDirection CurDirection = FHexCellData::CalcNextDirection(FromDirection);
			while (CurDirection != ToDirection)
			{
				int32 EdgeIndex = FHexCellData::GetVertIdFromDirection(CurDirection, false);
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
				int32 EdgeIndex = FHexCellData::GetVertIdFromDirection(CurDirection, false);
				EdgesV.Add(CalcHexCellVertex(InCellData, EdgeIndex, false));
			}

			TArray<FHexVertexData> CentersV;
			CentersV.Init(InCenter, EdgesV.Num());

			// texture
			int32 NumOfVerts = EdgesV.Num();
			for (int32 Index = 0; Index < NumOfVerts; ++Index)
			{
				FHexVertexData& EdgeVert = EdgesV[Index];
				FHexVertexData& CenterVert = CentersV[Index];

				EdgeVert.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
				CenterVert.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
			}

			FillGrid(CentersV, EdgesV, OutTerrainMesh.GroundSection, RiverSubdivision, false, false);
			
			// Road
			bool bHasRoadVert = GenerateRoadCenter(InCenter, EdgesV, OutTerrainMesh);

			// Features
			CurDirection = FHexCellData::CalcNextDirection(FromDirection);
			while (CurDirection != ToDirection)
			{
				AddDetailFeature(InCellData, InCenter.Position, static_cast<uint8>(CurDirection), OutTerrainMesh.Features);

				CurDirection = FHexCellData::CalcNextDirection(CurDirection);
			}

			return bHasRoadVert;
		};

	bool bHasRoadVertL = GenerateFansWithoutRiver(InCellData, OutTerrainMesh, OutDirection, InDirection, CenterR);
	bool bHasRoadVertR = GenerateFansWithoutRiver(InCellData, OutTerrainMesh, InDirection, OutDirection, CenterL);

	bool bSharpLeftTurn = OutDirection == FHexCellData::CalcNextDirection(InDirection);
	bool bSharpRightTurn = OutDirection == FHexCellData::CalcPreviousDirection(InDirection);

	auto GenerateRiverFan = [this](const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh,
		const FHexVertexData& InCenterL, const FHexVertexData& InCenter, const FHexVertexData& InCenterR, EHexDirection Direction, 
		bool bHasRoadVertL, bool bHasRoadVertR) -> void
		{
			int32 SVertId = FHexCellData::GetVertIdFromDirection(Direction, true, 0u);
			int32 MVertId = FHexCellData::GetVertIdFromDirection(Direction, true, 1u);
			int32 EVertId = FHexCellData::GetVertIdFromDirection(Direction, true, 2u);

			// Left Fan
			TArray<FHexVertexData> EdgesLV;
			int32 MainVertL = FHexCellData::GetVertIdFromDirection(Direction, false);
			EdgesLV.Add(CalcHexCellVertex(InCellData, MainVertL, false));
			for (int32 Index = SVertId; Index <= MVertId - 1; ++Index)
			{
				EdgesLV.Add(CalcHexCellVertex(InCellData, Index, true));
			}
			
			TArray<FHexVertexData> CentersLV;
			CentersLV.Init(InCenterL, EdgesLV.Num());
			
			// texture
			int32 NumOfVerts = EdgesLV.Num();
			for (int32 Index = 0; Index < NumOfVerts; ++Index)
			{
				FHexVertexData& EdgeVert = EdgesLV[Index];
				FHexVertexData& CenterVert = CentersLV[Index];

				EdgeVert.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
				CenterVert.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
			}

			FillGrid(CentersLV, EdgesLV, OutTerrainMesh.GroundSection, RiverSubdivision);

			// Center Two Quads
			FHexVertexData EdgeL = CalcHexCellVertex(InCellData, MVertId - 1, true);
			FHexVertexData EdgeC = CalcHexCellVertex(InCellData, MVertId, true);
			FHexVertexData EdgeR = CalcHexCellVertex(InCellData, MVertId + 1, true);

			// texture
			EdgeL.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
			if (EdgeC.VertexState == 1u)
				EdgeC.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 0.0f, 0.0f, 1.0f);
			else
				EdgeC.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
			EdgeR.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);

			FillStrip(InCenterL, InCenter, EdgeL, EdgeC, OutTerrainMesh.GroundSection, RiverSubdivision);
			FillStrip(InCenter, InCenterR, EdgeC, EdgeR, OutTerrainMesh.GroundSection, RiverSubdivision);

			// Right Fan
			TArray<FHexVertexData> EdgesRV;
			for (int32 Index = MVertId + 1; Index <= EVertId; ++Index)
			{
				EdgesRV.Add(CalcHexCellVertex(InCellData, Index, true));
			}
			int32 MainVertR = FHexCellData::GetVertIdFromDirection(FHexCellData::CalcNextDirection(Direction), false);
			EdgesRV.Add(CalcHexCellVertex(InCellData, MainVertR, false));

			TArray<FHexVertexData> CentersRV;
			CentersRV.Init(InCenterR, EdgesRV.Num());

			// texture
			NumOfVerts = EdgesRV.Num();
			for (int32 Index = 0; Index < NumOfVerts; ++Index)
			{
				FHexVertexData& EdgeVert = EdgesRV[Index];
				FHexVertexData& CenterVert = CentersRV[Index];

				EdgeVert.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
				CenterVert.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
			}

			FillGrid(CentersRV, EdgesRV, OutTerrainMesh.GroundSection, RiverSubdivision);

			// River
			{
				FVector WaterZOffset = CalcWaterVertOffset();

				float UVScale = CalcRiverUVScale();
				bool bHasOutRiver = Direction == InCellData.HexRiver.OutgoingDirection;
				FVector2D UV0s[4] = { FVector2D{ 0.0, 0.0 }, FVector2D{ 0.0, 1.0 }, FVector2D{ UVScale, 0.0 }, FVector2D{ UVScale, 1.0 } };

				FHexVertexData CopiedCenterL = InCenterL.ApplyOverride(WaterZOffset, nullptr, &UV0s[bHasOutRiver ? 0 : 3]);
				FHexVertexData CopiedCenterR = InCenterR.ApplyOverride(WaterZOffset, nullptr, &UV0s[bHasOutRiver ? 1 : 2]);
				EdgeL.ApplyOverrideInline(WaterZOffset, nullptr, &UV0s[bHasOutRiver ? 2 : 1]);
				EdgeR.ApplyOverrideInline(WaterZOffset, nullptr, &UV0s[bHasOutRiver ? 3 : 0]);

				FillStrip(CopiedCenterL, CopiedCenterR, EdgeL, EdgeR, OutTerrainMesh.RiverSection, RiverSubdivision);
			}

			// Road
			if (bHasRoadVertL)
				GenerateRoadCenterWithRiver(InCellData, InCenterL, EdgesLV, OutTerrainMesh);
			if (bHasRoadVertR)
				GenerateRoadCenterWithRiver(InCellData, InCenterR, EdgesRV, OutTerrainMesh);
		};
	
	auto GenerateSharpRiverFan = [this](const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh,
		const FHexVertexData& InCenterL, const FHexVertexData& InCenter, const FHexVertexData& InCenterR, EHexDirection Direction, bool bMoveLeft) -> void
		{
			int32 SVertId = FHexCellData::GetVertIdFromDirection(Direction, true, 0u);
			int32 MVertId = FHexCellData::GetVertIdFromDirection(Direction, true, 1u);
			int32 EVertId = FHexCellData::GetVertIdFromDirection(Direction, true, 2u);

			// Left Fan
			TArray<FHexVertexData> EdgesLV;
			int32 MainVertL = FHexCellData::GetVertIdFromDirection(Direction, false);
			EdgesLV.Add(CalcHexCellVertex(InCellData, MainVertL, false));
			if (SVertId <= MVertId - 2)
			{
				for (int32 Index = SVertId; Index <= MVertId - 2; ++Index)
				{
					EdgesLV.Add(CalcHexCellVertex(InCellData, Index, true));
				}
			}

			TArray<FHexVertexData> CentersLV;
			CentersLV.Init(InCenterL, EdgesLV.Num());

			// texture
			int32 NumOfVerts = EdgesLV.Num();
			for (int32 Index = 0; Index < NumOfVerts; ++Index)
			{
				FHexVertexData& EdgeVert = EdgesLV[Index];
				FHexVertexData& CenterVert = CentersLV[Index];

				EdgeVert.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
				CenterVert.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
			}

			FillGrid(CentersLV, EdgesLV, OutTerrainMesh.GroundSection, RiverSubdivision);

			// Center Two Quads
			int32 MainVertIdL = FHexCellData::GetVertIdFromDirection(Direction, false);
			int32 MainVertIdR = FHexCellData::GetVertIdFromDirection(FHexCellData::CalcNextDirection(Direction), false);

			FHexVertexData EdgeL2 = MVertId - 2 < SVertId ? CalcHexCellVertex(InCellData, MainVertIdL, false) : CalcHexCellVertex(InCellData, MVertId - 2, true);
			FHexVertexData EdgeL1 = CalcHexCellVertex(InCellData, MVertId - 1, true);
			FHexVertexData EdgeC = CalcHexCellVertex(InCellData, MVertId, true);
			FHexVertexData EdgeR1 = CalcHexCellVertex(InCellData, MVertId + 1, true);
			FHexVertexData EdgeR2 = MVertId + 2 > EVertId ? CalcHexCellVertex(InCellData, MainVertIdR, false) : CalcHexCellVertex(InCellData, MVertId + 2, true);
			
			FVector RiverOffset = CalcRiverVertOffset();
			FVector EdgeCNoOffset = EdgeC.Position - RiverOffset;
			FVector EdgeL1WithOffset = EdgeL1.Position + RiverOffset;
			FVector EdgeR1WithOffset = EdgeR1.Position + RiverOffset;
			FHexVertexData MidL{ bMoveLeft ? (InCenterL.Position + EdgeL2.Position) * 0.5 : (InCenterL.Position + EdgeCNoOffset) * 0.5 };
			FHexVertexData MidC{ bMoveLeft ? (InCenter.Position + EdgeL1WithOffset) * 0.5 : (InCenter.Position + EdgeR1WithOffset) * 0.5 };
			FHexVertexData MidR{ bMoveLeft ? (InCenterR.Position + EdgeCNoOffset) * 0.5 : (InCenterR.Position + EdgeR2.Position) * 0.5 };
			FHexVertexData MidE{ bMoveLeft ? (InCenterR.Position + EdgeR2.Position) * 0.5 : (InCenterL.Position + EdgeL2.Position) * 0.5 };

			// texture
			EdgeL2.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
			EdgeL1.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
			if (EdgeC.VertexState == 1u)
				EdgeC.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 0.0f, 0.0f, 1.0f);
			else
				EdgeC.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
			EdgeR1.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
			EdgeR2.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);

			MidL.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
			MidC.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 0.0f, 0.0f, 1.0f);
			MidR.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
			MidE.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
			
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
			TArray<FHexVertexData> EdgesRV;
			if (MVertId + 2 <= EVertId)
			{
				for (int32 Index = MVertId + 2; Index <= EVertId; ++Index)
				{
					EdgesRV.Add(CalcHexCellVertex(InCellData, Index, true));
				}
			}
			
			int32 MainVertR = FHexCellData::GetVertIdFromDirection(FHexCellData::CalcNextDirection(Direction), false);
			EdgesRV.Add(CalcHexCellVertex(InCellData, MainVertR, false));

			TArray<FHexVertexData> CentersRV;
			CentersRV.Init(InCenterR, EdgesRV.Num());

			// texture
			NumOfVerts = EdgesRV.Num();
			for (int32 Index = 0; Index < NumOfVerts; ++Index)
			{
				FHexVertexData& EdgeVert = EdgesRV[Index];
				FHexVertexData& CenterVert = CentersRV[Index];

				EdgeVert.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
				CenterVert.SetTextureData(InCellData.TerrainTextureType, EHexTerrainTextureType::None, InCellData.WaterTextureType, 1.0f, 0.0f, 0.0f);
			}

			FillGrid(CentersRV, EdgesRV, OutTerrainMesh.GroundSection, RiverSubdivision);

			// River
			{
				FVector WaterZOffset = CalcWaterVertOffset();
				bool bHasOutRiver = Direction == InCellData.HexRiver.OutgoingDirection;
				float UVScale = CalcRiverUVScale();
				FVector2D UV0s[6] = { FVector2D{ 0.0, 0.0 }, FVector2D{ 0.0, 1.0 }, FVector2D{ 0.5 * UVScale, 0.0 }, FVector2D{ 0.5 * UVScale, 1.0 },FVector2D{ UVScale, 0.0 },FVector2D{ UVScale, 1.0 } };

				CenterL.ApplyOverrideInline(WaterZOffset, nullptr, &UV0s[bHasOutRiver ? 0 : 5]);
				CenterR.ApplyOverrideInline(WaterZOffset, nullptr, &UV0s[bHasOutRiver ? 1 : 4]);
				MidL.ApplyOverrideInline(WaterZOffset, nullptr, &UV0s[bHasOutRiver ? 2 : 3]);
				MidR.ApplyOverrideInline(WaterZOffset, nullptr, &UV0s[bHasOutRiver ? 3 : 2]);
				EdgeL1.ApplyOverrideInline(WaterZOffset, nullptr, &UV0s[bHasOutRiver ? 4 : 1]);
				EdgeR1.ApplyOverrideInline(WaterZOffset, nullptr, &UV0s[bHasOutRiver ? 5 : 0]);

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

		// Road Bridge
		if (bHasRoadVertL || bHasRoadVertR)
		{
			AddRoadBridgeFeature(InCellData, CenterL, CenterR, OutDir, OutTerrainMesh.Features);
		}
	}
}

void AHexTerrainGenerator::GenerateNoTerraceCorner(const FHexCellData& Cell1, const FHexCellData& Cell2, const FHexCellData& Cell3, const FHexCellCorner& CornerData, FCachedChunkData& OutTerrainMesh)
{
	FHexVertexData V0 = CalcHexCellVertex(Cell1, CornerData.VertsId.X, false);
	FHexVertexData V1 = CalcHexCellVertex(Cell2, CornerData.VertsId.Y, false);
	FHexVertexData V2 = CalcHexCellVertex(Cell3, CornerData.VertsId.Z, false);

	// texture
	V0.SetTextureData(Cell1.GetTerrainTextureType(), Cell2.GetTerrainTextureType(), Cell3.GetTerrainTextureType(), 1.0f, 0.0f, 0.0f);
	V1.SetTextureData(Cell1.GetTerrainTextureType(), Cell2.GetTerrainTextureType(), Cell3.GetTerrainTextureType(), 0.0f, 1.0f, 0.0f);
	V2.SetTextureData(Cell1.GetTerrainTextureType(), Cell2.GetTerrainTextureType(), Cell3.GetTerrainTextureType(), 0.0f, 0.0f, 1.0f);

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

	// texture
	Vert0.SetTextureData(CellsList[0]->GetTerrainTextureType(), CellsList[1]->GetTerrainTextureType(), CellsList[2]->GetTerrainTextureType(), 1.0f, 0.0f, 0.0f);
	Vert1.SetTextureData(CellsList[0]->GetTerrainTextureType(), CellsList[1]->GetTerrainTextureType(), CellsList[2]->GetTerrainTextureType(), 0.0f, 1.0f, 0.0f);
	Vert2.SetTextureData(CellsList[0]->GetTerrainTextureType(), CellsList[1]->GetTerrainTextureType(), CellsList[2]->GetTerrainTextureType(), 0.0f, 0.0f, 1.0f);

	FHexVertexData DisturbedVert0 = PerturbingVertex(Vert0);
	FHexVertexData DisturbedVert1 = PerturbingVertex(Vert1);
	FHexVertexData DisturbedVert2 = PerturbingVertex(Vert2);

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
					//CurStepVert.bPerturbed = true;
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
		FillQuad(Verts02[Index - 1].Last(), Verts01[Index - 1].Last(), Verts02[Index][0], Verts01[Index][0], OutTerrainMesh.GroundSection);

		// Current Elevation
		if (Verts01[Index].Num() > 1 || Verts02[Index].Num() > 1)
			FillQuad(Verts02[Index][0], Verts01[Index][0], Verts02[Index].Last(), Verts01[Index].Last(), OutTerrainMesh.GroundSection);
	}
}

bool AHexTerrainGenerator::GenerateRoadCenter(const FHexVertexData& CenterV, const TArray<FHexVertexData>& EdgesV, FCachedChunkData& OutTerrainMesh)
{
	FVector RoadZOffset = CalcRoadVertOffset();
	const FColor& RoadColor = FHexCellData::RoadColor;
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
	const FColor& RoadColor = FHexCellData::RoadColor;
	FVector2D UV0s[2] = { FVector2D{ 0.0, 0.0 }, FVector2D{ 1.0, 0.5 } };

	TArray<int32> RoadVertIndices;
	for (int32 VertIndex = 0; VertIndex < CORNER_NUM; ++VertIndex)
	{
		int32 EdgeIndex = FHexCellData::CalcNextDirection(VertIndex);
		if (InCellData.HexRoad.RoadState[EdgeIndex])
		{
			int32 SubVertIndex = FHexCellData::GetVertIdFromDirection(static_cast<EHexDirection>(EdgeIndex));
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

void AHexTerrainGenerator::AddRoadBridgeFeature(const FHexCellData& InCellData, const FHexVertexData& CenterLV, const FHexVertexData& CenterRV, const FVector& OffsetDir, TArray<FCachedFeatureData>& OutFeatures)
{
	EHexDirection RiverInDirection = InCellData.HexRiver.IncomingDirection;
	EHexDirection RiverOutDirection = InCellData.HexRiver.OutgoingDirection;
	EHexDirection RiverOppoOutDirection = FHexCellData::CalcOppositeDirection(RiverOutDirection);

	FVector DirL2R = CenterRV.Position - CenterLV.Position;
	double LenL2R = DirL2R.Length();

	FVector BridgeOffset = FVector::ZeroVector;
	if (RiverOppoOutDirection == RiverInDirection)
	{
		uint8 TotalDistToRiverOut = 0u, TotalDistToRiverIn = 0u;
		for (uint8 EdgeIndex = 0; EdgeIndex < CORNER_UNUM; ++EdgeIndex)
		{
			if (InCellData.HexRoad.RoadState[EdgeIndex])
			{
				EHexDirection RoadDirection = static_cast<EHexDirection>(EdgeIndex);
				TotalDistToRiverOut += FHexCellData::CalcDirectionsDistance(RoadDirection, RiverOutDirection);
				TotalDistToRiverIn += FHexCellData::CalcDirectionsDistance(RoadDirection, RiverInDirection);
			}
		}
		float BridgeOffsetDist = HexCellRadius / float(HexCellSubdivision + 1) * 0.15f * FMath::Sign(TotalDistToRiverIn - TotalDistToRiverOut);
		BridgeOffset = OffsetDir.GetSafeNormal() * BridgeOffsetDist;
	}
	else
	{
		bool bRiverRightTurn = RiverOppoOutDirection == FHexCellData::CalcNextDirection(RiverInDirection);
		bool bRiverLeftTurn = RiverOppoOutDirection == FHexCellData::CalcPreviousDirection(RiverInDirection);

		float BridgeOffsetDist = HexCellRadius / float(HexCellSubdivision + 1) * 0.075f * (bRiverRightTurn ? 1.0f : (bRiverLeftTurn ? -1.0f : 0.0f));
		BridgeOffset = (DirL2R / LenL2R) * BridgeOffsetDist;
		LenL2R *= 1.06;
	}
	
	FVector Center = (CenterRV.Position + CenterLV.Position) * 0.5 + CalcRoadVertOffset() * 2.0 + BridgeOffset;
	FQuat BridgeRot = FRotationMatrix::MakeFromX(DirL2R).ToQuat();
	FTransform FeatureTransform{ BridgeRot, Center, FVector::OneVector * LenL2R * 0.01 };
	OutFeatures.Add(FCachedFeatureData{ EHexFeatureType::Bridge, FeatureTransform });
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

	FHexVertexData CenterV{ InCellData.CellCenter + WaterZOffset, FVector::UpVector, WaterUV0 };
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

FHexVertexData AHexTerrainGenerator::CalcHexCellVertex(const FHexCellData& InCellData, int32 VertIndex, bool bSubVert, uint8 VertType, bool bFillDefaultNormal) const
{
	FHexVertexData OutVertex{ InCellData.CellCenter };

	TArray<int32> BorderDirections;

	if (bSubVert)
	{
		int32 VertDirectionId = VertIndex / int32(HexCellSubdivision);
		OutVertex.VertexIndex = VertIndex + VertDirectionId + 1;
		int32 BorderDirectionId = FHexCellData::CalcNextDirection(uint8(VertDirectionId));

		if (VertType == 0u) // Ground
		{
			TArray<int32> RiverVerts;
			switch (InCellData.GetTerrainRiverState())
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
				OutVertex.Position += CalcRiverVertOffset();
			}
			else if (RoadVerts.Contains(VertIndex))
			{
				OutVertex.VertexState = 2u;
			}
		}
		else if (VertType == 1u) // Water
		{
			BorderDirections.Add(BorderDirectionId);
		}
	}
	else
	{
		OutVertex.VertexIndex = VertIndex * (HexCellSubdivision + 1);

		if (VertType == 1u) // Water
		{
			BorderDirections.Add(VertIndex);
			BorderDirections.Add(FHexCellData::CalcNextDirection(uint8(VertIndex)));
		}
	}

	double WaterVertRadiusScale = 1.0;
	if (VertType == 1u) // Water
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
		OutVertex.ApplyOverrideInline(CalcWaterVertOffset(bShore ? (NeighborWaterLevel - InCellData.Elevation) : WaterDepth), nullptr, &WaterUV0);
		
		WaterVertRadiusScale = (bFullNeighbor && WaterDepth > 0) ? 0.8 : 1.0;
	}

	if (bSubVert)
		OutVertex.Position += FHexCellData::HexSubVertices[VertIndex] * WaterVertRadiusScale;
	else
		OutVertex.Position += FHexCellData::HexVertices[VertIndex] * WaterVertRadiusScale;

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

FVector AHexTerrainGenerator::CalcFaceNormal(const FVector& V0, const FVector& V1, const FVector& V2)
{
	FVector Edge1 = (V1 - V0);
	FVector Edge2 = (V2 - V0);
	FVector NormalVector = FVector::CrossProduct(Edge1, Edge2);
	return NormalVector.GetSafeNormal();
}

void AHexTerrainGenerator::AddDetailFeature(const FHexCellData& InCellData, const FVector& InCenter, int32 LocDirectionId, TArray<FCachedFeatureData>& OutFeatures)
{
	const FHexCellFeature& FeatureData = InCellData.HexFeature;
	int32 NumOfFeatures = FeatureData.FeatureTypes.Num();
	if (InCellData.GetWaterDepth() > 0 || NumOfFeatures <= 0 ||
		FeatureData.FeatureValue <= 0 || FeatureData.FeatureValue > FHexCellFeature::MaxDetailFeatureValue)
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
	
	FVector4 FeatureRandom = GetRandomValueByPosition(FeatureLocation);
	EHexFeatureType FeatureType = EHexFeatureType::None;
	for (int32 Index = 0; Index < NumOfFeatures; ++Index)
	{
		if (FeatureRandom.W <= FeatureData.ProbabilityValues[Index])
		{
			FeatureType = FeatureData.FeatureTypes[Index];
			break;
		}
	}
	if (FeatureType == EHexFeatureType::None)
		return;

	PerturbingVertexInline(FeatureLocation, PerturbingStrengthHV, true);
	
	FQuat RandRot{ FVector::UpVector, FeatureRandom.Z * UE_TWO_PI };
	FVector2D RandXY = FVector2D{ FeatureRandom } *2.0 - 1.0;

	switch (FeatureType)
	{
	case EHexFeatureType::Tree:
	{
		double WidthXY = 0.4 + RandXY.X * 0.1;
		double Height = 0.4 + RandXY.Y * 0.1;
		FTransform FeatureTransform{ RandRot, FeatureLocation, FVector{WidthXY, WidthXY, Height} };
		OutFeatures.Add(FCachedFeatureData{ EHexFeatureType::Tree, FeatureTransform });
		break;
	}

	case EHexFeatureType::Farm:
	{
		double WidthX = 0.25 + RandXY.X * 0.1;
		double WidthY = WidthX;
		FTransform FeatureTransform{ FQuat::Identity, FeatureLocation, FVector{WidthX, WidthY, 0.1} };
		OutFeatures.Add(FCachedFeatureData{ EHexFeatureType::Farm, FeatureTransform });
		break;
	}


	case EHexFeatureType::Hovel:
	{
		double WidthX = 0.4 + RandXY.X * 0.2;
		double WidthY = 0.4 - RandXY.X * 0.2;
		double Height = 0.1 + RandXY.Y * 0.02;
		FTransform FeatureTransform{ RandRot, FeatureLocation, FVector{WidthX, WidthY, Height} };
		OutFeatures.Add(FCachedFeatureData{ EHexFeatureType::Hovel, FeatureTransform });
		break;
	}
		
	case EHexFeatureType::LowRise:
	{
		double WidthX = 0.4 + RandXY.X * 0.1;
		double WidthY = 0.4 - RandXY.X * 0.1;
		double Height = (0.1 + RandXY.Y * 0.02) * 3.0;
		FTransform FeatureTransform{ RandRot, FeatureLocation, FVector{WidthX, WidthY, Height} };
		OutFeatures.Add(FCachedFeatureData{ EHexFeatureType::LowRise, FeatureTransform });
		break;
	}
	
	case EHexFeatureType::HighRise:
	{
		double WidthX = 0.4 + RandXY.X * 0.05;
		double WidthY = 0.4 - RandXY.X * 0.05;
		double Height = (0.1 + RandXY.Y * 0.02) * 5.0;
		FTransform FeatureTransform{ RandRot, FeatureLocation, FVector{WidthX, WidthY, Height} };
		OutFeatures.Add(FCachedFeatureData{ EHexFeatureType::HighRise, FeatureTransform });
		break;
	}

	case EHexFeatureType::Tower:
	{
		double WidthX = 0.4 + RandXY.X * 0.05;
		double WidthY = 0.4 - RandXY.X * 0.05;
		double Height = (0.1 + RandXY.Y * 0.02) * 7.0;
		FTransform FeatureTransform{ RandRot, FeatureLocation, FVector{WidthX, WidthY, Height} };
		OutFeatures.Add(FCachedFeatureData{ EHexFeatureType::Tower, FeatureTransform });
		break;
	}

	default:
		break;
	}
}

void AHexTerrainGenerator::AddLargeFeature(const FHexCellData& InCellData, const FVector& InCenter, TArray<FCachedFeatureData>& OutFeatures)
{
	const FHexCellFeature& FeatureData = InCellData.HexFeature;
	int32 NumOfFeatures = FeatureData.FeatureTypes.Num();
	if (InCellData.GetWaterDepth() > 0 || NumOfFeatures <= 0 ||
		FeatureData.FeatureValue <= FHexCellFeature::MaxDetailFeatureValue)
		return;

	const FHexCellRiver& RiverData = InCellData.HexRiver;
	const FHexCellRoad& RoadData = InCellData.HexRoad;
	if (RiverData.RiverState != EHexRiverState::None || RoadData.GetPackedState() > 0u)
		return;

	FVector FeatureLocation = InCellData.CellCenter;

	FVector4 FeatureRandom = GetRandomValueByPosition(FeatureLocation);
	EHexFeatureType FeatureType = EHexFeatureType::None;
	for (int32 Index = 0; Index < NumOfFeatures; ++Index)
	{
		if (FeatureRandom.W <= FeatureData.ProbabilityValues[Index])
		{
			FeatureType = FeatureData.FeatureTypes[Index];
			break;
		}
	}
	if (FeatureType == EHexFeatureType::None)
		return;

	PerturbingVertexInline(FeatureLocation, PerturbingStrengthHV, true);

	FQuat RandRot{ FVector::UpVector, FeatureRandom.Z * UE_TWO_PI };
	FVector2D RandXY = FVector2D{ FeatureRandom } * 2.0 - 1.0;

	switch (FeatureType)
	{
	case EHexFeatureType::Castle:
	{
		double WidthX = 0.6 + RandXY.X * 0.15;
		double WidthY = 0.6 - RandXY.X * 0.15;
		double Height = 0.6 + RandXY.Y * 0.2;
		FTransform FeatureTransform{ RandRot, FeatureLocation, FVector{WidthX, WidthY, Height} };
		OutFeatures.Add(FCachedFeatureData{ EHexFeatureType::Castle, FeatureTransform });
		break;
	}

	case EHexFeatureType::Temple:
	{
		double WidthX = 0.6 + RandXY.X * 0.15;
		double WidthY = 0.6 - RandXY.X * 0.15;
		double Height = 0.6 + RandXY.Y * 0.2;
		FTransform FeatureTransform{ RandRot, FeatureLocation, FVector{WidthX, WidthY, Height} };
		OutFeatures.Add(FCachedFeatureData{ EHexFeatureType::Temple, FeatureTransform });
		break;
	}

	default:
		break;
	};
}

void AHexTerrainGenerator::GenerateWallFeature(const TArray<FHexVertexData>& FromVerts, const TArray<FHexVertexData>& ToVerts, const TArray<FIntPoint>& AttributesList, bool bToVertsInWall, bool bAddTower, FCachedChunkData& OutTerrainMesh)
{
	int32 NumOfVerts = FromVerts.Num();
	if (NumOfVerts <= 0)
		return;

	TArray<FHexVertexData> OutWallVertsDown, OutWallVertsUp;
	TArray<FHexVertexData> InWallVertsDown, InWallVertsUp;

	auto CalcTerraceRatio = [](double InRatio, int32 NumOfZSteps) -> double
		{
			double TotalSteps = double(NumOfZSteps * 2 - 1);
			InRatio *= TotalSteps;
			double FloorRatio = FMath::Floor(InRatio);
			double Alpha = FMath::Fmod(FloorRatio, 2.0);
			double FlatPart = FMath::Floor(FloorRatio / 2 + 0.5);
			double SlopePart = FlatPart + InRatio - FMath::Floor(InRatio);
			return FMath::Lerp(SlopePart, FlatPart, Alpha) / double(NumOfZSteps);
		};

	static FVector2D RatioXY{ 0.4, 0.6 };
	static FColor WallColor = FColor::Red;
	
	TArray<FVector2D> RatioZs;
	RatioZs.Init(RatioXY, NumOfVerts);

	int32 CliffEdgeIndex = -1;
	for (int32 Index = 0; Index < NumOfVerts; ++Index)
	{
		const FIntPoint& Attribute = AttributesList[Index];
		int32 NumOfZSteps = Attribute.X;
		bool bToCliff = Attribute.Y > 0;

		const FHexVertexData& FromVert = FromVerts[Index];
		const FHexVertexData& ToVert = ToVerts[Index];
		
		FHexVertexData DownVert0{ FVector::ZeroVector };
		FHexVertexData DownVert1{ FVector::ZeroVector };

		if (bToCliff)
		{
			DownVert0 = FromVert.Position.Z > ToVert.Position.Z ? FromVert : ToVert;
			DownVert1 = DownVert0;
			CliffEdgeIndex = Index;
		}
		else
		{
			FVector2D RatioZ = RatioXY;
			if (NumOfZSteps > 0)
			{
				RatioZ.X = CalcTerraceRatio(RatioZ.X, NumOfZSteps);
				RatioZ.Y = CalcTerraceRatio(RatioZ.Y, NumOfZSteps);
			}
			RatioZs[Index] = RatioZ;

			DownVert0 = FHexVertexData::LerpVertex(FromVert, ToVert, FVector{ RatioXY.X, RatioXY.X, RatioZ.X }, RatioXY.X);
			DownVert1 = FHexVertexData::LerpVertex(FromVert, ToVert, FVector{ RatioXY.Y, RatioXY.Y, RatioZ.Y }, RatioXY.Y);

			DownVert0.Position.Z = FMath::Min(DownVert0.Position.Z, DownVert1.Position.Z);
			DownVert1.Position.Z = DownVert0.Position.Z;
		}
		DownVert0.ClearProperties();
		DownVert1.ClearProperties();
		DownVert0.SetVertexColor(WallColor);
		DownVert1.SetVertexColor(WallColor);

		if (bToVertsInWall)
		{
			OutWallVertsDown.Add(DownVert0);
			OutWallVertsUp.Add(DownVert0.ApplyOverride(CalcWallVertOffset(), nullptr, nullptr, false));
			InWallVertsDown.Add(DownVert1);
			InWallVertsUp.Add(DownVert1.ApplyOverride(CalcWallVertOffset(), nullptr, nullptr, false));
		}
		else
		{
			OutWallVertsDown.Add(DownVert1);
			OutWallVertsUp.Add(DownVert1.ApplyOverride(CalcWallVertOffset(), nullptr, nullptr, false));
			InWallVertsDown.Add(DownVert0);
			InWallVertsUp.Add(DownVert0.ApplyOverride(CalcWallVertOffset(), nullptr, nullptr, false));
		}
	}

	if (OutWallVertsDown.Num() <= 1)
	{
		if (bToVertsInWall)
			FillQuad(OutWallVertsDown[0], InWallVertsDown[0], OutWallVertsUp[0], InWallVertsUp[0], OutTerrainMesh.WallSection);
		else
			FillQuad(InWallVertsDown[0], OutWallVertsDown[0], InWallVertsUp[0], OutWallVertsUp[0], OutTerrainMesh.WallSection);
	}
	else
	{
		if (CliffEdgeIndex >= 0)
		{
			int32 OtherEdgeIndex = (CliffEdgeIndex + 1) % 2;
			OutWallVertsDown[CliffEdgeIndex].Position.Z = OutWallVertsDown[OtherEdgeIndex].Position.Z;
			OutWallVertsUp[CliffEdgeIndex].Position.Z = OutWallVertsUp[OtherEdgeIndex].Position.Z;
			InWallVertsDown[CliffEdgeIndex].Position.Z = InWallVertsDown[OtherEdgeIndex].Position.Z;
			InWallVertsUp[CliffEdgeIndex].Position.Z = InWallVertsUp[OtherEdgeIndex].Position.Z;
		}

		if (bToVertsInWall)
		{
			FillGrid(OutWallVertsDown, OutWallVertsUp, OutTerrainMesh.WallSection, 1);
			FillGrid(InWallVertsUp, InWallVertsDown, OutTerrainMesh.WallSection, 1);
			FillGrid(OutWallVertsUp, InWallVertsUp, OutTerrainMesh.WallSection, 1);
		}
		else
		{
			FillGrid(OutWallVertsUp, OutWallVertsDown, OutTerrainMesh.WallSection, 1);
			FillGrid(InWallVertsDown, InWallVertsUp, OutTerrainMesh.WallSection, 1);
			FillGrid(InWallVertsUp, OutWallVertsUp, OutTerrainMesh.WallSection, 1);
		}

		if (bAddTower && CliffEdgeIndex < 0)
		{
			AddWallTowerFeature(FromVerts, ToVerts, RatioZs, OutTerrainMesh.Features);
		}
	}
}

void AHexTerrainGenerator::AddWallTowerFeature(const TArray<FHexVertexData>& FromVerts, const TArray<FHexVertexData>& ToVerts, const TArray<FVector2D>& RatioZ, TArray<FCachedFeatureData>& OutFeatures)
{
	int32 NumOfVerts = FromVerts.Num();
	for (int32 Index = 1; Index < NumOfVerts; ++Index)
	{
		FVector DirAlongWall = FromVerts[Index].Position - FromVerts[Index - 1].Position;
		if (DirAlongWall.IsNearlyZero())
			DirAlongWall = ToVerts[Index].Position - ToVerts[Index - 1].Position;

		if (FMath::IsNearlyZero(DirAlongWall.Z))
		{
			FVector FromCenter = (FromVerts[Index - 1].Position + FromVerts[Index].Position) * 0.5;
			FVector ToCenter = (ToVerts[Index - 1].Position + ToVerts[Index].Position) * 0.5;
			FVector FeatureLocation = (FromCenter + ToCenter) * 0.5;

			double LocZ0 = FMath::Lerp(FromCenter.Z, ToCenter.Z, RatioZ[Index - 1].X);
			double LocZ1 = FMath::Lerp(FromCenter.Z, ToCenter.Z, RatioZ[Index - 1].Y);
			FeatureLocation.Z = FMath::Min(LocZ0, LocZ1);

			FQuat TowerRot = FRotationMatrix::MakeFromX(DirAlongWall).ToQuat();

			FTransform FeatureTransform{ TowerRot, FeatureLocation, FVector{0.3, 0.3, 0.3} };
			OutFeatures.Add(FCachedFeatureData{ EHexFeatureType::WallTower, FeatureTransform });
		}
	}
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

	if ((FromV0_C.Position - FromV1_C.Position).IsNearlyZero())
	{
		OutTerrainMesh.AddTriangle(ToV0_C, FromV0_C, ToV1_C);
	}
	else if ((ToV0_C.Position - ToV1_C.Position).IsNearlyZero())
	{
		OutTerrainMesh.AddTriangle(ToV0_C, FromV0_C, FromV1_C);
	}
	else if ((ToV0_C.Position - FromV0_C.Position).IsNearlyZero())
	{
		OutTerrainMesh.AddTriangle(ToV0_C, FromV1_C, ToV1_C);
	}
	else if ((ToV1_C.Position - FromV1_C.Position).IsNearlyZero())
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
	if (Vertex.bPerturbed)
		return;

	Vertex.bPerturbed = true;
	PerturbingVertexInline(Vertex.Position, PerturbingStrengthHV, true);
}

FHexVertexData AHexTerrainGenerator::PerturbingVertex(const FHexVertexData& Vertex)
{
	if (!Vertex.bPerturbed)
	{
		FHexVertexData NewVert = Vertex;
		NewVert.bPerturbed = true;
		PerturbingVertexInline(NewVert.Position, PerturbingStrengthHV, true);
		return NewVert;
	}
	else
		return Vertex;
}

void AHexTerrainGenerator::GenerateRandomCache()
{
	static int32 RandomCacheSize = 512;
	RandomCache.Empty(RandomCacheSize);
	RandomCache.AddDefaulted(RandomCacheSize);
	for (int32 Y = 0; Y < RandomCacheSize; ++Y)
	{
		RandomCache[Y].AddDefaulted(RandomCacheSize);
		for (int32 X = 0; X < RandomCacheSize; ++X)
		{
			FVector4& OneRand = RandomCache[Y][X];
			OneRand.X = FMath::FRand();
			OneRand.Y = FMath::FRand();
			OneRand.Z = FMath::FRand();
			OneRand.W = FMath::FRand();
		}
	}
}

FVector4 AHexTerrainGenerator::GetRandomValueByPosition(const FVector& InVertex) const
{
	if (RandomCache.IsEmpty())
		return FVector::ZeroVector;

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
		FVector LocationLS = RootComponent->GetComponentTransform().InverseTransformPosition(HitResult.Location);
		GridId = CalcHexCellGridId(LocationLS);
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

	GenerateRandomCache();
	GenerateOrLoadTerrain();
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
	static FName Name_HexEditFeatureValue = GET_MEMBER_NAME_CHECKED(AHexTerrainGenerator, HexEditFeatureValue);
	
	static FName Name_ConfigFileName = GET_MEMBER_NAME_CHECKED(AHexTerrainGenerator, ConfigFileName);
	static FName Name_HexCellRadius = GET_MEMBER_NAME_CHECKED(AHexTerrainGenerator, HexCellRadius);
	static FName Name_HexCellBorderWidth = GET_MEMBER_NAME_CHECKED(AHexTerrainGenerator, HexCellBorderWidth);

	if (MemberPropertyName == Name_HexEditElevation || 
		MemberPropertyName == Name_HexEditTerrainType ||
		MemberPropertyName == Name_HexEditFeatureValue ||
		MemberPropertyName == Name_HexEditWaterLevel)
	{
		if (HexEditGridId.X < 0 || HexEditGridId.Y < 0)
			return;

		if (MemberPropertyName == Name_HexEditElevation)
			ConfigData.ElevationsList[HexEditGridId.Y][HexEditGridId.X] = HexEditElevation;
		else if (MemberPropertyName == Name_HexEditTerrainType)
			ConfigData.TerrainTypesList[HexEditGridId.Y][HexEditGridId.X] = HexEditTerrainType;
		else if (MemberPropertyName == Name_HexEditFeatureValue)
			ConfigData.FeatureValuesList[HexEditGridId.Y][HexEditGridId.X] = HexEditFeatureValue;
		else if (MemberPropertyName == Name_HexEditWaterLevel)
		{
			TSet<FIntPoint> ProcessedGrids;
			HexEditWater(ProcessedGrids, HexEditGridId, HexEditWaterLevel);
		}

		UpdateHexGridsData();
		CreateTerrain();
	}	
	else if (MemberPropertyName == Name_ConfigFileName)
	{
		GenerateOrLoadTerrain();
	}
	else if (MemberPropertyName == Name_HexCellRadius || MemberPropertyName == Name_HexCellBorderWidth)
	{
		UpdateHexGridsData();
		CreateTerrain();
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
		HexEditFeatureValue = ConfigData.FeatureValuesList[HitGridId.Y][HitGridId.X];
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
			CreateTerrain();
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
			CreateTerrain();
		}
		else if (HexEditRiverId >= 0)
		{
			ConfigData.RiversList.RemoveAt(HexEditRiverId);
			ClearEditParameters(EHexEditMode::River);

			UpdateHexGridsData();
			CreateTerrain();
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