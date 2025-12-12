#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/GenericOctree.h"
#include "ProceduralMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "HexTerrainGenerator.generated.h"

struct FCachedSectionData;
struct FCachedFeatureData;
struct FCachedChunkData;
struct FCachedTerrainData;
struct FHexCellConfigData;
enum class EImageFormat : int8;

#define	CORNER_NUM 6
#define	CORNER_UNUM 6u
#define	CORNER_HALF_UNUM 3u

UENUM()
enum class EHexDirection : uint8
{
	E, SE, SW, W, NW, NE
};

UENUM()
enum class EHexBorderState : uint8
{
	Flat, Slope, Terrace, Cliff
};

UENUM()
enum class EHexTerrainType : uint8
{
	None, Ice, Water, Grass, Sand, Stone, Moor, MAX
};
UENUM()
enum class EHexTerrainTextureType : uint8
{
	None, 
	Ice1, Ice2, Ice3,
	Water1, Water2, Water3,
	Grass1, Grass2, Grass3,
	Sand1, Sand2, Sand3,
	Stone1, Stone2, Stone3,
	Moor1, Moor2, Moor3,
	MAX
};

UENUM()
enum class EHexFeatureType : uint8
{
	None, 
	Tree, Farm, Hovel, LowRise, HighRise, Tower, Castle, Temple, 
	Bridge, WallTower, MAX
};

UENUM()
enum class EHexRiverState : uint8
{
	None, StartPoint, EndPoint, PassThrough
};

UENUM()
enum class EHexEditMode : uint8
{
	Ground, Road, River
};

struct FHexCellBorder
{
	int32 LinkedCellIndex;
	EHexBorderState LinkState;

	FIntPoint FromVert;
	FIntPoint ToVert;

	FHexCellBorder()
		: LinkedCellIndex(-1), LinkState(EHexBorderState::Flat), FromVert(-1), ToVert(-1)
	{}
};

struct FHexCellCorner
{
	FIntVector LinkedCellsIndex;
	EHexBorderState LinkState[3];
	FIntVector VertsId;

	FHexCellCorner()
		: LinkedCellsIndex(-1,-1,-1), VertsId(-1,-1,-1)
	{
		LinkState[0] = EHexBorderState::Flat;
		LinkState[1] = EHexBorderState::Flat;
		LinkState[2] = EHexBorderState::Flat;
	}
};

struct FHexCellRiver
{
	int32 RiverIndex;

	EHexRiverState RiverState;
	EHexDirection IncomingDirection;
	EHexDirection OutgoingDirection;

	FHexCellRiver()
		: RiverIndex(-1), RiverState(EHexRiverState::None), IncomingDirection(EHexDirection::E), OutgoingDirection(EHexDirection::E)
	{}

	bool CheckRiver(EHexDirection InDirection) const
	{
		bool bHasRiverInDirection = false;
		switch (RiverState)
		{
		case EHexRiverState::StartPoint:
			bHasRiverInDirection = InDirection == OutgoingDirection;
			break;

		case EHexRiverState::EndPoint:
			bHasRiverInDirection = InDirection == IncomingDirection;
			break;

		case EHexRiverState::PassThrough:
			bHasRiverInDirection = (InDirection == OutgoingDirection || InDirection == IncomingDirection);
			break;
		}
		return bHasRiverInDirection;
	}

	void Clear()
	{
		RiverIndex = -1;
		RiverState = EHexRiverState::None;
		IncomingDirection = EHexDirection::E;
		OutgoingDirection = EHexDirection::E;
	}
};

struct FHexCellRoad
{
	int32 RoadIndex[CORNER_NUM];
	bool RoadState[CORNER_NUM]; // E, SE, SW, W, NW, NE

	FHexCellRoad()
	{
		for (uint32 Index = 0u; Index < CORNER_NUM; ++Index)
		{
			RoadIndex[Index] = -1;
			RoadState[Index] = false;
		}
	}

	uint32 GetPackedState() const
	{
		uint32 OutValue = 0u;
		for (uint32 Index = 0u; Index < CORNER_NUM; ++Index)
		{
			if (RoadState[Index])
				OutValue |= (1u << Index);
		}
		return OutValue;
	}

	void LinkRoad(int32 RoadId, EHexDirection LinkDirection)
	{
		uint8 LinkId = static_cast<uint8>(LinkDirection);
		RoadState[LinkId] = true;
		RoadIndex[LinkId] = RoadId;
	}
};

