#pragma once

#include "CoreMinimal.h"

enum class EImageFormat : int8;

class FHexTerrainNoiser
{
public:
	FHexTerrainNoiser();

	void InitNoiser(const FString& InNoiseTexturePath);
	bool IsValid() const { return bIsValid; }
	FVector4 GetRandomValueByPosition(const FVector& InVertex) const;
	FLinearColor SampleTextureBilinear(const FVector& SamplePos, bool bRatio = false) const;
	FLinearColor SampleTextureNearest(int32 SamplePosX, int32 SamplePosY) const;

private:

	void GenerateRandomCache();
	void CreateTextureFromData(TArray<TArray<FColor>>& OutTexture, const TArray<uint8>& InBineryData, EImageFormat InFormat);

private:
	
	bool bIsValid;
	TArray<TArray<FColor>> NoiseTexture;
	TArray<TArray<FVector4>> RandomCache;
};