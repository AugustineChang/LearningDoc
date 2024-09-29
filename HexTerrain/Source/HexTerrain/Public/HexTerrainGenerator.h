#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "HexTerrainGenerator.generated.h"

struct FCachedSectionData;

enum class EHexDirection : uint8
{
	E, SE, SW, W, NW, NE
};

enum class EHexLinkState : uint8
{
	Plane, Slope, Terrace, Cliff
};

struct FHexCellLink
{
	int32 LinkedCellId;
	EHexLinkState LinkState;

	FIntPoint FromVert;
	FIntPoint ToVert;

	FHexCellLink()
		: LinkedCellId(-1), LinkState(EHexLinkState::Plane)
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
	static int32 RowSize;
	static int32 MaxTerranceElevation;
	static TArray<FVector> HexVertices;

	int32 GridId;
	FIntPoint GridIndex;
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
	FIntPoint HexGridSize;

	UPROPERTY(EditAnywhere, Category = "HexTerrain")
	float HexCellRadius;

	UPROPERTY(EditAnywhere, Category = "HexTerrain")
	float HexCellBorderWidth;

	UPROPERTY(EditAnywhere, Category = "HexTerrain")
	float HexElevationStep;

	UPROPERTY(EditAnywhere, Category = "HexTerrain")
	int32 MaxElevationForTerrace;
	
	UPROPERTY(EditAnywhere, Category = "HexTerrain")
	TArray<int32> DebugElevation;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void GenerateHexCell(const FHexCellData& InCellData, FCachedSectionData& OutCellMesh);
	void GenerateHexBorder(const FHexCellData& InCellData, EHexDirection BorderDirection, FCachedSectionData& OutCellMesh);
	void GenerateHexCorner(const FHexCellData& InCellData, EHexDirection CornerDirection, FCachedSectionData& OutCellMesh);

	void GenerateNoTerraceCorner(const FHexCellData& InCell1, const FHexCellData& InCell2, const FHexCellData& InCell3,
		const FHexCellCorner& CornerData, FCachedSectionData& OutCellMesh);
	void GenerateCornerWithTerrace(const FHexCellData& InCell1, const FHexCellData& InCell2, const FHexCellData& InCell3, 
		const FHexCellCorner& CornerData, FCachedSectionData& OutCellMesh);

	FVector CalcHexCellCenter(const FIntPoint& GridIndex, int32 Elevation);
	static FVector CalcFaceNormal(const FVector& V0, const FVector& V1, const FVector& V2);
	void FillQuad(const FVector& FromV0, const FVector& FromV1, const FVector& ToV0, const FVector& ToV1,
		const FColor& FromC0, const FColor& FromC1, const FColor& ToC0, const FColor& ToC1, FCachedSectionData& OutCellMesh);

protected:

	TArray<FHexCellData> HexGrids;
};