struct FHexCellFeature
{
	static int32 MaxFeatureValue;
	static int32 MaxDetailFeatureValue;
	int32 FeatureValue;
	uint32 bHasWall : 1;

	TArray<float> ProbabilityValues;
	TArray<EHexFeatureType> FeatureTypes;

	FHexCellFeature()
		: FeatureValue(0), bHasWall(false)
	{}

	void SetupFeature(int32 InFeatureValue);

	static FString GetHexFeatureString(EHexFeatureType InType)
	{
		switch (InType)
		{
		case EHexFeatureType::Tree:
			return TEXT("Tree");
		case EHexFeatureType::Farm:
			return TEXT("Farm");
		case EHexFeatureType::Hovel:
			return TEXT("Hovel");
		case EHexFeatureType::LowRise:
			return TEXT("LowRise");
		case EHexFeatureType::HighRise:
			return TEXT("HighRise");
		case EHexFeatureType::Tower:
			return TEXT("Tower");
		case EHexFeatureType::Castle:
			return TEXT("Castle");
		case EHexFeatureType::Temple:
			return TEXT("Temple");
		case EHexFeatureType::Bridge:
			return TEXT("Bridge");
		case EHexFeatureType::WallTower:
			return TEXT("WallTower");
		default:
			return TEXT("");
		}
	}
};

struct FHexCellData
{
	static FIntPoint ChunkSize;
	static FIntPoint ChunkCount;
	static uint8 CellSubdivision;
	static int32 MaxTerranceElevation;
	static TArray<FVector> HexVertices;
	static TArray<FVector> HexSubVertices;
	
	static FColor RoadColor;

	int32 GridIndex;
	FIntVector4 GridId;
	FIntVector GridCoord;

	FVector CellCenter;
	EHexTerrainType TerrainType;
	EHexTerrainTextureType TerrainTextureType;
	EHexTerrainTextureType WaterTextureType;
	//FColor SRGBColor;
	int32 Elevation;
	int32 WaterLevel;

	// Borders
	FHexCellBorder HexNeighbors[CORNER_NUM]; // E, SE, SW, W, NW, NE
	FHexCellCorner HexCorners[2]; // NW, N

	// River
	FHexCellRiver HexRiver;
	// Road
	FHexCellRoad HexRoad;
	// Feature
	FHexCellFeature HexFeature;

	FHexCellData(const FIntPoint& InIndex);
	void LinkBorder(FHexCellData& OtherCell, EHexDirection LinkDirection);
	void LinkCorner(FHexCellData& Cell1, FHexCellData& Cell2, EHexDirection LinkDirection);
	void LinkRoad(int32 RoadIndex, EHexDirection LinkDirection);
	bool operator<(const FHexCellData& Other) const;
	int32 GetWaterDepth() const { return WaterLevel - Elevation; }
	EHexTerrainTextureType GetTerrainTextureType() const { return GetWaterDepth() > 0 ? WaterTextureType : TerrainTextureType; }
	EHexRiverState GetTerrainRiverState() const { return GetWaterDepth() > 0 ? EHexRiverState::None : HexRiver.RiverState; }

	static FIntVector CalcGridCoordinate(const FIntPoint& InGridIndex);
	static EHexDirection CalcOppositeDirection(EHexDirection InDirection);
	static EHexDirection CalcPreviousDirection(EHexDirection InDirection);
	static EHexDirection CalcNextDirection(EHexDirection InDirection);
	static uint8 CalcDirectionsDistance(EHexDirection InDirectionA, EHexDirection InDirectionB);
	static uint8 CalcOppositeDirection(uint8 InDirection);
	static uint8 CalcPreviousDirection(uint8 InDirection);
	static uint8 CalcNextDirection(uint8 InDirection);
	static int32 CalcGridIndexByCoord(const FIntVector& InGridCoord);
	static EHexBorderState CalcLinkState(const FHexCellData& Cell1, const FHexCellData& Cell2);
	static uint8 GetVertIdFromDirection(EHexDirection InDirection, bool bSubVert = true, uint8 InState = 1u); // state: 0-start 1-mid 2-end
	static bool IsValidRiverDirection(const FHexCellData& FromCell, const FHexCellData& ToCell);
};

struct FHexVertexAttributeData
{
	FVector VertexPos;
	FVector NoiseVector;

	TSharedPtr<FOctreeElementId2> OctreeId;

