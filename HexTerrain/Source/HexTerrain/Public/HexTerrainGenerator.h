#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "HexTerrainGenerator.generated.h"

enum class EHexDirection : uint8
{
	E, SE, SW, W, NW, NE
};

struct FHexCellData
{
	static int32 RowSize;
	static double ElevationStep;
	static TArray<FVector> HexVertices;

	int32 GridId;
	FIntPoint GridIndex;
	FIntVector GridCoord;
	FColor CellColor;
	int32 Elevation;

	int32 HexNeighbors[6]; // E, SE, SW, W, NW, NE

	FHexCellData(const FIntPoint& InIndex);
	void LinkCell(FHexCellData& OtherCell, EHexDirection LinkDirection);

	static FIntVector CalcGridCoordinate(const FIntPoint& InGridIndex);
	static EHexDirection CalcOppositeDirection(EHexDirection InDirection);
	static int32 CalcGridIndexByCoord(const FIntVector& InGridCoord);
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
	float HexCellRadius;

	UPROPERTY(EditAnywhere, Category = "HexTerrain")
	float HexCellBorderWidth;

	UPROPERTY(EditAnywhere, Category = "HexTerrain")
	float HexElevationStep;

	UPROPERTY(EditAnywhere, Category = "HexTerrain")
	FIntPoint HexGridSize;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void GenerateHexCell(const FHexCellData& InCellData, TArray<FVector>& OutVertices, TArray<int32>& OutIndices, TArray<FVector>& OutNormals, TArray<FColor>& OutColors);
	FVector CalcHexCellCenter(const FIntPoint& GridIndex, int32 Elevation);
	static FVector CalcFaceNormal(const FVector& V0, const FVector& V1, const FVector& V2);
	template<typename T>
	static void ArrayAddSelfItem(TArray<T>& InArray, int32 CopiedIndex);

protected:

	TArray<FHexCellData> HexGrids;
};
