#include "HexTerrainNavigator.h"
#include "HexTerrainGenerator.h"

struct FNavigatorGrid
{
	int32 GridIndex;
	int32 ParentGridIndex;

	int32 FValue; // G + H
	int32 GValue; // Start -> this
	int32 HValue; // this -> End
};

struct FNavigatorContext 
{
	FNavigatorContext(TWeakObjectPtr<AHexTerrainGenerator> InTerrainActor, const FIntPoint& InStartId, const FIntPoint& InEndId)
		: bIsValid(false), ChunkSize(FHexCellData::ChunkSize), ChunkCount(FHexCellData::ChunkCount),
		HexGrids(InTerrainActor->GetHexGrids()), ResultGridIndex(-1)
	{
		if (HexGrids.Num() <= 0 || ChunkSize == FIntPoint::ZeroValue || ChunkCount == FIntPoint::ZeroValue)
			return;
		
		int32 HexGridSizeX = ChunkSize.X * ChunkCount.X;
		int32 HexGridSizeY = ChunkSize.Y * ChunkCount.Y;
		if (InStartId.X < 0 || InStartId.X >= HexGridSizeX ||
			InStartId.Y < 0 || InStartId.Y >= HexGridSizeY ||
			InEndId.X < 0 || InEndId.X >= HexGridSizeX ||
			InEndId.Y < 0 || InEndId.Y >= HexGridSizeY)
			return;
		
		StartGridIndex = InStartId.X + InStartId.Y * HexGridSizeX;
		EndGridIndex = InEndId.X + InEndId.Y * HexGridSizeX;
		if (StartGridIndex == EndGridIndex)
			return;

		const FHexCellData& StartGrid = HexGrids[StartGridIndex];
		const FHexCellData& EndGrid = HexGrids[EndGridIndex];
		if (StartGrid.GetWaterDepth() > 0 || EndGrid.GetWaterDepth() > 0)
			return;

		bIsValid = true;
	}

	FIntPoint GridIndex1DTo2D(int32 GridIndex) const
	{
		int32 HexGridSizeX = ChunkSize.X * ChunkCount.X;

		FIntPoint OutId;
		OutId.Y = GridIndex / HexGridSizeX;
		OutId.X = GridIndex - OutId.Y * HexGridSizeX;

		return OutId;
	}

	bool bIsValid;
	const FIntPoint& ChunkSize;
	const FIntPoint& ChunkCount;
	const TArray<FHexCellData>& HexGrids;

	int32 StartGridIndex;
	int32 EndGridIndex;
	int32 ResultGridIndex;
	TMap<int32, FNavigatorGrid> OpenGrids;
	TMap<int32, FNavigatorGrid> ClosedGrids;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Sets default values
AHexTerrainNavigator::AHexTerrainNavigator()
	: AActor(), StartGridId(ForceInitToZero), EndGridId(ForceInitToZero)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;

	CoordTextComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GridCoordComponent"));
	CoordTextComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoordTextComponent->Mobility = EComponentMobility::Movable;
	CoordTextComponent->SetGenerateOverlapEvents(false);
	CoordTextComponent->SetCastShadow(false);
	CoordTextComponent->SetupAttachment(RootComponent);
	CoordTextComponent->SetRelativeLocation(FVector{ 0.0, 0.0, 20.0 });

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultPlane(TEXT("/Engine/BasicShapes/Plane"));
	CoordTextComponent->SetStaticMesh(DefaultPlane.Object);
}

void AHexTerrainNavigator::DebugRunNavigator()
{
	TArray<int32> PathSteps;
	if (RunNavigator(StartGridId, EndGridId, PathSteps))
	{
		TArray<FVector> PathStepPostions;
		GridIndicesToGridPositions(PathSteps, PathStepPostions);
		AddDebugGridPathSteps(PathStepPostions);
	}
}

