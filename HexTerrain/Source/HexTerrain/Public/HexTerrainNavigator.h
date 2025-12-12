#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HexTerrainNavigator.generated.h"

class AHexTerrainGenerator;
struct FNavigatorContext;
struct FNavigatorGrid;
struct FHexCellData;

UCLASS()
class HEXTERRAIN_API AHexTerrainNavigator : public AActor
{
	GENERATED_BODY()
	
	friend struct FNavigatorGrid;

public:	
	// Sets default values for this actor's properties
	AHexTerrainNavigator();

	UPROPERTY(EditAnywhere, Category="Navigator Debug")
	FIntPoint StartGridId;

	UPROPERTY(EditAnywhere, Category = "Navigator Debug")
	FIntPoint EndGridId;

public:

	UFUNCTION(CallInEditor, Category = "Navigator Debug")
	void DebugRunNavigator();

	UFUNCTION(BlueprintCallable, Category = "Navigator")
	bool RunNavigator(const FIntPoint& InStartId, const FIntPoint& InEndId, TArray<int32>& OutPathSteps);

	UFUNCTION(BlueprintCallable, Category = "Navigator")
	void GridIndicesToGridPositions(const TArray<int32>& InPathSteps, TArray<FVector>& OutPathPositions);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void InitNavigatorGrid(FNavigatorGrid& OutGrid, const FNavigatorContext& InContext, int32 InGridIndex, int32 InParentGridIndex);
	void UpdateNavigatorGrid(FNavigatorGrid& OutGrid, const FNavigatorContext& InContext, int32 InNewParentGridIndex);

	bool IsNeighborPassable(const FNavigatorContext& InContext, const FHexCellData& CurGrid, int32 NeighborDir);

	static int32 CalcDirectDistance(const FNavigatorContext& InContext, int32 InStartGridIndex, int32 InEndGridIndex);

	void AddDebugGridPathSteps(const TArray<FVector>& PathStepPositions);

protected:

	UPROPERTY(Category = "Navigator", BlueprintReadOnly)
	TObjectPtr<USceneComponent> RootSceneComponent;

	UPROPERTY(Category = "Navigator", BlueprintReadOnly, Transient)
	TObjectPtr<UInstancedStaticMeshComponent> CoordTextComponent;

	UPROPERTY(EditAnywhere, Category = "Navigator")
	TWeakObjectPtr<AHexTerrainGenerator> TerrainActor;

	UPROPERTY(EditAnywhere, Category = "Navigator")
	TObjectPtr<UMaterialInterface> DebugPathMaterial;
};