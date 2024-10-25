#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/GenericOctree.h"
#include "ProceduralMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "HexTerrainGenerator.generated.h"

struct FCachedSectionData;
struct FHexCellConfigData;
enum class EImageFormat : int8;

UENUM()
enum class EHexDirection : uint8
{
	E, SE, SW, W, NW, NE
};

enum class EHexBorderState : uint8
{
	Flat, Slope, Terrace, Cliff
};

UENUM()
enum class EHexTerrainType : uint8
{
	None, Ice, Water, Grass, Sand, MAX
};

enum class EHexRiverState : uint8
{
	None, StartPoint, EndPoint, PassThrough
};

UENUM()
enum class EHexEditMode : uint8
{
	Cell, River
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
};

struct FHexCellData
{
	static FIntPoint ChunkSize;
	static int32 ChunkCountX;
	static int32 MaxTerranceElevation;
	static TArray<FVector> HexVertices;
	static TArray<FVector> HexSubVertices;

	int32 GridIndex;
	FIntVector4 GridId;
	FIntVector GridCoord;

	FVector CellCenter;
	//FLinearColor LinearColor;
	FColor SRGBColor;
	int32 Elevation;

	// Borders
	FHexCellBorder HexNeighbors[6]; // E, SE, SW, W, NW, NE
	FHexCellCorner HexCorners[2]; // NW, N

	// River
	FHexCellRiver HexRiver;

	FHexCellData(const FIntPoint& InIndex);
	void LinkBorder(FHexCellData& OtherCell, EHexDirection LinkDirection);
	void LinkCorner(FHexCellData& Cell1, FHexCellData& Cell2, EHexDirection LinkDirection);
	bool operator<(const FHexCellData& Other) const;

	static FIntVector CalcGridCoordinate(const FIntPoint& InGridIndex);
	static EHexDirection CalcOppositeDirection(EHexDirection InDirection);
	static int32 CalcGridIndexByCoord(const FIntVector& InGridCoord);
	static EHexBorderState CalcLinkState(const FHexCellData& Cell1, const FHexCellData& Cell2);
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

struct FHexRiverConfigData
{
	FIntPoint RiverStartPoint;
	TArray<EHexDirection> RiverFlowDirections;
};

struct FHexCellConfigData
{
	static int32 DefaultElevation;
	static EHexTerrainType DefaultTerrainType;

	bool bConfigValid;
	TArray<TArray<int32>> ElevationsList;
	TArray<TArray<EHexTerrainType>> TerrainTypesList;
	TArray<FHexRiverConfigData> RiversList;
	TMap<EHexTerrainType, FColor> ColorsMap;

	FHexCellConfigData()
		: bConfigValid(false)
	{}