	FHexVertexAttributeData(const FVector& InVertex, const TSharedPtr<FOctreeElementId2>& InIdPtr)
		: VertexPos(InVertex), NoiseVector(FVector::ZeroVector), OctreeId(InIdPtr)
	{
	}
};

struct FUniqueVertexArray
{
public:
	FUniqueVertexArray()
		: VectorOctree(FVector::ZeroVector, 750.0)
	{}

	FHexVertexAttributeData& FindOrAddVertex(const FVector& InVertex, bool& bFound);

private:

	static double VectorTolerence;

	struct FUniqueVectorOctreeSemantics
	{
		enum { MaxElementsPerLeaf = 16 };
		enum { MinInclusiveElementsPerNode = 7 };
		enum { MaxNodeDepth = 16 };

		typedef TInlineAllocator<MaxElementsPerLeaf, TAlignedHeapAllocator<alignof(FHexVertexAttributeData)>> ElementAllocator;

		FORCEINLINE static FBoxCenterAndExtent GetBoundingBox(const FHexVertexAttributeData& Element)
		{
			return FBoxCenterAndExtent{ FVector(Element.VertexPos), FVector::ZeroVector };
		}

		FORCEINLINE static void SetElementId(const FHexVertexAttributeData& Element, FOctreeElementId2 Id)
		{
			FOctreeElementId2* CachedIdPtr = Element.OctreeId.Get();
			*CachedIdPtr = Id;
		}

		FORCEINLINE static void ApplyOffset(FHexVertexAttributeData& Element, FVector Offset)
		{
			ensureMsgf(false, TEXT("Not implemented yet"));
		}
	};

	TOctree2<FHexVertexAttributeData, FUniqueVectorOctreeSemantics> VectorOctree;
};

struct FHexVertexData
{
	FHexVertexData() = delete;

	FHexVertexData(const FVector& InPos);

	FHexVertexData(const FVector& InPos, const FColor& InColor);

	FHexVertexData(const FVector& InPos, const FVector2D& InUV0);

	FHexVertexData(const FVector& InPos, const FVector& InNormal);

	FHexVertexData(const FVector& InPos, const FColor& InColor, const FVector& InNormal);

	FHexVertexData(const FVector& InPos, const FColor& InColor, const FVector2D& InUV0);

	FHexVertexData(const FVector& InPos, const FVector& InNormal, const FVector2D& InUV0);

	FHexVertexData(const FVector& InPos, const FColor& InColor, const FVector& InNormal, const FVector2D& InUV0);

	static FHexVertexData LerpVertex(const FHexVertexData& FromV, const FHexVertexData& ToV, FVector PosRatio, float AttrRatio);

	FHexVertexData ApplyOverride(const FVector& InPosOffset, const FColor* InOverrideColor = nullptr, const FVector2D* InOverrideUV0 = nullptr, bool bClear = true) const;
	void ApplyOverrideInline(const FVector& InPosOffset, const FColor* InOverrideColor = nullptr, const FVector2D* InOverrideUV0 = nullptr, bool bClear = true);

	void SetUV0(const FVector2D& InUV0)
	{
		UV0 = InUV0;
		bHasUV0 = true;
	}

	void SetUV1(const FVector2D& InUV1)
	{
		UV1 = InUV1;
		bHasUV1 = true;
	}

	void SetUV2(const FVector2D& InUV2)
	{
		UV2 = InUV2;
		bHasUV2 = true;
	}

	void SetVertexColor(const FColor& InColor)
	{
		VertexColor = InColor;
		bHasVertexColor = true;
	}

	void SetNormal(const FVector& InNormal)
	{
		Normal = InNormal;
		bHasNormal = true;
	}

	void SetTextureData(
		EHexTerrainTextureType InType1, EHexTerrainTextureType InType2, EHexTerrainTextureType InType3,
		float InWeight1, float InWeight2, float InWeight3)
	{
		VertexColor = FColor{ uint8(InType1), uint8(InType2), uint8(InType3) };
		UV0.X = InWeight1;
		UV0.Y = InWeight2;
		UV1.X = InWeight3;
		UV1.Y = 0.0f;

		bHasVertexColor = true;
		bHasUV0 = true;
		bHasUV1 = true;
	}