bool AHexTerrainNavigator::RunNavigator(const FIntPoint& InStartId, const FIntPoint& InEndId, TArray<int32>& OutPathSteps)
{
	if (!TerrainActor.IsValid())
		return false;

	FNavigatorContext NaviContext{ TerrainActor, InStartId, InEndId };
	if (!NaviContext.bIsValid)
		return false;

	int32 CurGridIndex = NaviContext.StartGridIndex;
	while (true)
	{
		const FHexCellData& CurGrid = NaviContext.HexGrids[CurGridIndex];

		// Check reach EndGrid
		bool CachedPassable[CORNER_NUM];
		for (int32 EdgeIndex = 0; EdgeIndex < CORNER_NUM; ++EdgeIndex)
		{
			CachedPassable[EdgeIndex] = IsNeighborPassable(NaviContext, CurGrid, EdgeIndex);
			if (!CachedPassable[EdgeIndex])
				continue;

			int32 NeighborIndex = CurGrid.HexNeighbors[EdgeIndex].LinkedCellIndex;
			if (NeighborIndex == NaviContext.EndGridIndex)
			{
				NaviContext.ResultGridIndex = CurGridIndex;
				break;
			}
		}
		if (NaviContext.ResultGridIndex >= 0)
			break; // Stop, find EndGrid

		// Continue finding...
		for (int32 EdgeIndex = 0; EdgeIndex < CORNER_NUM; ++EdgeIndex)
		{
			if (!CachedPassable[EdgeIndex])
				continue;

			int32 NeighborIndex = CurGrid.HexNeighbors[EdgeIndex].LinkedCellIndex;
			if (NaviContext.ClosedGrids.Contains(NeighborIndex))
				continue;

			if (NaviContext.OpenGrids.Contains(NeighborIndex))
				UpdateNavigatorGrid(NaviContext.OpenGrids[NeighborIndex], NaviContext, CurGridIndex);
			else
			{
				FNavigatorGrid NaviGrid;
				InitNavigatorGrid(NaviGrid, NaviContext, NeighborIndex, CurGridIndex);
				NaviContext.OpenGrids.Add(NeighborIndex, NaviGrid);
			}
		}
		
		if (NaviContext.OpenGrids.Num() > 0)
		{
			NaviContext.OpenGrids.ValueSort([](const FNavigatorGrid& Grid0, const FNavigatorGrid& Grid1)
				{
					return Grid0.FValue < Grid1.FValue;
				});

			TPair<int32, FNavigatorGrid> FirstGrid = NaviContext.OpenGrids.Get(FSetElementId::FromInteger(0));
			CurGridIndex = FirstGrid.Key;
			NaviContext.ClosedGrids.Add(FirstGrid);
			NaviContext.OpenGrids.Remove(CurGridIndex);
		}
		else
			break; // Stop, not find EndGrid and no grid to search
	}

	if (NaviContext.ResultGridIndex >= 0)
	{
		UE_LOG(LogTemp, Display, TEXT("Found a Path to EndGrid"));
		OutPathSteps.Empty();
		OutPathSteps.Add(NaviContext.EndGridIndex);

		int32 TempGridIndex = NaviContext.ResultGridIndex;
		while (TempGridIndex != NaviContext.StartGridIndex)
		{
			OutPathSteps.Insert(TempGridIndex, 0);
			TempGridIndex = NaviContext.ClosedGrids[TempGridIndex].ParentGridIndex;
		}
		OutPathSteps.Insert(NaviContext.StartGridIndex, 0);
		
		return true;
	}
	else
		return false;
}

void AHexTerrainNavigator::GridIndicesToGridPositions(const TArray<int32>& InPathSteps, TArray<FVector>& OutPathPositions)
{
	const TArray<FHexCellData>& HexGrids = TerrainActor->GetHexGrids();

	int32 NumOfSteps = InPathSteps.Num();
	OutPathPositions.Empty(NumOfSteps);
	for (int32 Index = 0; Index < NumOfSteps; ++Index)
	{	
		OutPathPositions.Add(HexGrids[InPathSteps[Index]].CellCenter);
	}
}