	void GetHexCellTerrainData(const FIntPoint& GridId, FColor& OutColor, int32& OutElevation)
	{
		EHexTerrainType TerrainType = TerrainTypesList[GridId.Y][GridId.X];
		OutColor = ColorsMap[TerrainType];
		OutElevation = ElevationsList[GridId.Y][GridId.X];
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

protected:

	UPROPERTY(Category = "HexTerrain", BlueprintReadOnly, Transient)
	TObjectPtr<UProceduralMeshComponent> ProceduralMeshComponent;

	UPROPERTY(Category = "HexTerrain", BlueprintReadOnly, Transient)
	TObjectPtr<UInstancedStaticMeshComponent> CoordTextComponent;

	UPROPERTY(EditAnywhere, Category = "HexTerrain")
	TObjectPtr<UMaterialInterface> HexTerrainMaterial;

	UPROPERTY(EditAnywhere, Category = "HexTerrain")
	TObjectPtr<UMaterialInterface> TextMaterial;

	UPROPERTY(EditAnywhere, Category = "HexTerrain")
	FString NoiseTexturePath;

	UPROPERTY(EditAnywhere, Category = "HexTerrain")
	FIntPoint HexChunkCount;

	UPROPERTY(EditAnywhere, Category = "HexTerrain", AdvancedDisplay)
	FIntPoint HexChunkSize;

	UPROPERTY(EditAnywhere, Category = "HexTerrain", AdvancedDisplay)
	float HexCellRadius;

	UPROPERTY(EditAnywhere, Category = "HexTerrain", AdvancedDisplay)
	float HexCellBorderWidth;

	UPROPERTY(EditAnywhere, Category = "HexTerrain", AdvancedDisplay)
	uint8 HexCellSubdivision;

	UPROPERTY(EditAnywhere, Category = "HexTerrain", AdvancedDisplay)
	float HexElevationStep;

	UPROPERTY(EditAnywhere, Category = "HexTerrain", AdvancedDisplay)
	int32 MaxElevationForTerrace;

	UPROPERTY(EditAnywhere, Category = "HexTerrain", AdvancedDisplay)
	int32 RiverElevationOffset;

	UPROPERTY(EditAnywhere, Category = "HexTerrain", AdvancedDisplay)
	FVector2D PerturbingStrengthHV;

	UPROPERTY(EditAnywhere, Category = "HexTerrain", AdvancedDisplay)
	FVector2D PerturbingScalingHV;

protected:
	
	UPROPERTY(EditAnywhere, Category = "HexTerrainEditor | Common")
	EHexEditMode HexEditMode;

	UPROPERTY(VisibleAnywhere, Category = "HexTerrainEditor | Cells")
	FIntPoint HexEditGridId;

	UPROPERTY(EditAnywhere, Category = "HexTerrainEditor | Cells")
	int32 HexEditElevation;

	UPROPERTY(EditAnywhere, Category = "HexTerrainEditor | Cells")
	EHexTerrainType HexEditTerrainType;

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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool LoadHexTerrainConfig();
	void UpdateHexTerrainConfig();
	void SaveHexTerrainConfig();

	void UpdateHexGridsData();

	void GenerateHexCell(const FHexCellData& InCellData, FCachedSectionData& OutCellMesh, FCachedSectionData& OutCellCollisionMesh);
	void GenerateHexCenter(const FHexCellData& InCellData, FCachedSectionData& OutCellMesh);
	void GenerateHexBorder(const FHexCellData& InCellData, EHexDirection BorderDirection, FCachedSectionData& OutCellMesh);
	void GenerateHexCorner(const FHexCellData& InCellData, EHexDirection CornerDirection, FCachedSectionData& OutCellMesh);
	
	void GenerateNoTerraceCorner(const FHexCellData& InCell1, const FHexCellData& InCell2, const FHexCellData& InCell3,
		const FHexCellCorner& CornerData, FCachedSectionData& OutCellMesh);
	void GenerateCornerWithTerrace(const FHexCellData& InCell1, const FHexCellData& InCell2, const FHexCellData& InCell3, 
		const FHexCellCorner& CornerData, FCachedSectionData& OutCellMesh);
	
	FVector CalcHexCellCenter(const FIntPoint& GridId, int32 Elevation) const;
	FVector CalcHexCellVertex(const FHexCellData& InCellData, int32 VertIndex, bool bSubVert) const;
	FVector CalcHexCellVertex(const FHexCellData& InCellData, int32 VertIndex, bool bSubVert, bool &bOutRiverVert) const;
	FIntPoint CalcHexCellGridId(const FVector& WorldPos);
	static FVector CalcFaceNormal(const FVector& V0, const FVector& V1, const FVector& V2);
	void FillQuad(const FVector& FromV0, const FVector& FromV1, const FVector& ToV0, const FVector& ToV1,
		const FColor& FromC0, const FColor& FromC1, const FColor& ToC0, const FColor& ToC1, FCachedSectionData& OutCellMesh);
	
	void PerturbingVertexInline(FVector& Vertex, int32 Elevation);
	FVector PerturbingVertex(const FVector& Vertex, int32 Elevation);
	FLinearColor SampleTextureBilinear(const TArray<TArray<FColor>>& InTexture, const FVector& SamplePos);
	FLinearColor SampleTextureBilinear(const TArray<TArray<FColor>>& InTexture, int32 SamplePosX, int32 SamplePosY);
	void CreateTextureFromData(TArray<TArray<FColor>>& OutTexture, const TArray<uint8>& InBineryData, EImageFormat InFormat);

	UFUNCTION()
	void OnClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);
	UFUNCTION()
	void OnReleased(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	void ClearEditParameters(EHexEditMode ModeToClear);

public:
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:

	TArray<FHexCellData> HexGrids;
	TArray<TArray<FColor>> NoiseTexture;
	FUniqueVertexArray CacehdVertexData;
	TMap<int32, double> CachedNoiseZ;
	FHexCellConfigData ConfigData;
	TObjectPtr<APlayerController> PlayerController;
};
