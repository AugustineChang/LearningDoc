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

enum class EHexDirection : uint8
{
	E, SE, SW, W, NW, NE
};

enum class EHexLinkState : uint8
{
	Flat, Slope, Terrace, Cliff
};

struct FHexCellLink
{
	int32 LinkedCellId;
	EHexLinkState LinkState;

	FIntPoint FromVert;
	FIntPoint ToVert;

	FHexCellLink()
		: LinkedCellId(-1), LinkState(EHexLinkState::Flat)
	{}
};

struct FHexCellCorner
{
	FIntVector LinkedCellsId;
	EHexLinkState LinkState[3];
	FIntVector VertsId;
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

	FHexCellLink HexNeighbors[6]; // E, SE, SW, W, NW, NE
	FHexCellCorner HexCorners[2]; // NW, N

	FHexCellData(const FIntPoint& InIndex);
	void LinkCell(FHexCellData& OtherCell, EHexDirection LinkDirection);
	void LinkCorner(FHexCellData& Cell1, FHexCellData& Cell2, EHexDirection LinkDirection);
	bool operator<(const FHexCellData& Other) const;

	static FIntVector CalcGridCoordinate(const FIntPoint& InGridIndex);
	static EHexDirection CalcOppositeDirection(EHexDirection InDirection);
	static int32 CalcGridIndexByCoord(const FIntVector& InGridCoord);
	static EHexLinkState CalcLinkState(const FHexCellData& Cell1, const FHexCellData& Cell2);
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
	FVector2D PerturbingStrengthHV;

	UPROPERTY(EditAnywhere, Category = "HexTerrain", AdvancedDisplay)
	FVector2D PerturbingScalingHV;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool LoadHexTerrainConfig(FHexCellConfigData& OutConfigData);

	void GenerateHexCell(const FHexCellData& InCellData, FCachedSectionData& OutCellMesh);
	void GenerateHexBorder(const FHexCellData& InCellData, EHexDirection BorderDirection, FCachedSectionData& OutCellMesh);
	void GenerateHexCorner(const FHexCellData& InCellData, EHexDirection CornerDirection, FCachedSectionData& OutCellMesh);

	void GenerateNoTerraceCorner(const FHexCellData& InCell1, const FHexCellData& InCell2, const FHexCellData& InCell3,
		const FHexCellCorner& CornerData, FCachedSectionData& OutCellMesh);
	void GenerateCornerWithTerrace(const FHexCellData& InCell1, const FHexCellData& InCell2, const FHexCellData& InCell3, 
		const FHexCellCorner& CornerData, FCachedSectionData& OutCellMesh);

	
	FVector CalcHexCellCenter(const FIntPoint& GridId, int32 Elevation);
	static FVector CalcFaceNormal(const FVector& V0, const FVector& V1, const FVector& V2);
	void FillQuad(const FVector& FromV0, const FVector& FromV1, const FVector& ToV0, const FVector& ToV1,
		const FColor& FromC0, const FColor& FromC1, const FColor& ToC0, const FColor& ToC1, FCachedSectionData& OutCellMesh);
	
	void PerturbingVertexInline(FVector& Vertex, int32 Elevation);
	FVector PerturbingVertex(const FVector& Vertex, int32 Elevation);
	FLinearColor SampleTextureBilinear(const TArray<TArray<FColor>>& InTexture, const FVector& SamplePos);
	FLinearColor SampleTextureBilinear(const TArray<TArray<FColor>>& InTexture, int32 SamplePosX, int32 SamplePosY);
	void CreateTextureFromData(TArray<TArray<FColor>>& OutTexture, const TArray<uint8>& InBineryData, EImageFormat InFormat);

protected:

	TArray<FHexCellData> HexGrids;
	TArray<TArray<FColor>> NoiseTexture;
	FUniqueVertexArray CacehdVertexData;
	TMap<int32, double> CachedNoiseZ;
};