// Called when the game starts or when spawned
void AHexTerrainNavigator::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AHexTerrainNavigator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHexTerrainNavigator::InitNavigatorGrid(FNavigatorGrid& OutGrid, const FNavigatorContext& InContext, int32 InGridIndex, int32 InParentGridIndex)
{
	OutGrid.GridIndex = InGridIndex;
	OutGrid.ParentGridIndex = InParentGridIndex;

	// G
	OutGrid.GValue = 0;
	if (InParentGridIndex != InContext.StartGridIndex)
		OutGrid.GValue = InContext.ClosedGrids[InParentGridIndex].GValue;
	++OutGrid.GValue;

	// H
	OutGrid.HValue = AHexTerrainNavigator::CalcDirectDistance(InContext, InGridIndex, InContext.EndGridIndex);

	OutGrid.FValue = OutGrid.GValue + OutGrid.HValue;
}

void AHexTerrainNavigator::UpdateNavigatorGrid(FNavigatorGrid& OutGrid, const FNavigatorContext& InContext, int32 InNewParentGridIndex)
{
	int32 NewGValue = 0;
	if (InNewParentGridIndex != InContext.StartGridIndex)
		NewGValue = InContext.ClosedGrids[InNewParentGridIndex].GValue;
	++NewGValue;

	if (NewGValue < OutGrid.GValue)
	{
		OutGrid.ParentGridIndex = InNewParentGridIndex;
		OutGrid.GValue = NewGValue;
		OutGrid.FValue = OutGrid.GValue + OutGrid.HValue;
	}
}

bool AHexTerrainNavigator::IsNeighborPassable(const FNavigatorContext& InContext, const FHexCellData& CurGrid, int32 NeighborDir)
{
	const FHexCellBorder& HexNeighbor = CurGrid.HexNeighbors[NeighborDir];
	int32 NeighborIndex = HexNeighbor.LinkedCellIndex;
	if (NeighborIndex < 0 || HexNeighbor.LinkState >= EHexBorderState::Cliff)
		return false;

	const FHexCellData& NeighborGrid = InContext.HexGrids[NeighborIndex];
	if (NeighborGrid.GetWaterDepth() > 0)
		return false;

	bool bHasWall = NeighborGrid.HexFeature.bHasWall != CurGrid.HexFeature.bHasWall;
	if (bHasWall && !CurGrid.HexRoad.RoadState[NeighborDir])
		return false;

	return true;
}

int32 AHexTerrainNavigator::CalcDirectDistance(const FNavigatorContext& InContext, int32 InStartGridIndex, int32 InEndGridIndex)
{
	const FHexCellData& StartCellData = InContext.HexGrids[InStartGridIndex];
	const FHexCellData& EndCellData = InContext.HexGrids[InEndGridIndex];

	FIntVector Diff = EndCellData.GridCoord - StartCellData.GridCoord;
	Diff.X = FMath::Abs(Diff.X);
	Diff.Y = FMath::Abs(Diff.Y);
	Diff.Z = FMath::Abs(Diff.Z);
	return Diff.X + Diff.Y + Diff.Z - Diff.GetMax();
}

void AHexTerrainNavigator::AddDebugGridPathSteps(const TArray<FVector>& PathStepPositions)
{
	if (!DebugPathMaterial)
		return;

	CoordTextComponent->SetMaterial(0, DebugPathMaterial);

	CoordTextComponent->ClearInstances();
	CoordTextComponent->SetNumCustomDataFloats(1);

	int32 NumOfSteps = PathStepPositions.Num();
	for (int32 Index = 0; Index < NumOfSteps; ++Index)
	{
		FTransform Instance{ PathStepPositions[Index] };

		CoordTextComponent->AddInstance(Instance, false);
		CoordTextComponent->SetCustomDataValue(Index, 0, Index);
	}
	CoordTextComponent->MarkRenderStateDirty();
}