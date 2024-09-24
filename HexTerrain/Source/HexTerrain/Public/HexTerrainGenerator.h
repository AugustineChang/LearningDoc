#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "HexTerrainGenerator.generated.h"

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

	UPROPERTY(Category = "HexTerrain", VisibleAnywhere, BlueprintReadOnly, Transient)
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
	FIntPoint HexGridSize;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void GenerateHexCell(const FIntPoint& GridIndex, TArray<FVector>& OutVertices, TArray<int32>& OutIndices);
};