	void ClearProperties()
	{
		VertexColor = FColor::White;
		UV0 = FVector2D::ZeroVector;
		UV1 = FVector2D::ZeroVector;
		UV2 = FVector2D::ZeroVector;
		bHasVertexColor = false;
		bHasUV0 = false;
		bHasUV1 = false;
		bHasUV2 = false;
	}

	FVector Position;
	FVector Normal;
	FVector2D UV0;
	FVector2D UV1;
	FVector2D UV2;
	FColor VertexColor;

	uint32 bHasNormal : 1;
	uint32 bHasUV0 : 1;
	uint32 bHasUV1 : 1;
	uint32 bHasUV2 : 1;
	uint32 bHasVertexColor : 1;
	uint32 bPerturbed : 1;
	
	uint8 VertexState; //0-Ground 1-Water 2-Road
	int32 VertexIndex;
};

struct FHexRiverRoadConfigData
{
	FIntPoint StartPoint;
	TArray<EHexDirection> ExtensionDirections;
};

struct FHexCellConfigData
{
	static int32 DefaultElevation;
	static int32 DefaultWaterLevel;
	static EHexTerrainType DefaultTerrainType;
	static int32 DefaultFeatureValue;

	bool bConfigValid;
	TArray<TArray<int32>> ElevationsList;
	TArray<TArray<int32>> WaterLevelsList;
	TArray<TArray<EHexTerrainType>> TerrainTypesList;
	TArray<TArray<int32>> FeatureValuesList;
	TArray<FHexRiverRoadConfigData> RiversList;
	TArray<FHexRiverRoadConfigData> RoadsList;

	FHexCellConfigData()
		: bConfigValid(false)
	{}

	void GetHexCellTerrainData(const FIntPoint& GridId, FHexCellData& OutCell)
	{
		EHexTerrainType TerrainType = TerrainTypesList[GridId.Y][GridId.X];
		OutCell.TerrainType = TerrainType;
		if (TerrainType == EHexTerrainType::None || TerrainType == EHexTerrainType::MAX)
			OutCell.TerrainTextureType = EHexTerrainTextureType::None;
		else
		{
			uint8 TextureTypeId = (uint8(TerrainType) - 1u) * 3u + uint8(FMath::RandHelper(3)) + 1u;
			OutCell.TerrainTextureType = EHexTerrainTextureType(TextureTypeId);
		}
		uint8 WaterTypeId = uint8(EHexTerrainTextureType::Water1) + uint8(FMath::RandHelper(3));
		OutCell.WaterTextureType = EHexTerrainTextureType(WaterTypeId);

		OutCell.Elevation = ElevationsList[GridId.Y][GridId.X];
		OutCell.WaterLevel = WaterLevelsList[GridId.Y][GridId.X];
		OutCell.HexFeature.SetupFeature(FeatureValuesList[GridId.Y][GridId.X]);
	}

	static EHexTerrainType GetHexTerrainType(const FString& InTypeStr)
	{
		if (InTypeStr.Equals(TEXT("Ice")))
			return EHexTerrainType::Ice;
		else if (InTypeStr.Equals(TEXT("Water")))
			return EHexTerrainType::Water;
		else if (InTypeStr.Equals(TEXT("Grass")))
			return EHexTerrainType::Grass;
		else if (InTypeStr.Equals(TEXT("Sand")))
			return EHexTerrainType::Sand;
		else if (InTypeStr.Equals(TEXT("Stone")))
			return EHexTerrainType::Stone;
		else if (InTypeStr.Equals(TEXT("Moor")))
			return EHexTerrainType::Moor;
		else
			return EHexTerrainType::None;
	}

	static FString GetHexTerrainString(EHexTerrainType InType)
	{
		switch (InType)
		{
		case EHexTerrainType::Ice:
			return TEXT("Ice");
		case EHexTerrainType::Water:
			return TEXT("Water");
		case EHexTerrainType::Grass:
			return TEXT("Grass");
		case EHexTerrainType::Sand:
			return TEXT("Sand");
		case EHexTerrainType::Stone:
			return TEXT("Stone");
		case EHexTerrainType::Moor:
			return TEXT("Moor");
		default:
			return TEXT("");
		}
	}
};

UCLASS()
class HEXTERRAIN_API AHexTerrainGenerator : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AHexTerrainGenerator();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(CallInEditor, Category = "HexTerrain")
	void LoadTerrain();

	UFUNCTION(CallInEditor, Category = "HexTerrain")
	void SaveTerrain();

	UFUNCTION(CallInEditor, Category = "HexTerrain")
	void GenerateTerrain();

public:

	const TArray<FHexCellData>& GetHexGrids() const { return HexGrids; }

	UFUNCTION(BlueprintCallable, Category = "HexTerrain")
	FIntPoint CalcHexCellGridId(const FVector& WorldPos) const;

protected:

	UPROPERTY(Category = "HexTerrain", BlueprintReadOnly)
	TObjectPtr<USceneComponent> RootSceneComponent;

	UPROPERTY(Category = "HexTerrain", BlueprintReadOnly, Transient)
	TObjectPtr<UProceduralMeshComponent> TerrainMeshComponent;

	UPROPERTY(Category = "HexTerrain", BlueprintReadOnly, Transient)
	TMap<EHexFeatureType, TObjectPtr<UInstancedStaticMeshComponent>> FeatureMeshComponents;

	UPROPERTY(Category = "HexTerrain", BlueprintReadOnly, Transient)
	TObjectPtr<UInstancedStaticMeshComponent> CoordTextComponent;

	UPROPERTY(EditAnywhere, Category = "HexTerrain")
	TMap<FString, TObjectPtr<UMaterialInterface>> MaterialsLibrary;

	UPROPERTY(EditAnywhere, Category = "HexTerrain")
	TMap<FString, TObjectPtr<UStaticMesh>> ModelsLibrary;

	UPROPERTY(EditAnywhere, Category = "HexTerrain")
	FString ConfigFileName;

	UPROPERTY(VisibleAnywhere, Category = "HexTerrain")
	FIntPoint HexChunkCount;

	UPROPERTY(VisibleAnywhere, Category = "HexTerrain", AdvancedDisplay)
	FIntPoint HexChunkSize;

	UPROPERTY(EditAnywhere, Category = "HexTerrain", AdvancedDisplay)
	float HexCellRadius;

	UPROPERTY(EditAnywhere, Category = "HexTerrain", AdvancedDisplay)
	float HexCellBorderWidth;

	UPROPERTY(VisibleAnywhere, Category = "HexTerrain", AdvancedDisplay)
	uint8 HexCellSubdivision;

	UPROPERTY(EditAnywhere, Category = "HexTerrain", AdvancedDisplay)
	float HexElevationStep;

	UPROPERTY(EditAnywhere, Category = "HexTerrain", AdvancedDisplay)
	int32 MaxElevationForTerrace;

	UPROPERTY(VisibleAnywhere, Category = "HexTerrain", AdvancedDisplay)
	int32 RiverElevationOffset;

	UPROPERTY(VisibleAnywhere, Category = "HexTerrain", AdvancedDisplay)
	uint8 RiverSubdivision;

	UPROPERTY(VisibleAnywhere, Category = "HexTerrain", AdvancedDisplay)
	float RoadElevationOffset;

	UPROPERTY(VisibleAnywhere, Category = "HexTerrain", AdvancedDisplay)
	float RoadWidthRatio;

	UPROPERTY(EditAnywhere, Category = "HexTerrain", AdvancedDisplay)
	FString NoiseTexturePath;

	UPROPERTY(EditAnywhere, Category = "HexTerrain", AdvancedDisplay)
	FVector2D PerturbingStrengthHV;

	UPROPERTY(EditAnywhere, Category = "HexTerrain", AdvancedDisplay)
	FVector2D PerturbingScalingHV;

	UPROPERTY(EditAnywhere, Category = "HexTerrain", AdvancedDisplay)
	FVector2D PerturbingTest;

protected:
	
	UPROPERTY(EditAnywhere, Category = "HexTerrainEditor | Common")
	EHexEditMode HexEditMode;

	UPROPERTY(VisibleAnywhere, Category = "HexTerrainEditor | Cells")
	FIntPoint HexEditGridId;

	UPROPERTY(EditAnywhere, Category = "HexTerrainEditor | Cells")
	int32 HexEditElevation;

	UPROPERTY(EditAnywhere, Category = "HexTerrainEditor | Cells")
	int32 HexEditWaterLevel;

	UPROPERTY(EditAnywhere, Category = "HexTerrainEditor | Cells")
	EHexTerrainType HexEditTerrainType;

	UPROPERTY(EditAnywhere, Category = "HexTerrainEditor | Cells")
	int32 HexEditFeatureValue;

	UPROPERTY(VisibleAnywhere, Category = "HexTerrainEditor | Rivers")
	int32 HexEditRiverId;

	UPROPERTY(VisibleAnywhere, Category = "HexTerrainEditor | Rivers")
	FIntPoint HexEditRiverStartPoint;

	UPROPERTY(VisibleAnywhere, Category = "HexTerrainEditor | Rivers")
	FIntPoint HexEditRiverLastPoint;

	UPROPERTY(VisibleAnywhere, Category = "HexTerrainEditor | Rivers")
	TArray<FIntPoint> HexEditRiverPoints;

	UPROPERTY(VisibleAnywhere, Category = "HexTerrainEditor | Rivers")
	TArray<EHexDirection> HexEditRiverFlowDirections;

	UPROPERTY(VisibleAnywhere, Category = "HexTerrainEditor | Roads")
	FIntPoint HexEditRoadFirstPoint;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void AddTerrainFeatures(const FCachedTerrainData& CachedTerrain);
	void AddGridCoordinates(int32 HexGridSizeX, int32 HexGridSizeY);

	bool LoadHexTerrainConfig();
	void SaveHexTerrainConfig();
	void UpdateHexGridsData();

	void GenerateHexCell(const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh);
	void GenerateHexCenter(const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh);
	void GenerateHexBorder(const FHexCellData& InCellData, EHexDirection BorderDirection, FCachedChunkData& OutTerrainMesh);
	void GenerateHexCorner(const FHexCellData& InCellData, EHexDirection CornerDirection, FCachedChunkData& OutTerrainMesh);

	void GenerateNoRiverCenter(const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh);
	void GenerateCenterWithRiverEnd(const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh);
	void GenerateCenterWithRiverThrough(const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh);
	
	void GenerateNoTerraceCorner(const FHexCellData& InCell1, const FHexCellData& InCell2, const FHexCellData& InCell3,
		const FHexCellCorner& CornerData, FCachedChunkData& OutTerrainMesh);
	void GenerateCornerWithTerrace(const FHexCellData& InCell1, const FHexCellData& InCell2, const FHexCellData& InCell3,
		const FHexCellCorner& CornerData, FCachedChunkData& OutTerrainMesh);

	bool GenerateRoadCenter(const FHexVertexData& CenterV, const TArray<FHexVertexData>& EdgesV, FCachedChunkData& OutTerrainMesh);
	void GenerateRoadCenterWithRiver(const FHexCellData& InCellData, const FHexVertexData& CenterV, const TArray<FHexVertexData>& EdgesV, FCachedChunkData& OutTerrainMesh);
	void AddRoadBridgeFeature(const FHexCellData& InCellData, const FHexVertexData& CenterLV, const FHexVertexData& CenterRV, const FVector& OffsetDir, TArray<FCachedFeatureData>& OutFeatures);
	
	void GenerateHexWaterCell(const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh);
	void GenerateHexWaterCenter(const FHexCellData& InCellData, FCachedChunkData& OutTerrainMesh);
	void GenerateHexWaterBorder(const FHexCellData& InCellData, EHexDirection BorderDirection, FCachedChunkData& OutTerrainMesh);
	void GenerateHexWaterCorner(const FHexCellData& InCellData, EHexDirection CornerDirection, FCachedChunkData& OutTerrainMesh);

	FVector CalcHexCellCenter(const FIntPoint& GridId, int32 Elevation) const;
	FHexVertexData CalcHexCellVertex(const FHexCellData& InCellData, int32 VertIndex, bool bSubVert, uint8 VertType = 0u, bool bFillDefaultNormal = false) const;
	//FIntPoint CalcHexCellGridId(const FVector& WorldPos) const;
	int32 CalcDiffToRoadVert(const TArray<int32>& RoadVertIndices, int32 CurIndex) const;
	float CalcRoadWidthScale(int32 DiffToRoad) const;

	FVector CalcRiverVertOffset() const 
	{
		double RiverElevOffset = static_cast<double>(RiverElevationOffset);
		return FVector::UpVector * RiverElevOffset * HexElevationStep;
	}

	float CalcRiverUVScale(bool bBorder = false, int32 ElevDiff = 0) const
	{
		if (bBorder)
		{
			float RealBoderLength = ElevDiff <= 0 ? HexCellBorderWidth : FMath::Sqrt(FMath::Square(HexCellBorderWidth) + FMath::Square(ElevDiff * HexElevationStep));
			return FMath::RoundToFloat(RealBoderLength / HexCellBorderWidth);
		}
		else
		{
			return FMath::RoundToFloat(0.4330127f * HexCellRadius / HexCellBorderWidth);
		}
	}

	FVector CalcRoadVertOffset() const
	{
		return FVector::UpVector * RoadElevationOffset;
	}

	FVector CalcWaterVertOffset(int32 WaterLevelOffset = 0) const
	{
		double WaterElevOffset = static_cast<double>(WaterLevelOffset);
		WaterElevOffset -= 0.5;

		return FVector::UpVector * WaterElevOffset * HexElevationStep;
	}

	FVector CalcWallVertOffset() const
	{
		return FVector::UpVector * HexElevationStep;
	}

	static FVector CalcFaceNormal(const FVector& V0, const FVector& V1, const FVector& V2);
	
	void AddDetailFeature(const FHexCellData& InCellData, const FVector& InCenter, int32 LocDirectionId, TArray<FCachedFeatureData>& OutFeatures);
	void AddLargeFeature(const FHexCellData& InCellData, const FVector& InCenter, TArray<FCachedFeatureData>& OutFeatures);
	void GenerateWallFeature(const TArray<FHexVertexData>& FromVerts, const TArray<FHexVertexData>& ToVerts, const TArray<FIntPoint>& AttributesList, bool bToVertsInWall, bool bAddTower, FCachedChunkData& OutTerrainMesh);
	void AddWallTowerFeature(const TArray<FHexVertexData>& FromVerts, const TArray<FHexVertexData>& ToVerts, const TArray<FVector2D>& RatioZ, TArray<FCachedFeatureData>& OutFeatures);

	void FillGrid(const TArray<FHexVertexData>& FromV, const TArray<FHexVertexData>& ToV, FCachedSectionData& OutTerrainMesh,
		int32 NumOfSteps, bool bTerrace = false, bool bClosed = false, bool bRotTriangle = false);
	void FillStrip(const FHexVertexData& FromV0, const FHexVertexData& FromV1, const FHexVertexData& ToV0, const FHexVertexData& ToV1,
		FCachedSectionData& OutTerrainMesh, int32 NumOfSteps, bool bTerrace = false, bool bRotTriangle = false);
	void FillQuad(const FHexVertexData& FromV0, const FHexVertexData& FromV1, const FHexVertexData& ToV0, const FHexVertexData& ToV1, FCachedSectionData& OutTerrainMesh, bool bRotTriangle = false);
	void FillFan(const FHexVertexData& CenterV, const TArray<FHexVertexData>& EdgesV, const TArray<bool>& bRecalcNormal, FCachedSectionData& OutTerrainMesh, bool bClosed = false);

	void PerturbingVertexInline(FVector& Vertex, const FVector2D& Strength, bool bPerturbZ);
	FVector PerturbingVertex(const FVector& Vertex, const FVector2D& Strength, bool bPerturbZ);
	void PerturbingVertexInline(FHexVertexData& Vertex);
	FHexVertexData PerturbingVertex(const FHexVertexData& Vertex);
	
	FVector4 GetRandomValueByPosition(const FVector& InVertex) const;
	FLinearColor SampleTextureBilinear(const TArray<TArray<FColor>>& InTexture, const FVector& SamplePos) const;
	FLinearColor SampleTextureBilinear(const TArray<TArray<FColor>>& InTexture, int32 SamplePosX, int32 SamplePosY) const;
	void CreateTextureFromData(TArray<TArray<FColor>>& OutTexture, const TArray<uint8>& InBineryData, EImageFormat InFormat);

	UFUNCTION()
	void OnClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);
	UFUNCTION()
	void OnReleased(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	void HexEditGround(bool bHit, const FIntPoint& HitGridId);
	void HexEditRoad(bool bHit, const FIntPoint& HitGridId);
	void HexEditRiver(bool bHit, const FIntPoint& HitGridId);
	void HexEditWater(TSet<FIntPoint>& ProcessedGrids, const FIntPoint& CurGridId, int32 NewWaterLevel);
	void ClearEditParameters(EHexEditMode ModeToClear);

public:
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:

	TArray<FHexCellData> HexGrids;
	TArray<TArray<FColor>> NoiseTexture;

	TArray<TArray<FVector4>> RandomCache;
	TMap<int32, double> CachedNoiseZ;
	FHexCellConfigData ConfigData;
	TObjectPtr<APlayerController> PlayerController;
};
